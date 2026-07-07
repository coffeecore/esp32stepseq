#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "HTimer.h"
#include "SequencerTimer.h"
#include "AiEsp32RotaryEncoder.h"
#include "Input/Input.h"
#include "Display/DisplayEngine.h"
#include "Input/InputEngine.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 21, /* data=*/ 22);

HTimer hTimer;

UIState uiState;

SequencerTimer sequencerTimer(hTimer, uiState);

RotaryEncoder rotaryEncoders;

DisplayEngine displayEngine(u8g2, uiState, sequencerTimer);

InputEngine inputEngine(sequencerTimer, rotaryEncoders, uiState);

Input input(sequencerTimer, inputEngine, rotaryEncoders);

void setup() {

    Serial.begin(115200);

    Wire.begin(21, 22);

    if (!u8g2.begin()) {
        Serial.println("OLED failed");

        while (true);
    }


    sequencerTimer.begin();

    Serial.println("Add track and quarter notes");

    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    sequencerTimer.addTrack();
    
    sequencerTimer.addQuarterNote();
    sequencerTimer.addQuarterNote();
    sequencerTimer.addQuarterNote();

    displayEngine.begin();

    rotaryEncoders.begin();

    inputEngine.begin();

    input.begin();
}

void loop() {
}
