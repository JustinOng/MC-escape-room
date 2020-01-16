#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include "main.h"

LiquidCrystal_I2C *lcd;

/*
  initialise_lcd()

  Loops until an LCD is found and initialised
*/
void initialise_lcd(void) {
  while(1) {
    for(uint8_t i = 0; i < sizeof(LCD_ADDRESSES); i++) {
      uint8_t address = LCD_ADDRESSES[i];

      lcd = new LiquidCrystal_I2C((PCF8574_address) address, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
      if (lcd->begin(LCD_COLS, LCD_ROWS, LCD_5x8DOTS, PIN_SDA, PIN_SCL)) {
        Serial.print("LCD initialised at 0x");
        Serial.println(address, HEX);
        return;
      }

      Serial.print("Failed to initialise LCD at 0x");
      Serial.println(address, HEX);
      delete lcd;
    }

    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  initialise_lcd();

  Serial.println("Setup complete");
}

void loop() {
  static States_t state_cur = CODE,
    state_prev = INVALID,
    state_new = INVALID;
  


  state_prev = state_cur;

  if (state_new != INVALID) {
    state_cur = state_new;
  }
}
