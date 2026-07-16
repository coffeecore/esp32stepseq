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

  constexpr uint8_t NUMBER_OF_VOICES = 16;

  constexpr uint8_t SCREEN_WIDTH = 128;
  constexpr uint8_t  SCREEN_HEIGHT = 64;

  constexpr uint16_t MAX_SAMPLES = 255;

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

  constexpr uint8_t ROTARY_ENCODERS_PIN[NUMBER_OF_ROTARY_ENCODERS][2] = {
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

  constexpr char constNotesStr[128][6] = {
    "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
    "C0",  "C#0",  "D0",  "D#0",  "E0",  "F0",  "F#0",  "G0",  "G#0",  "A0",  "A#0",  "B0",
    "C1",  "C#1",  "D1",  "D#1",  "E1",  "F1",  "F#1",  "G1",  "G#1",  "A1",  "A#1",  "B1",
    "C2",  "C#2",  "D2",  "D#2",  "E2",  "F2",  "F#2",  "G2",  "G#2",  "A2",  "A#2",  "B2",
    "C3",  "C#3",  "D3",  "D#3",  "E3",  "F3",  "F#3",  "G3",  "G#3",  "A3",  "A#3",  "B3",
    "C4",  "C#4",  "D4",  "D#4",  "E4",  "F4",  "F#4",  "G4",  "G#4",  "A4",  "A#4",  "B4",
    "C5",  "C#5",  "D5",  "D#5",  "E5",  "F5",  "F#5",  "G5",  "G#5",  "A5",  "A#5",  "B5",
    "C6",  "C#6",  "D6",  "D#6",  "E6",  "F6",  "F#6",  "G6",  "G#6",  "A6",  "A#6",  "B6",
    "C7",  "C#7",  "D7",  "D#7",  "E7",  "F7",  "F#7",  "G7",  "G#7",  "A7",  "A#7",  "B7",
    "C8",  "C#8",  "D8",  "D#8",  "E8",  "F8",  "F#8",  "G8",  "G#8",  "A8",  "A#8",  "B8",
    "C9",  "C#9",  "D9",  "D#9",  "E9",  "F9",  "F#9",  "G9"
};

constexpr uint32_t constNotesFreq[128] = {
      8175,   8661,   9177,   9722,  10300,  10913,  11562,  12249,
     12978,  13750,  14567,  15434,  16351,  17323,  18354,  19445,
     20601,  21827,  23124,  24499,  25956,  27500,  29135,  30867,
     32703,  34647,  36708,  38890,  41203,  43654,  46249,  48999,
     51913,  55000,  58270,  61735,  65406,  69295,  73416,  77781,
     82406,  87307,  92498,  97999, 103826, 110000, 116540, 123470,
    130812, 138591, 146832, 155563, 164813, 174614, 184997, 195997,
    207652, 220000, 233081, 246941, 261625, 277182, 293664, 311126,
    329627, 349228, 369994, 391995, 415304, 440000, 466163, 493883,
    523251, 554365, 587329, 622253, 659255, 698456, 739988, 783990,
    830609, 880000, 932327, 987766,1046502,1108730,1174658,1244507,
   1318510,1396913,1479977,1567980,1661218,1760000,1864655,1975533,
   2093004,2217461,2349317,2489015,2637020,2793826,2959955,3135960,
   3322437,3520000,3729310,3951066,4186009,4434922,4698635,4978031,
   5274041,5587652,5919911,6271920,6644875,7040000,7458620,7902132
};
};
