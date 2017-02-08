#include "MC-escape-room.h"

MFRC522 mfrc522[MFRC522_NUM];
Keypad keypad = Keypad( makeKeymap(keys), keypad_row_pins, keypad_col_pins, KEYPAD_ROWS, KEYPAD_COLS );

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

MFRC522::MIFARE_Key key;

SendOnlySoftwareSerial swSerial(1);

void print_uid(byte *uid, int column, int row) {
  lcd.setCursor(column, row);
  for(byte i = 0; i < UID_LENGTH; i++) {
    lcd.print(uid[i], HEX);
  }
}

byte read_rfid_reader(byte i, byte * data) {
  /* 
   *  reads rfid reader with index i
   *  returns:
   *    0: successful read
   *    1: no card found
   *    2: no signature found
   *    3: read error
   *    
   *    if successful read, data will be written to *data
   */ 
  
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  
  //reset_ss_pins();

  if (!mfrc522[i].PICC_IsNewCardPresent() || !mfrc522[i].PICC_ReadCardSerial()) {
    return 1;
  }

  // authenticate with key A, default keys (configured in setup)
  status = (MFRC522::StatusCode) mfrc522[i].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATA_TRAILER_BLOCK, &key, &(mfrc522[i].uid));
  if (status != MFRC522::STATUS_OK) {
    swSerial.print(F("PCD_Authenticate() failed: "));
    swSerial.println(mfrc522[i].GetStatusCodeName(status));
    return 3;
  }
  
  status = (MFRC522::StatusCode) mfrc522[i].MIFARE_Read(DATA_BLOCK, buffer, &size);
  mfrc522[i].PCD_StopCrypto1();
  
  if (status != MFRC522::STATUS_OK) {
    swSerial.print(F("MIFARE_Read() failed: "));
    swSerial.println(mfrc522[i].GetStatusCodeName(status));
    
    return 3;
  }

  if (buffer[DATA_SIGNATURE_INDEX] != DATA_SIGNATURE) {
    return 2;
  }

  *data = buffer[DATA_INDEX];
  
  return 0;
}

byte write_rfid_reader(byte i) {
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  byte dataBlock[16] = {0};

  dataBlock[DATA_SIGNATURE_INDEX] = DATA_SIGNATURE;
  dataBlock[DATA_INDEX] = i+1;

  if (!mfrc522[i].PICC_IsNewCardPresent() || !mfrc522[i].PICC_ReadCardSerial()) {
    return 1;
  }

  // authenticate with key A, default keys (configured in setup)
  status = (MFRC522::StatusCode) mfrc522[i].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATA_TRAILER_BLOCK, &key, &(mfrc522[i].uid));
  if (status != MFRC522::STATUS_OK) {
    swSerial.print(F("PCD_Authenticate() failed: "));
    swSerial.println(mfrc522[i].GetStatusCodeName(status));
    return 2;
  }

  status = (MFRC522::StatusCode) mfrc522[i].MIFARE_Write(DATA_BLOCK, dataBlock, 16);
  mfrc522[i].PCD_StopCrypto1();
  
  if (status != MFRC522::STATUS_OK) {
    swSerial.print(F("MIFARE_Write() failed: "));
    swSerial.println(mfrc522[i].GetStatusCodeName(status));
    return 2;
  }

  return 0;
}

void setup(void) {
  SPI.begin();
  delay(50);

  for (byte i = 0; i < MFRC522_NUM; i++) {
    // reset pin is not really used by the library, putting the ss pin makes it function yet saves a pin
    mfrc522[i].PCD_Init(mfrc522_ss_pins[i], mfrc522_ss_pins[i]);
    mfrc522[i].PCD_SetAntennaGain(mfrc522[i].RxGain_avg);
  }
  
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  
  lcd.begin();

  swSerial.begin(115200);
  swSerial.println("Setup finished!");
}

byte is_reader_right(byte i) {
  /*
   * checks whether rfid reader has i+1 stored in the card
   */

  static unsigned long last_correct[MFRC522_NUM] = {};
  byte data;
  byte result = read_rfid_reader(i, &data);

  if (result == 0) {
    if (data == (i+1)) {
      last_correct[i] = millis();
    }
  }

  return (millis() - last_correct[i]) < READER_TIMEOUT;
}

char code[MAX_CODE_LENGTH];
byte code_length = 0;

