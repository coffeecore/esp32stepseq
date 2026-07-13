#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "Sequencer/Sequencer.h"
#include "Sequencer/HTimer.h"
#include "AiEsp32RotaryEncoder.h"
#include "Input/Input.h"
#include "Display/DisplayEngine.h"
#include "Input/InputEngine.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Audio/ESP32SynthAudioEngine.h"
#include "Audio/Notes.h"


#include "ESP32Synth.h"

ESP32Synth synth;

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 21, /* data=*/ 22);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE
);
HTimer hTimer;

UIState uiState;

ESP32SynthAudioEngine esp32SynthAudioEngine(synth);

Sequencer sequencer(hTimer, uiState, esp32SynthAudioEngine);

RotaryEncoder rotaryEncoders;

DisplayEngine displayEngine(u8g2, uiState, sequencer);

InputEngine inputEngine(sequencer, rotaryEncoders, uiState);

Input input(sequencer, inputEngine, rotaryEncoders);

void setup_audio() {
    // Standard I2S Mode (External DAC like PCM5102A - BCK, WS, DATA)
    // Parameters: dataPin, mode, clkPin, wsPin, BitDepth
    // synth.begin(4, 15, 2, I2S_32BIT);

    // Or: Single-Pin Hardware PWM Mode (10-bit audio on pin 25)
    synth.begin(25, SMODE_PWM, -1, -1, I2S_16BIT);

    // Or: PDM Mode (High-Frequency 1-bit oversampled audio on pin 2)
    // synth.begin(2, SMODE_PDM, 4, -1, I2S_16BIT);

    // Set engine-wide volume (0-255 scaling)
    synth.setMasterVolume(255);
}

void setup() {
    Serial.begin(115200);

    Wire.begin(21,22);

// for (uint8_t addr = 1; addr < 127; addr++) {
//     Wire.beginTransmission(addr);
//     if (Wire.endTransmission() == 0) {
//         Serial.printf("I2C found: 0x%02X\n", addr);
//     }
// }
// return;

    Wire.begin();
    Wire.setClock(100000);

    if (!u8g2.begin()) {
        Serial.println("OLED failed");

        while (true);
    }

    initNotes();


    sequencer.begin();

    Serial.println("Add track and quarter notes");

    sequencer.addTrack();
    sequencer.addTrack();
    sequencer.addTrack();
    sequencer.addTrack();
    sequencer.addTrack();
    
    sequencer.addQuarterNote();
    sequencer.addQuarterNote();
    sequencer.addQuarterNote();

    displayEngine.begin();

    rotaryEncoders.begin();

    inputEngine.begin();

    input.begin();
}

void loop() {
}
