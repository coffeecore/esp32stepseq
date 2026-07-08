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

#include "ESP32Synth.h"

ESP32Synth synth;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 21, /* data=*/ 22);

HTimer hTimer;

UIState uiState;

SequencerTimer sequencerTimer(hTimer, uiState);

RotaryEncoder rotaryEncoders;

DisplayEngine displayEngine(u8g2, uiState, sequencerTimer);

InputEngine inputEngine(sequencerTimer, rotaryEncoders, uiState);

Input input(sequencerTimer, inputEngine, rotaryEncoders);

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
    // Triggers voice 0 at C4 (Middle C), Volume 255
    synth.noteOn(0, c4, 255);

    // Update frequency and pulse-width dynamically
    synth.setFrequency(0, cs4); // Shift pitch up to C#4
    synth.setWave(0, WAVE_PULSE);
    synth.setPulseWidth(0, 128); // 50% square duty cycle (0-255 scale)

    // Set custom bitcrush resolution (0-32 bits, 0 means disabled)
    synth.setMasterBitcrush(8); // Lo-fi 8-bit output reduction

    // Triggers envelope release stage
    synth.noteOff(0);
}
