#pragma once

#include <math.h>
#include "ESP32Synth.h"
#include "Constants.h"
#include "IAudioEngine.h"
#include "PlayNoteRequest.h"
#include "Audio/Notes.h"
#include "Sample/SampleLoader.h"

// #include "closed_hihat_44100hz.h"

const SampleZone closed_hihat_44100hz[] = {
    { c0, g10, 0, c4 } 
};

class ESP32SynthAudioEngine : public IAudioEngine
{
    public:
        explicit ESP32SynthAudioEngine(ESP32Synth& _synth, SampleLoader& _sampleLoader)
            : synth(_synth),
            sampleLoader(_sampleLoader)
        {
        }

        uint8_t nextSample = 0;
        int16_t samples[MAX_SAMPLES];

        void begin() override
        {
            // synth.begin(...) reste fait dans setup_audio() comme aujourd'hui.
            for (uint8_t i = 0; i < Constants::NUMBER_OF_VOICES; i++) {
                voiceActive[i] = false;
            }
        }

        uint8_t allocateSample()
        {
            for (uint8_t i = 0; i < MAX_SAMPLES; i++) {
                for (uint8_t j = 0; j < Constants::NUMBER_OF_INSTRUMENTS; j++) {
                    if (instruments[j].sampleId != samples[i]) {
                        return i;
                    }
                }
            }

            return 0;
        }

        virtual void addSample(uint16_t sampleId)
        {
            uint16_t esp32SampleId = allocateSample();

            samples[esp32SampleId] = sampleId;

            synth.registerSample(esp32SampleId, sampleLoader.getData(sampleId), sampleLoader.getLength(sampleId), sampleLoader.getRate(sampleId), c4);
        }

        void addInstrument(const MyInstrument& instrument)
        {
            if (instrumentsCount >= Constants::NUMBER_OF_INSTRUMENTS) {
                return;
            }

            instrumentsCount++;

            instruments[instrumentsCount] = instrument;
            addSample(instrument.sampleId);
        }

        VoiceHandle play(const PlayNoteRequest& request) override
        {
            uint8_t voice = allocateVoice();

            uint8_t instrumentIndex = request.instrument;
            if (instrumentIndex >= Constants::NUMBER_OF_INSTRUMENTS) {
                instrumentIndex = 0; // fallback sûr plutôt qu'un accès hors tableau
            }

            MyInstrument& inst = instruments[instrumentIndex];

            if (inst.source == InstrumentSource::Sample) {
                voiceSource[voice] = InstrumentSource::Sample;

                // TODO: nom de fonction NON confirmé — je n'ai pas trouvé la
                // signature exacte pour jouer un sample chargé en RAM
                // (distinct de playStream() qui lit depuis la SD en flux).
                // MAX_SAMPLES existe dans ESP32Synth_Config.hpp donc la
                // fonctionnalité existe, mais remplace l'appel ci-dessous par
                // la vraie signature trouvée dans ton ESP32Synth.h local.
                //
                // Attendu : un identifiant de sample pré-chargé (ex: via
                // synth.loadSample(index, data, length) fait une fois au
                // setup()), puis ici on le déclenche sur la voix :
                applyInstrumentSample(voice, inst);
                applyCommands(voice, request);

                uint32_t freqCentiHz = notesFreq[request.note];
                synth.noteOn(voice, freqCentiHz, request.velocity);
            } else {
                voiceSource[voice] = InstrumentSource::Wave;

                applyInstrument(voice, inst);
                applyCommands(voice, request);

                uint32_t freqCentiHz = notesFreq[request.note];
                synth.noteOn(voice, freqCentiHz, request.velocity);
            }

            voiceActive[voice] = true;

            return VoiceHandle{voice};
        }

        void stop(VoiceHandle voice) override
        {
            if (!voice.valid() || voice.id >= Constants::NUMBER_OF_VOICES) {
                return;
            }

            // TODO: pour le cas Sample, je n'ai pas trouvé de stopStream()
            // confirmé dans la doc consultée. Vérifie dans ton ESP32Synth.h
            // local s'il en existe une ; sinon noteOff(voice) coupe peut-être
            // aussi un stream (comportement à vérifier empiriquement).
            synth.noteOff(voice.id);
            voiceActive[voice.id] = false;
        }

    private:
        ESP32Synth& synth;
        SampleLoader& sampleLoader;

