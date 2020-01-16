#ifndef STATE_CODE_H
#define STATE_CODE_H

#include <Arduino.h>
#include <Preferences.h>

const uint8_t MAX_CODE_LENGTH = 4;
const uint8_t CORRECT_CODE_LENGTH = 3;
const char PREFERENCE_NAMESPACE[] = "state code";
const char PREFERENCE_NAME[] = "code";

class State_Code {
  public:
    void begin(void);
    void reset(void);
    void add(char c);
    void del(void);
    bool check(void);
  
    uint8_t code_length = 0;
    char code[MAX_CODE_LENGTH];
  
  private:
    Preferences prefs;
    char correct_code[CORRECT_CODE_LENGTH] = {0};

    void read_code(void);
};

#endif
