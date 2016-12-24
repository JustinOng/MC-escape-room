#include "MC-escape-room.h"

MFRC522 *mfrc522[MFRC522_NUM];
Keypad keypad = Keypad( makeKeymap(keys), keypad_row_pins, keypad_col_pins, KEYPAD_ROWS, KEYPAD_COLS );

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

MFRC522::MIFARE_Key key;

void print_uid(byte *uid, int column, int row) {
  lcd.setCursor(column, row);
  for(byte i = 0; i < UID_LENGTH; i++) {
    lcd.print(uid[i], HEX);
  }
}

void reset_ss_pins(void) {
  // http://forum.arduino.cc/index.php?topic=27986.msg207074#msg207074
  // 10 needs to be HIGH when SPI is reading or it'll hang for some odd reason
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  for (byte i = 0; i < MFRC522_NUM; i++) {
    digitalWrite(mfrc522_ss_pins[i], HIGH);
  }
}

void setup(void) {
  SPI.begin();

  for (byte i = 0; i < MFRC522_NUM; i++) {
    // reset pin is not really used by the library, putting the ss pin makes it function yet saves a pin
    mfrc522[i] = new MFRC522(mfrc522_ss_pins[i], mfrc522_ss_pins[i]);
    mfrc522[i]->PCD_Init();
    mfrc522[i]->PCD_SetAntennaGain(mfrc522[i]->RxGain_avg);
  }
  
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  
  lcd.begin(16, 2);

  Serial.begin(115200);
  Serial.println("Setup finished!");
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
  */
  static byte state = 0;
  
  //static unsigned long last_update = 0;
  
  //byte UID[UID_LENGTH];
  char key = keypad.getKey();
  
  // re-enable SPI mode. long story short, the use of pin 10 (SS) as one of the keypad pins
  // sets the SPI bus to SLAVE mode when the keypad is scanned.
  SPCR |= (1<<MSTR);

  if (state == 0) {
    lcd.setCursor(0, 1);
    lcd.print("*: Back, #:Check");
    lcd.setCursor(0, 0);
    lcd.print("Enter Code: ");
    
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
          state = 1;
        }
        else {
          lcd.print("Wrong Code!      ");
        }
        
        delay(1000);
      }
    }
  }
  else if (state == 1) {
    lcd.setCursor(0, 0);
    lcd.print("   What's the   ");
    lcd.setCursor(0, 1);
    lcd.print("    pattern?    ");
  }
  
  /*for(byte i = 0; i < MFRC522_NUM; i++) {
    reset_ss_pins();
    if (mfrc522[i]->PICC_IsNewCardPresent() && mfrc522[i]->PICC_ReadCardSerial()) {
      Serial.print("Found card on reader ");
      Serial.println(i);
      memcpy(UID, mfrc522[i]->uid.uidByte, 4);
      print_uid(UID, 0, i);
      mfrc522[i]->PICC_HaltA();
    }
    else {
      lcd.setCursor(0, i);
      lcd.print("        ");
    }
  }*/
}

