# MC-escape-room

Source code and hardware for a escape room prop I built. First part involves keying in the correct 3 digit code. Second part involves hanging 6 RFID cards on hooks in the correct sequence.

Primary modules used:
* LM2596 DC-DC Step Down: Create 3.3V rail to power everything
* RC522: RFID Reader
* Arduino Nano: Main microcontroller, running at 3.3V so level shifting is not required
* Generic 16x2 LCD with I2C Backpack: Used to display prompts and feedback

The Arduino Nano's were programmed with a 3.3V USBasp (primarily because I wanted to see if I could program it while the Nano was plugged into the board, but turns out it dosent work reliably with all the RFID modules plugged in. Perhaps the USBasp cannot drive the SPI pins hard enough?).

Schematics:
![Schematics](img/schematics.png)

PCB:
![PCB](img/pcb.jpg)

Overall layout:
![Overall](img/overall.jpg)

# Operation

On power on, the Arduino will wait until a I2C device is found at `0x3F` or `0x27`. These are the known addresses of the LCD modules used.

Once a LCD is found, the 6 RC522 rfid readers are initialised.

Operation then continues on to a state machine:

## Game States
* 0: Keypad entry, correct 3 digit code needs to be entered.
  ```
  Enter Code: 
  *: Back, #:Check
  ```
  Upon entering the code, state 1 is entered.
* 1: Card sequence, 6 cards need to be placed on the readers in the correct sequence
  ```
     What's the   
      pattern?
  ```
  Once the 6 cards are detected and are in the correct order, state 2 is entered
* 2: Finish state, indicates to user that they have finished the challenge
  ```
  Congratulations!
  ```

## Debug States
A debug state can be entered by pressing the following sequence on the keypad at any state:

  `2, 2, 8, 8, 4, 6, 4, 6`

The sequence has to be entered consecutively, start again from the beginning if the wrong key is pressed. Once entered correctly, state 100 is entered.

* 100: Debug menu
  ```
  DEBUG 1:Code
  2:Cards 3:Jump
  ```

  Pressing `1` enters state 101, `2` enters state 102, `3` enters state 103, `*` returns to state 0

* 101: Code editing
  ```
  Cur code: xxx
  New code: yyy
  ```
  Where `xxx` is the current 3 digit code and `yyy` is the new 3 digit to code. Press `0`-`9` to set the new code, `*` to backspace (return to state 100 if no code is currently entered), `#` to save the new code

* 102: Card editing
  ```
  Reader states:
  Readers: xxxxxx
  ```
  where each `x` represents the current state of the RFID readers from left to right:

  * `?`: No card found
  * `*`: Card found, but has invalid data signature
  * `E`: Read error

  Pressing `#` will mark the current cards placed on the readers as correct (ie writing 1, 2, 3, ..., 6 to each of the cards)

* 103: Jump to game state
  ```
  JUMP-0: Code
  1: Cards 2: Win
  ```

  Self explanatory, press `0` to enter state 0, `1` to enter state 1, `2` to enter state 2

## Cards

The card positions are tracked by writing data into the cards. For example, the leftmost card will have `A5 01` stored in it (`A5` indicates that the card has valid data), the next card will have `A5 02` and so on.
