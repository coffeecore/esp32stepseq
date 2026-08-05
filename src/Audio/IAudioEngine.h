#pragma once
#include "ESP32Synth.h"

#include "PlayNoteRequest.h"
#include "VoiceHandle.h"
#include "Sample/SampleLoader.h"

enum class InstrumentSource : uint8_t
{
    Wave,
    Sample
};

using AdsrState = uint8_t;

constexpr AdsrState ATTACK = 1u << 0;
constexpr AdsrState DECAY = 1u << 1;
constexpr AdsrState SUSTAIN = 1u << 2;
constexpr AdsrState RELEASE = 1u << 3;

struct Adsr
{
    uint16_t attackMs = 5;
    uint16_t decayMs = 150;
    uint8_t sustainLvl = 200;
    uint16_t releaseMs = 300;

    AdsrState state = 0;
};

// TODO: remplace par ta vraie structure/banque d'instruments. Elle n'existait
// pas dans le code fourni, donc ceci reste un placeholder minimal couvrant
// les deux cas (oscillateur interne / sample streamé) pour ne pas casser
// la compilation.
struct MyInstrument
{
    InstrumentSource source = InstrumentSource::Wave;

    Adsr adsr;

    // -- Cas Wave --
    WaveType wave = WAVE_SINE;

    // -- Cas Sample --
    uint32_t sampleRootPitch = 44000; // centiHz de la note d'origine du sample
    LoopMode sampleLoop = LOOP_OFF;
    uint16_t sampleId = 0;
};

class IAudioEngine
{
public:
    // SampleLoader& sampleLoader;
    MyInstrument instruments[Constants::NUMBER_OF_INSTRUMENTS];
    uint8_t instrumentsCount = 0;

    virtual ~IAudioEngine() = default;

    virtual void begin() = 0;

    virtual VoiceHandle play(const PlayNoteRequest& request) = 0;

    virtual void stop(VoiceHandle voice) = 0;

    virtual void addSample(uint16_t sampleId) = 0;

    virtual void updateInstrument(uint8_t index, const MyInstrument& instrument) = 0;
};
