// === pin config ===
const uint8_t PIN_SCL = 22,
  PIN_SDA = 23;

// === config for LCD ===
const uint8_t LCD_COLS = 16, LCD_ROWS = 2;
const uint8_t LCD_ADDRESSES[] = {0x27, 0x3F};

// === config for keypad ===

enum States_t {
  CODE,
  CARD,
  WIN,
  DEBUG_CODE,
  DEBUG_CARD,
  DEBUG_JUMP,
  INVALID
};