        bool voiceActive[Constants::NUMBER_OF_VOICES] = {false};
        InstrumentSource voiceSource[Constants::NUMBER_OF_VOICES] = {InstrumentSource::Wave};
        uint8_t nextVoice = 0;

        // Alloue une voix : prend la prochaine libre, ou vole la plus ancienne
        // (round-robin) si le pool est saturé. Toujours bornée à
        // Constants::NUMBER_OF_VOICES, jamais d'incrément infini.
        uint8_t allocateVoice()
        {
            for (uint8_t i = 0; i < Constants::NUMBER_OF_VOICES; i++) {
                uint8_t candidate = (nextVoice + i) % Constants::NUMBER_OF_VOICES;

                if (!voiceActive[candidate]) {
                    nextVoice = (candidate + 1) % Constants::NUMBER_OF_VOICES;
                    return candidate;
                }
            }

            // Aucune voix libre : vol de la plus ancienne (round-robin actuel).
            uint8_t stolen = nextVoice;
            synth.noteOff(stolen);
            nextVoice = (nextVoice + 1) % Constants::NUMBER_OF_VOICES;

            return stolen;
        }

        // Ne s'applique qu'aux instruments de type Wave (oscillateur interne).
        void applyInstrument(uint8_t voice, const MyInstrument& inst)
        {
            synth.setWave(voice, inst.wave);
            synth.setEnv(voice, inst.adsr.attackMs, inst.adsr.decayMs, inst.adsr.sustainLvl, inst.adsr.releaseMs);
        }

        void applyInstrumentSample(uint8_t voice, const MyInstrument& inst)
        {
            // Instrument_Sample inst_closed_hihat_44100hz = {
            //     closed_hihat_44100hz, // O const SampleZone de cima 
            //     1, // Quantas zonas
            //     inst.sampleLoop, // Modo de loop 
            //     0, // inicio do loop
            //     0  // fim do loop ( 0 = ultimo sample)
            // };
            // uint16_t a[] = {};

            // synth.registerSample(inst.sampleId, sampleLoader.getData(inst.sampleId, *a), sampleLoader.getLength(inst.sampleId), sampleLoader.getRate(inst.sampleId), c4);
            uint8_t sampleId = 0;
            for (uint8_t i = 0;i<MAX_SAMPLES;i++) {
                if (samples[i] == inst.sampleId) {
                    sampleId = i;

                    break;
                }
            }
            synth.setWave(voice, WAVE_SAMPLE);
            synth.setSample(voice, sampleId, inst.sampleLoop, 0, 0);
            // synth.setInstrument(voice, &inst_closed_hihat_44100hz);

            synth.setEnv(voice, inst.adsr.attackMs, inst.adsr.decayMs, inst.adsr.sustainLvl, inst.adsr.releaseMs);
        }

        void applyCommands(uint8_t voice, const PlayNoteRequest& request)
        {
            for (uint8_t i = 0; i < request.commandCount && i < MAX_COMMANDS; i++) {
                const Command& cmd = request.commands[i];

                switch (cmd.type) {
                    case CommandType::Arpeggio: {
                        // TODO: vérifie que cet encodage (offsets en demi-tons dans
                        // data[0..2], durée de pas dans data[3]) correspond à ce que
                        // tu mets réellement dans tes Steps. setArpeggio prend des
                        // NOTES explicites (pas des offsets), donc on les recalcule
                        // ici à partir de la note de base.
                        uint32_t n1 = notesFreq[request.note];
                        uint32_t n2 = notesFreq[request.note + cmd.data[0]];
                        uint32_t n3 = notesFreq[request.note + cmd.data[1]];
                        uint32_t n4 = notesFreq[request.note + cmd.data[2]];

                        synth.setArpeggio(voice, cmd.data[3], n1, n2, n3, n4);
                        break;
                    }

                    case CommandType::Portamento: {
                        // Pas de "setPortamento" dans l'API : on utilise slideFreqTo
                        // vers la note cible, sur la durée donnée par data[0] (ms).
                        uint32_t targetFreq = notesFreq[request.note];
                        synth.slideFreqTo(voice, targetFreq, cmd.data[0]);
                        break;
                    }

                    case CommandType::Vibrato: {
                        // data[0] = taux LFO en Hz, data[1] = profondeur en Hz
                        synth.setVibrato(voice, cmd.data[0] * 100, cmd.data[1] * 100);
                        break;
                    }

                    default:
                        break;
                }
            }
        }
};
