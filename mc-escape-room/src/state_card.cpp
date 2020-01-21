#include <SPI.h>
#include "state_card.h"

byte State_Card::begin(void) {
  for (uint8_t i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  for(uint8_t i = 0; i < NUM_MFRC522; i++) {
    pinMode(RESET_PINS[i], OUTPUT);
    // assert all RSTs
    digitalWrite(RESET_PINS[i], LOW);
  }

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SS);

  for(uint8_t i = 0; i < NUM_MFRC522; i++) {
    Serial.print("Initialising reader ");
    Serial.println(i);

    digitalWrite(RESET_PINS[i], HIGH);

    mfrc522.PCD_Init(PIN_SS, -1);
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();

    char version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);

    digitalWrite(RESET_PINS[i], LOW);

    if (version == 0x00 || version == 0xFF) {
      return i+1;
    }
  }

  return 0;
}

bool State_Card::check(void) {
  byte data;

  for (uint8_t i = 0; i < NUM_MFRC522; i++) {
    if (read_reader(i, &data) != OK) return false;

    if (data != i) return false;
  }

  return true;
}

State_Card::Reader_State_t State_Card::read_reader(uint8_t i, uint8_t *data) {
  digitalWrite(RESET_PINS[i], HIGH);

  Reader_State_t st = _read_reader(i, data);
  
  digitalWrite(RESET_PINS[i], LOW);

  return st;
}

State_Card::Reader_State_t State_Card::_read_reader(uint8_t i, uint8_t *data) {
  MFRC522::StatusCode status;
  byte buf[18];
  byte buf_size = sizeof(buf);

  mfrc522.PCD_Init(PIN_SS, -1);
  delay(4);

  char version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);

  if (version == 0x00 || version == 0xFF) {
    // 0x00 or 0xFF is likely failed to communicate with reader
    Serial.print("Got bad version from reader ");
    Serial.print(i);
    Serial.print(": 0x");
    Serial.println(version, HEX);

    return NO_READER;
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return NO_CARD;
  }

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATA_TRAILER_BLOCK, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return AUTH_ERROR;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(DATA_BLOCK, buf, &buf_size);
  mfrc522.PCD_StopCrypto1();
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    
    return OP_ERROR;
  }

  if (buf[DATA_SIGNATURE_INDEX] != DATA_SIGNATURE) {
    return NO_DATA;
  }

  *data = buf[DATA_INDEX];

  return OK;
}

State_Card::Reader_State_t State_Card::write_reader(uint8_t i) {
  digitalWrite(RESET_PINS[i], HIGH);

  Reader_State_t st = _write_reader(i);
  
  digitalWrite(RESET_PINS[i], LOW);

  return st;
}

State_Card::Reader_State_t State_Card::_write_reader(uint8_t i) {
  MFRC522::StatusCode status;
  byte buf[16] = {0};
  byte buf_size = sizeof(buf);

  digitalWrite(RESET_PINS[i], HIGH);

  mfrc522.PCD_Init(PIN_SS, -1);
  delay(4);

  char version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);

  if (version == 0x00 || version == 0xFF) {
    // 0x00 or 0xFF is likely failed to communicate with reader
    Serial.print("Got bad version from reader ");
    Serial.print(i);
    Serial.print(": 0x");
    Serial.println(version, HEX);

    return NO_READER;
  }

  buf[DATA_SIGNATURE_INDEX] = DATA_SIGNATURE;
  buf[DATA_INDEX] = i+1;

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return NO_CARD;
  }

  // authenticate with key A, default keys (configured in setup)
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, DATA_TRAILER_BLOCK, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return AUTH_ERROR;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(DATA_BLOCK, buf, buf_size);
  mfrc522.PCD_StopCrypto1();
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return OP_ERROR;
  }

  return OK;
}
