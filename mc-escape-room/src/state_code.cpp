#include "state_code.h"

void State_Code::begin(void) {
  prefs.begin(PREFERENCE_NAMESPACE);

  read_code();
}

void State_Code::reset(void) {
  code_length = 0;
}

void State_Code::add(char c) {
  if (code_length < MAX_CODE_LENGTH) {
    Serial.print("Adding char: ");
    Serial.println(c);
    code[code_length++] = c;
  }
}

void State_Code::del(void) {
  if (code_length > 0) {
    Serial.println("Backspace");
    code_length--;
  }
}

bool State_Code::check(void) {
  Serial.print("Checking code: ");
  for (uint8_t i = 0; i < CORRECT_CODE_LENGTH; i++) {
    Serial.printf("%02X ", code[i]);
  }
  Serial.println();

  return memcmp(code, correct_code, CORRECT_CODE_LENGTH);
}

void State_Code::read_code(void) {
  prefs.getBytes(PREFERENCE_NAME, correct_code, CORRECT_CODE_LENGTH);

  Serial.print("Read code from preferences: ");
  for (uint8_t i = 0; i < CORRECT_CODE_LENGTH; i++) {
    Serial.printf("%02X ", correct_code[i]);
  }
  Serial.println();
}
