#include "libraries/Keypad/Keypad.h"
#include "libraries/rfid/MFRC522.h"
#include <LiquidCrystal.h>

#include "MC-escape-room.h"

MFRC522 *mfrc522 = new MFRC522(4, 4);
Keypad keypad = Keypad( makeKeymap(keys), keypad_row_pins, keypad_col_pins, KEYPAD_ROWS, KEYPAD_COLS );

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void print_uid(byte *uid, int column, int row) {
  lcd.setCursor(column, row);
  for(byte i = 0; i < UID_LENGTH; i++) {
    lcd.print(uid[i], HEX);
  }
}

void setup(void) {
  SPI.begin();
  mfrc522->PCD_Init();
  mfrc522->PCD_SetAntennaGain(mfrc522->RxGain_max);
  
  lcd.begin(16, 2);
}

void loop(void) {
  byte UID[UID_LENGTH];
  char key = keypad.getKey();

  if (key != NO_KEY){
    lcd.print(key);
  }

  if (mfrc522->PICC_IsNewCardPresent() && mfrc522->PICC_ReadCardSerial()) {
    memcpy(UID, mfrc522->uid.uidByte, 4);
    print_uid(UID, 0, 0);
  }
}

