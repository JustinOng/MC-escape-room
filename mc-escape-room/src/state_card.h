#ifndef STATE_CARD_H
#define STATE_CARD_H

#include <Arduino.h>
#include <MFRC522.h>

const uint8_t NUM_MFRC522 = 6;
const uint8_t RESET_PINS[NUM_MFRC522] = {5, 18, 19, 14, 15, 13};

const uint8_t PIN_SCK = 17;
const uint8_t PIN_MOSI = 16;
const uint8_t PIN_MISO = 4;
const uint8_t PIN_SS = 21;

// For memory structure of the mifare classic 1kb cards,
// http://www.nxp.com/documents/application_note/AN1304.pdf
/*
 * Mifare classic 1kb has 16 sectors, each sector has 4 blocks, each block storing 16 bytes
 */
// sector to store our data in
const uint8_t DATA_SECTOR = 1;

// trailer block of the above sector that will be authenticated against
// indexed from start of card
const uint8_t DATA_TRAILER_BLOCK = 7;

// block to store our data in
// indexed from start of card
const uint8_t DATA_BLOCK = 4;

// signature byte used to indicate that this system has stored data
const uint8_t DATA_SIGNATURE = 0xA5;

// byte index to store DATA_SIGNATURE in
const uint8_t DATA_SIGNATURE_INDEX = 1;

// byte index to store our data in
const uint8_t DATA_INDEX = 2;

// how long before a "correct reading" expires
const uint16_t READER_TIMEOUT = 500;

class State_Card {
  public:
    enum Reader_State_t {
      NO_READER,
      NO_CARD,
      NO_DATA,
      OK,
      AUTH_ERROR,
      OP_ERROR
    };

    byte begin(void);
    bool check(void);
    Reader_State_t read_reader(uint8_t i, uint8_t *data);
    Reader_State_t write_reader(uint8_t i);
  
  private:
    MFRC522::MIFARE_Key key;
    MFRC522 mfrc522;
};

#endif
