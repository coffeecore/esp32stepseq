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

constexpr EnumOption loopOptions[] = {
    { static_cast<int8_t>(LoopMode::LOOP_OFF), "Off" },
    { static_cast<int8_t>(LoopMode::LOOP_FORWARD), "Forward" },
    { static_cast<int8_t>(LoopMode::LOOP_PINGPONG), "Pingpong" },
    { static_cast<int8_t>(LoopMode::LOOP_REVERSE), "Reverse" }
};
constexpr EnumDescriptor loopDescriptor(
    loopOptions,
    Constants::countof(loopOptions)
);


// Simple selecteur numerique 0..MAX_SAMPLES-1 (pas de nom de fichier /
// picker dans ce menu pour l'instant, juste l'index du sample).
constexpr IntDescriptor sampleIdDescriptor(0, MAX_SAMPLES - 1);

constexpr IntDescriptor sampleRootPitchDescriptor(0, 44000, 100);

// --- ---

class InstrumentMenuScreen : public MenuScreen
{
public:
    Menu menu;

    int8_t source;
    int8_t wave;
    int32_t sampleId;
    int32_t sampleRootPitch;
    int8_t sampleLoop;

    char title[32];

    InstrumentMenuScreen(UIState& u,
                     Sequencer& s,
                     U8G2& d,
                     IAudioEngine& a, MenuManager& m)
        : MenuScreen(u, s, d, a, m),
        menu()
    {
    }

    // Appele par DisplayEngine a chaque changement d'overlay. Ne fait
    // quelque chose que si c'est le menu qui s'ouvre (DisplayEngine ne
    // filtre plus sur Menu specifiquement, c'est a l'ecran de le faire).
    void onEnter(UIOverlay overlay) override
    {
        if (overlay != UIOverlay::Menu) {
            return;
        }

        snprintf(title, sizeof(title),
                 "Instrument %u",
                 uiState.selectedInstrument + 1);

        menu.setTitle(title);

        MyInstrument& instrument = audioEngine.instruments[uiState.selectedInstrument];

        source = static_cast<int8_t>(instrument.source);
        wave = static_cast<int8_t>(instrument.wave);
        sampleId = static_cast<int32_t>(instrument.sampleId);
        sampleRootPitch = static_cast<int32_t>(instrument.sampleRootPitch);
        sampleLoop = static_cast<int8_t>(instrument.sampleLoop);

        buildMenu();
        menuManager.begin(&menu);
    }

    // Appele par DisplayEngine a chaque changement d'overlay. On ne
    // reporte les valeurs vers l'instrument reel que si c'est le menu
    // qu'on quitte (le menu ne modifie que les copies locales source/
    // wave/sampleId, jamais l'instrument directement : MenuValue ne
    // connait que des int8_t*/int32_t*, pas des enums specifiques comme
    // InstrumentSource/WaveType). On ecrit les 3 sans condition : le champ
    // non pertinent (wave si Sample, sampleId si Wave) garde simplement sa
    // valeur d'origine, ecrire dessus est sans effet.
    void onExit(UIOverlay overlay) override
    {
        if (overlay != UIOverlay::Menu) {
            return;
        }

        MyInstrument& instrument = audioEngine.instruments[uiState.selectedInstrument];

        instrument.source = static_cast<InstrumentSource>(source);
        instrument.wave = static_cast<WaveType>(wave);
        instrument.sampleId = static_cast<int32_t>(sampleId);
        instrument.sampleRootPitch = static_cast<int32_t>(sampleRootPitch);
        instrument.sampleLoop = static_cast<LoopMode>(sampleLoop);
    }

    void draw() override
    {
        if (uiState.uiOverlay == UIOverlay::Menu) {
            // Source a change (via le menu) depuis le dernier build : on
            // reconstruit la liste pour faire apparaitre/disparaitre
            // WaveType vs Sample. Peu couteux (2 addXxx max) et on ne le
            // fait que quand c'est necessaire, pas a chaque frame.
            if (source != _lastBuiltSource) {
                buildMenu();
            }

            drawMenu();

            return;
        }
    }

private:
    // Sentinelle invalide pour forcer le tout premier build dans onEnter().
    int8_t _lastBuiltSource = -1;

    // Reconstruit entierement les items du menu depuis les copies locales.
    // Menu::clear() ne fait que remettre le compteur d'items a 0 (pas de
    // heap), donc rappeler menu.addXxx() ensuite est bon marche. On ne
    // touche pas a menuManager ici : le pointeur vers `menu` reste valide,
    // et sa selection courante n'est pas resetee (contrairement a
    // menuManager.begin(), qu'on n'appelle QUE dans onEnter()).
    void buildMenu()
    {
        menu.clear();

        menu.addEnum("Source", &source, &sourceDescriptor);

        if (static_cast<InstrumentSource>(source) == InstrumentSource::Wave) {
            menu.addEnum("WaveType", &wave, &waveTypeDescriptor);
        } else {
            menu.addInt("Sample", &sampleId, &sampleIdDescriptor);
            menu.addInt("Rootpitch", &sampleRootPitch, &sampleRootPitchDescriptor);
            menu.addEnum("Loop", &sampleLoop, &loopDescriptor);
        }

        menu.addAction("ADSR", openAdsr, this);        

        _lastBuiltSource = source;
    }

    static void openAdsr(void* ctx, MenuManager& menuManager)
    {
        InstrumentMenuScreen* screen = static_cast<InstrumentMenuScreen*>(ctx);
        screen->uiState.workspace = Workspace::InstrumentAdsr;
    }
};
