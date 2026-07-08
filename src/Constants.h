#pragma once

#include <stdint.h>

namespace Constants
{
  constexpr uint8_t DEFAULT_PPQN = 24;
  constexpr uint16_t DEFAULT_BPM = 10;

  constexpr uint8_t NUMBER_OF_TRACKS = 5;
  constexpr uint8_t NUMBER_OF_QUARTER_NOTES = 64;
  constexpr uint8_t NUMBER_OF_STEPS = 4;
  constexpr uint8_t NUMBER_OF_INSTRUMENTS = 12;
  constexpr uint8_t NUMBER_OF_DISPLAYED_TRACKS = 2;
  constexpr uint8_t NUMBER_OF_DISPLAYED_QN = 1;
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t  SCREEN_HEIGHT = 64;

  /**
  | OLED | ESP32   |
  | ---- | ------- |
  | VCC  | 3.3V    |
  | GND  | GND     |
  | SDA  | GPIO 21 |
  | SCL  | GPIO 22 |

  | KY040 #1 | ESP32   |
  | -------- | ------- |
  | CLK      | GPIO 32 |
  | DT       | GPIO 33 |
  | SW       | GPIO 25 |

  | KY040 #2 | ESP32   |
  | -------- | ------- |
  | CLK      | GPIO 26 |
  | DT       | GPIO 27 |
  | SW       | GPIO 14 |

  | Bouton | GPIO    |
  | ------ | ------- |
  | B1     | GPIO 4  |
  | B2     | GPIO 5  |
  | B3     | GPIO 16 |
  | B4     | GPIO 17 |
  | B5     | GPIO 18 |
  | B6     | GPIO 19 |
  | B7     | GPIO 23 |
  | B8     | GPIO 12 |
  | B9     | GPIO 13 |

  */

  constexpr uint8_t NUMBER_OF_ROTARY_ENCODERS = 2;

  constexpr uint8_t ROTARY_ENCODER_A = 0;
  constexpr uint8_t ROTARY_ENCODER_B = 1;

  constexpr uint8_t ROTARY_ENCODER_ONE_A = 32;
  constexpr uint8_t ROTARY_ENCODER_ONE_B = 33;

  constexpr uint8_t ROTARY_ENCODER_TWO_A = 26;
  constexpr uint8_t ROTARY_ENCODER_TWO_B = 27;

  constexpr uint8_t ROTARY_ENCODERS_PIN[NUMBER_OF_ROTARY_ENCODERS][3] = {
    {
      ROTARY_ENCODER_ONE_A,
      ROTARY_ENCODER_ONE_B
    },
    {
      ROTARY_ENCODER_TWO_A,
      ROTARY_ENCODER_TWO_B
    }
  };

  constexpr byte ROWS = 4;
  constexpr byte COLS = 4;

  constexpr uint8_t NUMBER_OF_BUTTONS = ROWS * COLS;

  // Define the keymaps.  The blank spot (lower left) is the space character.
  char KEY_MATRIX[ROWS][COLS] = {
      { 'A', 'B', 'C', 'D' },
      { 'E', 'F', 'G', 'H' },
      { 'I', 'J', 'K', 'L' },
      { 'M', 'N', 'O', 'P' }
  };

  byte ROWS_PINS[ROWS] = {4, 13, 14, 16}; //connect to the row pinouts of the keypad
  byte COLS_PINS[COLS] = {17, 0, 2, 15};

  // constexpr uint8_t FN_BUTTON_PIN = 4;

  // // GPIO boutons
  // constexpr uint8_t BUTTONS_PIN[8] = {
  //     5, 16, 17, 18, 
  //     19, 23, 12, 13
  // };
};
