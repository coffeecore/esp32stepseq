#include <Arduino.h>
#include <Wire.h>

#include <U8g2lib.h>

#include "HTimer.h"
#include "SequencerTimer.h"
#include "AiEsp32RotaryEncoder.h"

#include "Input/Input.h"
#include "Input/Mode/MainInputMode.h"
#include "Input/Mode/StepInputMode.h"
#include "Input/Mode/QuarterNoteInputMode.h"


#include "Display/Display.h"
#include "Display/Mode/MainDisplayMode.h"
#include "Display/Mode/StepDisplayMode.h"
#include "Display/Mode/QuarterNoteDisplayMode.h"

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t  SCREEN_HEIGHT = 64;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 21, /* data=*/ 22);

HTimer hTimer;

SequencerTimer sequencerTimer(hTimer);

Display display(sequencerTimer, u8g2);

Input input(sequencerTimer);


MainInputMode mainInputMode(input, sequencerTimer, display);
StepInputMode stepInputMode(input, sequencerTimer, display);
QuarterNoteInputMode quarterNoteInputMode(input, sequencerTimer, display);

MainDisplayMode mainDisplayMode(display, sequencerTimer);
StepDisplayMode stepDisplayMode(display, sequencerTimer);
QuarterNoteDisplayMode quarterNoteDisplayMode(display, sequencerTimer);

void setup() {

    Serial.begin(115200);

    Wire.begin(21, 22);

    if (!u8g2.begin()) {
        Serial.println("OLED failed");

        while (true);
    }


    sequencerTimer.begin();

    Serial.println("Add track and pattern");

    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    // sequencerTimer.addPattern();
    sequencerTimer.addQuarterNote();
    sequencerTimer.addQuarterNote();
    // sequencerTimer.addQuarterNote();
    // sequencerTimer.addQuarterNote();
    // sequencerTimer.addQuarterNote();
    // for (uint8_t i = 0;i<Constants::MAX_TRACKS;i++) {
    //     for (uint8_t j = 0;j<4;j++) {
    //         // sequencerTimer.addStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, i);
    //         sequencerTimer.addStep(i, sequencerTimer.selectedPattern, 0);
    //     }
    // }
    sequencerTimer.addStep(0, 0);
    sequencerTimer.addStep(0, 0);
    sequencerTimer.addStep(0, 0);
    sequencerTimer.addStep(0, 0);

    sequencerTimer.addStep(1, 0);
    sequencerTimer.addStep(1, 0);
    sequencerTimer.addStep(1, 0);
    sequencerTimer.tracks[1].quarterNotes->stepsCount = 3;

    sequencerTimer.addStep(2, 0);
    sequencerTimer.addStep(2, 0);

    sequencerTimer.addStep(3, 0);

    sequencerTimer.addStep(4, 0);
    sequencerTimer.addStep(4, 0);
    sequencerTimer.addStep(4, 0);
    sequencerTimer.addStep(4, 0);



    sequencerTimer.toggleStep(0, 0, 0);
    sequencerTimer.toggleStep(0, 0, 2);
    sequencerTimer.toggleStep(0, 0, 3);

    sequencerTimer.toggleStep(1, 0, 0);
    sequencerTimer.toggleStep(1, 0, 2);

    sequencerTimer.toggleStep(2, 0, 0);

    sequencerTimer.toggleStep(3, 0, 0);

    sequencerTimer.toggleStep(4, 0, 1);
    sequencerTimer.toggleStep(4, 0, 2);
    sequencerTimer.toggleStep(4, 0, 4);

    display.registerMode(DisplayModes::Main, &mainDisplayMode);
    display.registerMode(DisplayModes::Step, &stepDisplayMode);
    display.registerMode(DisplayModes::QuarterNote, &quarterNoteDisplayMode);

    display.begin();

    input.attachDisplayTask(
        display.getTaskHandle()
    );

    input.registerMode(InputModes::Main, &mainInputMode);
    input.registerMode(InputModes::Step, &stepInputMode);
    input.registerMode(InputModes::QuarterNote, &quarterNoteInputMode);

    input.begin();
    // input.setMode(InputModes::Main);

    // input.setMode(&mainInputMode);

    // Serial.println("Add track and pattern");

    // sequencerTimer.addTrack();
    // sequencerTimer.addTrack();
    // sequencerTimer.addTrack();
    // sequencerTimer.addTrack();
    // // sequencerTimer.addPattern();
    // sequencerTimer.addQuarterNote();
    // // sequencerTimer.addQuarterNote();
    // // sequencerTimer.addQuarterNote();
    // // sequencerTimer.addQuarterNote();
    // for (uint8_t i = 0;i<Constants::MAX_TRACKS;i++) {
    //     for (uint8_t j = 0;j<4;j++) {
    //         // sequencerTimer.addStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, i);
    //         sequencerTimer.addStep(i, sequencerTimer.selectedPattern, 0);
    //     }
    // }

    // sequencerTimer.toggleStep(0, sequencerTimer.selectedPattern, 0, 2);
    // sequencerTimer.toggleStep(2, sequencerTimer.selectedPattern, 0, 2);
    // sequencerTimer.toggleStep(2, sequencerTimer.selectedPattern, 0, 3);
    // sequencerTimer.toggleStep(3, sequencerTimer.selectedPattern, 0, 0);


}

void loop() {
}


// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64

// Adafruit_SSD1306 display(
//   SCREEN_WIDTH,
//   SCREEN_HEIGHT,
//   &Wire,
//   -1
// );

// // GPIO boutons
// const int buttons[8] = {
//   13, 12, 14, 27,
//   26, 25, 33, 32
// };

// bool lastState[8];

// void setup() {

//   Serial.begin(115200);

//   // I2C OLED
//   Wire.begin(21, 22);

//   // OLED SSD1306
//   if (!display.begin(
//         SSD1306_SWITCHCAPVCC,
//         0x3C
//       )) {

//     Serial.println("OLED failed");

//     while (true);
//   }

//   display.clearDisplay();
//   display.setTextColor(SSD1306_WHITE);
//   display.setTextSize(1);

//   // boutons
//   for (int i = 0; i < 8; i++) {

//     pinMode(buttons[i], INPUT_PULLUP);

//     lastState[i] =
//       digitalRead(buttons[i]);
//   }

//   display.setCursor(0, 0);
//   display.println("ESP32 READY");
//   display.display();

//   delay(1000);
// }

// void loop() {

//   bool changed = false;

//   // check changements
//   for (int i = 0; i < 8; i++) {

//     bool current =
//       digitalRead(buttons[i]);

//     if (current != lastState[i]) {

//       changed = true;

//       if (current == LOW) {

//         Serial.printf(
//           "BTN %d pressed\n",
//           i
//         );

//       } else {

//         Serial.printf(
//           "BTN %d released\n",
//           i
//         );
//       }

//       lastState[i] = current;
//     }
//   }

//   // refresh écran seulement si changement
//   if (changed) {

//     display.clearDisplay();

//     display.setCursor(0, 0);
//     display.println("BUTTON STATES");

//     for (int i = 0; i < 8; i++) {

//       bool pressed =
//         lastState[i] == LOW;

//       display.setCursor(
//         0,
//         10 + i * 7
//       );

//       display.print("BTN ");
//       display.print(i);
//       display.print(": ");

//       if (pressed) {
//         display.println("ON");
//       } else {
//         display.println("OFF");
//       }
//     }

//     display.display();
//   }

//   delay(10);
// }
