#pragma once

#include "Display/Workspace.h"
#include "Sequencer/Sequencer.h"
#include "Display/Screen/Screen.h"
// #include "Audio/Notes.h"
#include "Audio/IAudioEngine.h"
#include "Constants.h"
 #include "ESP32Synth.h"

// --- Menu configuration ---
constexpr EnumOption sourceOptions[] = {
    { static_cast<int8_t>(InstrumentSource::Wave), "Wave" },
    { static_cast<int8_t>(InstrumentSource::Sample), "Sample" }
};
constexpr EnumDescriptor sourceDescriptor(
    sourceOptions,
    Constants::countof(sourceOptions)
);

constexpr EnumOption waveTypeOptions[] = {
    { static_cast<int8_t>(WaveType::WAVE_NOISE), "Noise" },
    { static_cast<int8_t>(WaveType::WAVE_PULSE), "Pulse" },
    { static_cast<int8_t>(WaveType::WAVE_SAW), "Saw" },
    { static_cast<int8_t>(WaveType::WAVE_SINE), "Sine" },
    { static_cast<int8_t>(WaveType::WAVE_TRIANGLE), "Triangle" },
};
constexpr EnumDescriptor waveTypeDescriptor(
    waveTypeOptions,
    Constants::countof(waveTypeOptions)
);
// --- ---

class InstrumentScreen : public MenuScreen
{
public:
    Menu menu;

    int8_t source;
    int8_t wave;

    InstrumentScreen(UIState& u,
                     Sequencer& s,
                     U8G2& d,
                     IAudioEngine& a, MenuManager& m)
        : MenuScreen(u, s, d, a, m),
        menu("Instrument menu")
    {
    }

    void open()
    {
        menu.clear();
        MyInstrument& instrument = audioEngine.instruments[uiState.selectedInstrument];

        source = static_cast<int8_t>(instrument.source);
        wave = static_cast<int8_t>(instrument.wave);

        Serial.println("instruct");
        Serial.println(uiState.selectedInstrument);


        Serial.println("SOURCE");
        Serial.println(source);
        Serial.println("WAVE");
        Serial.println(wave);

        menu.addEnum("Source", &source, &sourceDescriptor);

        menu.addEnum("WaveType", &wave, &waveTypeDescriptor);

        menuManager.begin(&menu);
    }

    void draw() override
    {
        if (uiState.uiOverlay == UIOverlay::Menu) {
            drawMenu();

            return;
        }
    }
};
