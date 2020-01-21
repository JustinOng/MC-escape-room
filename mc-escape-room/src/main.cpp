#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include "main.h"
#include "state_code.h"
#include "state_card.h"

LiquidCrystal_I2C *lcd;
Keypad keypad = Keypad( makeKeymap(keys), KEYPAD_ROW_PINS, KEYPAD_COL_PINS, KEYPAD_ROWS, KEYPAD_COLS);

State_Code st_code;
State_Card st_card;

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
  byte result;

  Serial.begin(115200);

  initialise_lcd();

  st_code.begin();

  result = st_card.begin();
  if (result > 0) {
    lcd->clear();
    lcd->print("Reader ");
    lcd->print(result);
    lcd->setCursor(0, 1);
    lcd->print("failed to init");
    while(1);
  }

  Serial.println("Setup complete");
}

void loop() {
  static States_t state_cur = CODE,
    state_prev = INVALID;

  States_t state_new = INVALID;
  
  static uint32_t state_last_change = 0;
  
  // tracks how many correct digits of the debug sequence
  // has been entered
  static uint8_t debug_count = 0;
  
  char key = keypad.getKey();

  if (key == debug_sequence[debug_count]) {
    debug_count++;

    if (debug_count >= DEBUG_SEQUENCE_LENGTH) {
      state_new = DEBUG_MENU;
    }
  }
  else if (key) {
    debug_count = 0;
  }

  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
  }

  switch(state_cur) {
    case CODE:
      if (state_prev != state_cur) {
        lcd->clear();
        lcd->print("Enter Code: ");
        lcd->setCursor(0, 1);
        lcd->print("*: Back, #:Check");
        st_code.reset();
      }

      if (key >= '0' && key <= '9') {
        st_code.add(key);
      } else if (key == '*') {
        st_code.del();
      } else if (key == '#') {
        if (st_code.check()) {
          state_new = CODE_CORRECT;
        } else {
          state_new = CODE_WRONG;
        }
      } else if (state_prev == state_cur) {
        // no change, just break
        break;
      }
    
      // update displayed code
      lcd->setCursor(12, 0);
      
      for(byte i = 0; i < MAX_CODE_LENGTH; i++) {
        if (i < st_code.code_length) {
          lcd->print(st_code.code[i]);
        }
        else {
          lcd->print(' ');
        }
      }
      break;
    case CODE_CORRECT:
      if (state_prev != state_cur) {
        lcd->setCursor(0, 1);
        lcd->print("Correct!         ");
      }

      if ((millis() - state_last_change) > 1000) {
        state_new = CARD;
      }
      break;
    case CODE_WRONG:
      if (state_prev != state_cur) {
        lcd->setCursor(0, 1);
        lcd->print("Wrong Code!      ");
      }

      if ((millis() - state_last_change) > 1000) {
        state_new = CODE;
      }
      break;
    case CARD:
      if (state_prev != state_cur) {
        lcd->clear();
        lcd->print("   What's the   ");
        lcd->setCursor(0, 1);
        lcd->print("    pattern?    ");
      }

      if (st_card.check()) {
        state_new = WIN;
      }
      break;
    case WIN:
      if (state_prev != state_cur) {
        lcd->clear();
        lcd->print("Congratulations!");
      }

      if (key == '#') {
        state_new = CODE;
      }
      break;
    case DEBUG_MENU:
      if (state_prev != state_cur) {
        lcd->clear();
        lcd->print("DEBUG 1:Code");
        lcd->setCursor(0, 1);
        lcd->print("2:Cards 3:Jump");
      }

      if (key == '1') {
        state_new = DEBUG_CODE;
      } else if (key == '2') {
        state_new = DEBUG_CARD;
      } else if (key == '3') {
        state_new = DEBUG_JUMP;
      } else if (key) {
        state_new = CODE;
      }
      break;
    case DEBUG_CODE:
      if (state_prev != state_cur) {
        lcd->clear();
        lcd->print("Cur code: ");
        for (uint8_t i = 0; i < CORRECT_CODE_LENGTH; i++) {
          lcd->print(st_code.correct_code[i]);
        }
        lcd->setCursor(0, 1);
        lcd->print("New code: ");
        st_code.reset();
      }

      if (key >= '0' && key <= '9') {
        if (st_code.code_length < CORRECT_CODE_LENGTH) {
          st_code.add(key);
        }
      } else if (key == '*') {
        if (st_code.code_length > 0) {
          st_code.del();
        } else {
          state_new = DEBUG_MENU;
        }
      } else if (key == '#') {
        if (st_code.write_code()) {
          state_new = DEBUG_CODE_OK;
        } else {
          state_new = DEBUG_CODE_BAD;
        }
      } else if (state_prev == state_cur) {
        // no change, just break
        break;
      }
    
      // update displayed code
      lcd->setCursor(10, 1);
      
      for(byte i = 0; i < CORRECT_CODE_LENGTH; i++) {
        if (i < st_code.code_length) {
          lcd->print(st_code.code[i]);
        }
        else {
          lcd->print(' ');
        }
      }

      break;
    case DEBUG_CODE_OK:
      if (state_prev != state_cur) {
        lcd->setCursor(0, 1);
        lcd->print("Code updated!");
      }

      if ((millis() - state_last_change) > 1000) {
        state_new = DEBUG_CODE;
      }
      break;
    case DEBUG_CODE_BAD:
      if (state_prev != state_cur) {
        lcd->setCursor(0, 1);
        lcd->print("Update failed!");
      }

      if ((millis() - state_last_change) > 1000) {
        state_new = DEBUG_CODE;
      }
      break;
  }

  state_prev = state_cur;

  if (state_new != INVALID) {
    state_cur = state_new;

    Serial.print("Entering state ");
    switch(state_cur) {
      case CODE: Serial.println("CODE"); break;
      case CODE_CORRECT: Serial.println("CODE_CORRECT"); break;
      case CODE_WRONG: Serial.println("CODE_WRONG"); break;
      case CARD: Serial.println("CARD"); break;
      case WIN: Serial.println("WIN"); break;
      case DEBUG_MENU: Serial.println("DEBUG_MENU"); break;
      case DEBUG_CODE: Serial.println("DEBUG_CODE"); break;
      case DEBUG_CODE_OK: Serial.println("DEBUG_CODE_OK"); break;
      case DEBUG_CODE_BAD: Serial.println("DEBUG_CODE_BAD"); break;
      case DEBUG_CARD: Serial.println("DEBUG_CARD"); break;
      case DEBUG_JUMP: Serial.println("DEBUG_JUMP"); break;
      case INVALID: Serial.println("INVALID?!"); break;
    }
    state_last_change = millis();
  }
}
