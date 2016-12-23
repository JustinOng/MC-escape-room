#include "Keypad.h"
#include "MFRC522.h"
#include <LiquidCrystal.h>

/*
 * === Config for LCD ===
 */
#define LCD_RS 2
// LCD_RW is hardwired to ground
#define LCD_EN 3
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

/*
 *  === Config for Keypad ===
 */
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

byte keypad_row_pins[KEYPAD_ROWS] = {14, 15, 16, 17}; //connect to the row pinouts of the keypad
byte keypad_col_pins[KEYPAD_COLS] = {8, 9, 10}; //connect to the column pinouts of the keypad

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

/*
 * === Config for MFRC522
 */
#define UID_LENGTH 4
#define MFRC522_NUM 2

byte mfrc522_ss_pins[MFRC522_NUM] = {4, 5};
