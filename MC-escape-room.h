#include "Keypad.h"
#include "MFRC522.h"
#include "SendOnlySoftwareSerial.h"
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "FastLED.h"

/*
 * === Config for LEDs ===
 */
#define LED_DATA_PIN 0
#define NUM_LEDS 6
#define COLOR_NOWIN CRGB::Black;
#define COLOR_WIN CRGB::Green;

/*
 * === Config for LCD ===
 */
#define LCD_ADDR 0x4E
/*
 *  === Config for Keypad ===
 */
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

byte keypad_row_pins[KEYPAD_ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte keypad_col_pins[KEYPAD_COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

/*
 * === Config for MFRC522 ===
 */
#define UID_LENGTH 4
#define MFRC522_NUM 6

byte mfrc522_ss_pins[] = {9, 10, 14, 15, 16, 17};

// For memory structure of the mifare classic 1kb cards,
// http://www.nxp.com/documents/application_note/AN1304.pdf
/*
 * Mifare classic 1kb has 16 sectors, each sector has 4 blocks, each block storing 16 bytes
 */
// sector to store our data in
#define DATA_SECTOR 1

// trailer block of the above sector that will be authenticated against
// indexed from start of card
#define DATA_TRAILER_BLOCK 7

// block to store our data in
// indexed from start of card
#define DATA_BLOCK 4

// signature byte used to indicate that this system has stored data
#define DATA_SIGNATURE 0xA5

// byte index to store DATA_SIGNATURE in
#define DATA_SIGNATURE_INDEX 1

// byte index to store our data in
#define DATA_INDEX 2

// how long before a "correct reading" expires
#define READER_TIMEOUT 500

/*
 * === Config for state 0: waiting for code ===
 */

#define MAX_CODE_LENGTH 4
#define CORRECT_CODE_LENGTH 3
#define CODE_EEPROM_ADDRESS 10
char correct_code[CORRECT_CODE_LENGTH] = {'1', '2', '3'};

/*
 * === Config for state 100: debug mode ===
 */

#define DEBUG_SEQUENCE_LENGTH 8
char debug_sequence[DEBUG_SEQUENCE_LENGTH] = {'2', '2', '8', '8', '4', '6', '4', '6'};

