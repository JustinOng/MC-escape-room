#ifndef MAIN_H
#define MAIN_H

// === pin config ===
const uint8_t PIN_SCL = 22,
  PIN_SDA = 23;

// === config for LCD ===
const uint8_t LCD_COLS = 16, LCD_ROWS = 2;
const uint8_t LCD_ADDRESSES[] = {0x27, 0x3F};

// === config for keypad ===
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 3;

byte KEYPAD_ROW_PINS[KEYPAD_ROWS] = {35, 34, 39, 36};
byte KEYPAD_COL_PINS[KEYPAD_COLS] = {25, 33, 32};

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// === config for debug mode ===

// key sequence to enter debug mode
const uint8_t DEBUG_SEQUENCE_LENGTH = 8;
const char debug_sequence[DEBUG_SEQUENCE_LENGTH] = {'2', '2', '8', '8', '4', '6', '4', '6'};

enum States_t {
  CODE,
  CODE_CORRECT,
  CODE_WRONG,
  CARD,
  WIN,
  DEBUG_MENU,
  DEBUG_CODE,
  DEBUG_CARD,
  DEBUG_JUMP,
  INVALID
};

#endif
