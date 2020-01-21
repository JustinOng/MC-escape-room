# MC-escape-room

Source code and hardware for a escape room prop I built. First part involves keying in the correct 3 digit code. Second part involves hanging 6 RFID cards on hooks in the correct sequence.

# Operation

On power on, the Arduino will wait until a I2C device is found at `0x3F` or `0x27`. These are the known addresses of the LCD modules used.

Once a LCD is found, the 6 RC522 rfid readers are initialised. If any reader is not detected, execution will halt. The microcontroller will have to be restarted (once the missing reader is fixed).

Operation then continues on to a state machine:

## Game States
* `CODE`: Keypad entry, correct 3 digit code needs to be entered.
  ```
  Enter Code: 
  *: Back, #:Check
  ```
  Upon entering the code, state `CARD` is entered.
* `CARD`: Card sequence, 6 cards need to be placed on the readers in the correct sequence
  ```
     What's the   
      pattern?
  ```
  Once the 6 cards are detected and are in the correct order, state `WIN` is entered
* `WIN`: Finish state, indicates to user that they have finished the challenge
  ```
  Congratulations!
  ```

  In this state, `PIN_DRIVE` will also be asserted.

## Debug States
A debug state can be entered by pressing the following sequence on the keypad at any state:

  `2, 2, 8, 8, 4, 6, 4, 6`

The sequence has to be entered consecutively, start again from the beginning if the wrong key is pressed. Once entered correctly, state 100 is entered.

* `DEBUG_MENU`
  ```
  DEBUG 1:Code
  2:Cards 3:Jump
  ```

  Pressing `1` enters state `DEBUG_CODE`, `2` enters state `DEBUG_CARD`, `3` enters state `DEBUG_JUMP`, any other key returns to state `CODE`.

* `DEBUG_CODE`: Code editing
  ```
  Cur code: xxx
  New code: yyy
  ```
  Where `xxx` is the current 3 digit code and `yyy` is the new 3 digit to code. Press `0`-`9` to set the new code, `*` to backspace (return to state `DEBUG_MENU` if no code is currently entered), `#` to save the new code

* `DEBUG_CARD`: Card editing
  ```
  Reader states:
  Readers: xxxxxx
  ```
  where each `x` represents the current state of the RFID readers from left to right:

  * `!`: No reader found
  * `?`: No card found
  * `-`: Card found, but has invalid data signature
  * `E`: Read error

  Pressing `#` will mark the current cards placed on the readers as correct (ie writing 1, 2, 3, ..., 6 to each of the cards)

* `DEBUG_JUMP`: Jump to game state
  ```
  JUMP-1: Code
  2: Cards 3: Win
  ```

  Self explanatory, press `1` to enter state `CODE`, `2` to enter state `CARD`, `3` to enter state `WIN`

## Cards

The card positions are tracked by writing data into the cards. For example, the leftmost card will have `A5 01` stored in it (`A5` indicates that the card has valid data), the next card will have `A5 02` and so on. This means that cards are interchangeable across different sets.