byte is_code_correct(void) {
  if (code_length != CORRECT_CODE_LENGTH) return 0;

  for(byte i = 0; i < code_length; i++) {
    if (code[i] != correct_code[i]) return 0;
  }

  return 1;
}

void loop(void) {
  /* 
   * state = 0 for keypad entry
   * state = 1 for rfid card sequence
   * state = 2 for congratulations screen
   * state = 100 for debug
  */
  static byte state = 0;
  static byte pState = 255;

  byte state_to_set = 255;
  
  // counter used to track length of debug sequence entered
  // increments for every correct character of debug sequence entered
  static byte debug_sequence_count = 0;
  
  char key = keypad.getKey();

  if (key == debug_sequence[debug_sequence_count]) {
    debug_sequence_count++;

    if (debug_sequence_count >= DEBUG_SEQUENCE_LENGTH) {
      state_to_set = 100;
    }
  }
  else if (key) {
    debug_sequence_count = 0;
  }

  if (state == 0) {
    if (pState != 0) {
      lcd.clear();
      lcd.print("Enter Code: ");
      lcd.setCursor(0, 1);
      lcd.print("*: Back, #:Check");
      code_length = 0;
    }
    
    if (key) {
      if (key >= '0' && key <= '9') {
        if (code_length < MAX_CODE_LENGTH) {
          code[code_length++] = key;
        }
      }

      if (key == '*') {
        if (code_length > 0) {
          code_length--;
        }
      }
    
      lcd.setCursor(12, 0);
      
      for(byte i = 0; i < MAX_CODE_LENGTH; i++) {
        if (i < code_length) {
          lcd.print(code[i]);
        }
        else {
          lcd.print(' ');
        }
      }
      
      if (key == '#') {
        lcd.setCursor(0, 1);
        
        if (is_code_correct()) {
          lcd.print("Correct!         ");
          state_to_set = 1;
        }
        else {
          lcd.print("Wrong Code!      ");
        }
        
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("*: Back, #:Check");
      }
    }
  }
  else if (state == 1) {
    if (pState != 1) {
      lcd.clear();
      lcd.print("   What's the   ");
      lcd.setCursor(0, 1);
      lcd.print("    pattern?    ");
    }
    
    byte correct = 1;
    
    for(byte i = 0; i < MFRC522_NUM; i++) {
      if (!is_reader_right(i)) {
        correct = 0;
        break;
      }
    }

    if (correct) state_to_set = 2;
  }
  else if (state == 2) {
    if (pState != 2) {
      lcd.clear();
      lcd.print("Congratulations!");
    }

    if (key == '#') {
      state_to_set = 0;
    }
  }
  else if (state == 100) {
    static unsigned long last_state_update[MFRC522_NUM] = {0};
    static byte last_read_state[MFRC522_NUM] = {0};

    // index tracking which reader was last read
    // used to read only one reader per cycle so buttons remain responsive
    static byte read_index = 255;
    
    if (pState != 100) {
      lcd.clear();
      lcd.print("DEBUG");
      lcd.setCursor(0, 1);
      lcd.print("Readers: ");
      swSerial.println("Entering debug mode");
    }

    if (key >= '0' && key <= '2') state_to_set = key - '0';

    read_index++;
    
    if (read_index >= MFRC522_NUM) read_index = 0;
    
    byte data = 0;
    byte result = read_rfid_reader(read_index, &data);

    // some "debouncing" since reads can fail while card is still there
    // only allows not found state to propagate when last state update > READER_TIMEOUT ms ago
    if (result == 1) {
      if ((millis() - last_state_update[read_index]) < READER_TIMEOUT) {
        result = last_read_state[read_index];
      }
    }
    else {        
      last_state_update[read_index] = millis();
    }
    
    lcd.setCursor(9+read_index, 1);
    switch(result) {
      case 0:
        if (data != 0) {
          lcd.print(data);
        }
        break;
      case 1:
        lcd.print('?');
        break;
      case 2:
        lcd.print('*');
        break;
      case 3:
        lcd.print('E');
        break;
    }

    if (result != 1) {
      last_read_state[read_index] = result;
    }

    if (key == '*') {
      state_to_set = 0;
    }
    else if (key == '#') {
      for(byte i = 0; i < MFRC522_NUM; i++) {
        write_rfid_reader(i);
      }
    }
  }
  else if (state == 101) {
    
  }

  pState = state;

  if (state_to_set != 255) {
    state = state_to_set;
  }
}


