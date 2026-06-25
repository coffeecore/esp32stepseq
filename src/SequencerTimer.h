#pragma once

#include "Arduino.h"
#include "Constants.h"
#include "HTimer.h"

typedef struct {
    bool state = false;

    uint8_t note = 60;
    char noteStr[6] = "C-4";
    uint32_t noteFreq = 26163;

    uint8_t length = 6;

    int8_t instrument = -1;
} Step;

typedef struct {
    uint8_t stepIndex = 0;
    uint8_t ticksByStep = 6;

    uint8_t nextStepTick = 0;

    Step steps[4];
    uint8_t stepsCount = 4;

} QuarterNote;

// typedef struct {
//     QuarterNote quarterNotes[4];
//     uint8_t quarterNotesCount = 4;
// } Pattern;

typedef struct {
    int8_t remainingTicks = 0;
    Step* step = nullptr;
    bool active = false;
} TrackNoteState;

typedef struct {
    // Pattern patterns[16];
    QuarterNote quarterNotes[64];
    int8_t transpose = 0;
    uint8_t volume = 255;

    uint8_t instrument = 0;
} Track;

class SequencerTimer
{
    public:
        uint8_t volume = 127;
        uint16_t bpm = Constants::DEFAULT_BPM;
        uint8_t selectedTrack = 0;
        uint8_t selectedQuarterNote = 0;
        uint8_t selectedStep = 0;
        uint8_t selectedInstrument = 0;
        uint8_t trackCounts = 0;
        uint8_t quarterNoteCounts = 0;

        Track tracks[Constants::MAX_TRACKS];
        uint8_t ppqn = Constants::DEFAULT_PPQN;


        SequencerTimer(HTimer& _timer): timer(_timer)
        {

        }

        void begin()
        {
            Serial.println("Start sequencer");
            instance = this;

            xTaskCreatePinnedToCore(
                sequencerTask,
                "SeqTaskTimer",
                8192,
                this,
                2,
                &xHandle,
                0
            );
            timer.setCallback(onTimerStatic);
            timer.begin(instance->computeTickDurationInMicroSeconds());
            Serial.println("Started sequencer");
        }

        static void IRAM_ATTR onTimerStatic()
        {
             if (instance == nullptr) {
                return;
            }

            TaskHandle_t handle = instance->xHandle;

            if (handle == nullptr) {
                return;
            }

            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            vTaskNotifyGiveFromISR(handle, &xHigherPriorityTaskWoken);

            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }

        uint64_t computeTickDurationInMicroSeconds()
        {
            return 60 * 1000 * 1000 / (ppqn * bpm);
        }

        void midiToName(uint8_t midi, char* buffer, size_t bufferSize)
        {
            static const char* names[] =
            {
                "C", "C#", "D", "D#", "E", "F",
                "F#", "G", "G#", "A", "A#", "B"
            };

            const char* note = names[midi % 12];
            uint8_t octave = (midi / 12) - 1;

            if (note[1] == '\0') {
                snprintf(buffer, bufferSize, "%s-%d", note, octave);
            } else {
                snprintf(buffer, bufferSize, "%s%d", note, octave);
            }
        }

        uint32_t midiToFreq(uint8_t note) {
            return (uint32_t)(44000.0 * pow(2.0, (note - 69) / 12.0));
        }

        /**
         * -- Getter/Setter selected ---
         */
        void setStepNote(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex, uint8_t value)
        {
            Step& step = tracks[trackIndex]
                    .quarterNotes[quarterNoteIndex]
                    .steps[stepIndex];
           step.note = value;
            midiToName(value, step.noteStr, sizeof(step.noteStr));
            step.noteFreq = midiToFreq(value);

        }

        void setSelectedTrack(uint8_t trackIndex)
        {
            if (trackIndex >= trackCounts) {
                return;
            }

            selectedTrack = trackIndex;
        }

        // void setSelectedPattern(uint8_t patternIndex)
        // {
        //     if (patternIndex >= patternCounts) {
        //         return;
        //     }

        //     selectedPattern = patternIndex;
        // }

        void setSelectedQuarterNote(uint8_t quarterNoteIndex)
        {
            // if (quarterNoteIndex >= 4) {
            if (quarterNoteIndex >= quarterNoteCounts) {
                return;
            }

            selectedQuarterNote = quarterNoteIndex;
        }

        void setSelectedStep(uint8_t stepIndex)
        {
            if (stepIndex >= tracks[selectedTrack].quarterNotes[selectedQuarterNote].stepsCount) {
            // if (stepIndex >= getQuarterNote(selectedTrack, selectedPattern, selectedQuarterNote).stepsCount) {
                return;
            }

            selectedStep = stepIndex;
        }


        /**
         * -- Getter/Setter ---
         */
        void setBpm(uint16_t _bpm)
        {
            if (_bpm == 0) {
                _bpm = 1;
            }

            bpm = _bpm;

            timer.setTickDurationInMicroSeconds(computeTickDurationInMicroSeconds());
        }

        void setPpqn(uint8_t _ppqn)
        {
            if (_ppqn == 0) {
                _ppqn = 1;
            }

            ppqn = _ppqn;

            timer.setTickDurationInMicroSeconds(computeTickDurationInMicroSeconds());
        }

        // void setCurrentPattern(uint8_t _currentPattern)
        // {
        //     if (_currentPattern >= patternCounts) {
        //         return;
        //     }

        //     currentPattern = _currentPattern;
        // }

        // Pattern& getPattern(uint8_t trackIndex, uint8_t patternIndex)
        // {
        //     return tracks[trackIndex].patterns[patternIndex];
        // }

        void setCurrentQuarterNote(uint8_t _currentQuarterNote)
        {
            if (_currentQuarterNote >= 4) {
                return;
            }

            currentQuarterNote = _currentQuarterNote;
        }

        // QuarterNote& getQuarterNote(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex)
        // {
        //     return tracks[trackIndex].patterns[patternIndex].quarterNotes[quarterNoteIndex];
        // }

        // Step& getStep(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
        // {
        //     return tracks[trackIndex].patterns[patternIndex].quarterNotes[quarterNoteIndex].steps[stepIndex];
        // }

        void setVolume(uint8_t _volume)
        {
            volume = _volume;
        }

        // void setPan(int8_t _pan)
        // {
        //     if (_pan < -10) {
        //         _pan = -10;
        //     }
        //     if (_pan > 10) {
        //         _pan = 10;
        //     }
        //     pan = _pan;
        // }

        Track& getTrack(uint8_t trackIndex)
        {
            return tracks[trackIndex];
        }

        /**
         * --- Adder ---
        */
        void addTrack()
        {
            if (trackCounts >= Constants::MAX_TRACKS) {
                return;
            }

            trackCounts++;
        }

        // void addPattern()
        // {
        //     if (patternCounts >= 16) {
        //         return;
        //     }

        //     patternCounts++;
        // }

        void addQuarterNote()
        {
            if (quarterNoteCounts >= 64) {
                return;
            }

            quarterNoteCounts++;
        }

        void addStep(uint8_t trackIndex, uint8_t quarterNoteIndex)
        {
            // if (quarterNoteIndex >= getPattern(trackIndex, patternIndex).quarterNotesCount) {
            if (quarterNoteIndex >= quarterNoteCounts) {
                return;
            }

            // if (getPattern(trackIndex, patternIndex).quarterNotes[quarterNoteIndex].stepsCount >= 4) {
            if (tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount >= 4) {
                return;
            }
            // Serial.println("addstep");
            // Serial.println(tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount);
            // getPattern(trackIndex, patternIndex).quarterNotes[quarterNoteIndex].stepsCount++;
            tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount++;

            tracks[trackIndex].quarterNotes[quarterNoteIndex].ticksByStep = ppqn / tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount;

            for (uint8_t i=0;i<tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount;i++) {
                // if (tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[i].length > tracks[trackIndex].quarterNotes[quarterNoteIndex].ticksByStep) {
                    tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[i].length = tracks[trackIndex].quarterNotes[quarterNoteIndex].ticksByStep;
                // }
            }
        }

        /**
         * --- Getter/Setter Track
         */
        void setTrackVolume(uint8_t _trackIndex, uint8_t _volume)
        {
            Track& track = getTrack(_trackIndex);

            track.volume = _volume;
        }

        void setTrackTranspose(uint8_t _trackIndex, int8_t _transpose)
        {
            if (_transpose < -12) {
                _transpose = -12;
            }

             if (_transpose > 12) {
                _transpose = 12;
            }

            Track& track = getTrack(_trackIndex);

            track.transpose = _transpose;
        }

        // void toggleStep(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
        // {
        //     // if (quarterNoteIndex >= getPattern(trackIndex, patternIndex).quarterNotesCount) {
        //     if (quarterNoteIndex >= quarterNoteCounts) {
        //         Serial.println("quarterNoteIndex >= quarterNoteCounts");
        //         return;
        //     }

        //     // if (getQuarterNote(trackIndex, patternIndex, quarterNoteIndex).stepsCount <= stepIndex) {
        //     if (tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount <= stepIndex) {
        //         Serial.println(tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount);
        //         Serial.println(trackIndex);
        //         Serial.println(quarterNoteIndex);
        //         Serial.println(stepIndex);
        //                         Serial.println("tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount <= stepIndex");
        //         return;
        //     }

        //     // getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state = !getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state;
        //     tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex].state = !tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex].state;
        // }

        void toggleStep(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
        {
            // if (quarterNoteIndex >= getPattern(trackIndex, patternIndex).quarterNotesCount) {
            if (quarterNoteIndex >= quarterNoteCounts) {
                // Serial.println("quarterNoteIndex >= quarterNoteCounts");
                return;
            }

            // if (getQuarterNote(trackIndex, patternIndex, quarterNoteIndex).stepsCount <= stepIndex) {
            if (tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount <= stepIndex) {
                // Serial.println(tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount);
                // Serial.println(trackIndex);
                // Serial.println(quarterNoteIndex);
                // Serial.println(stepIndex);
                //                 Serial.println("tracks[trackIndex].quarterNotes[quarterNoteIndex].stepsCount <= stepIndex");
                return;
            }

            // getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state = !getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state;
            tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex].state = !tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex].state;
        }

        static void sequencerTask(void* pvParameters)
        {
            SequencerTimer* seq = static_cast<SequencerTimer*>(pvParameters);
            for (;;) {
                uint32_t pending = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

                while (pending--)
                {
                    bool quarterChanged = false;
                     if (seq->currentTick >= seq->ppqn) {
                        seq->currentTick = 0;
                        seq->currentQuarterNote++;
                        quarterChanged = true;

                        // if (seq->currentQuarterNote >= 4) {
                        if (seq->currentQuarterNote >= seq->quarterNoteCounts) {
                            seq->currentQuarterNote = 0;

                            // seq->currentPattern++;

                            // if (seq->currentPattern >= seq->patternCounts) {
                            //     seq->currentPattern = 0;
                            // }
                        }
                    }

                    if (quarterChanged) {
                        for (uint8_t i = 0; i < seq->trackCounts; i++) {
                            // QuarterNote& quarterNote = seq->tracks[i].patterns[seq->currentPattern].quarterNotes[seq->currentQuarterNote];
                            QuarterNote& quarterNote = seq->tracks[i].quarterNotes[seq->currentQuarterNote];

                            quarterNote.stepIndex = 0;
                            quarterNote.nextStepTick = 0;
                        }
                    }

                    uint8_t tick = seq->currentTick;
                    seq->currentTick++;

                    seq->process(tick);
                }
            }
        }

        void process(uint8_t tick)
        {
            processNoteOffs();

            for (uint8_t i = 0; i < trackCounts; i++) {
                Track& track = tracks[i];
                processPattern(tick, i, track);
            }
        }

        bool processPattern(uint8_t tick, uint8_t trackIndex, Track& track)
        {
            // QuarterNote& quarterNote = track.patterns[currentPattern].quarterNotes[currentQuarterNote];
            QuarterNote& quarterNote = track.quarterNotes[currentQuarterNote];

            int8_t diff = (tick - quarterNote.nextStepTick);

            if (diff < 0) {
                return false;
            }

            quarterNote.nextStepTick += quarterNote.ticksByStep;

            if (diff > quarterNote.ticksByStep * 2) {
                quarterNote.nextStepTick = tick;
            }

            return advancePattern(trackIndex, quarterNote);
        }

        bool advancePattern(uint8_t trackIndex, QuarterNote& quarterNote)
        {
            uint8_t current = quarterNote.stepIndex;
            Step& step = quarterNote.steps[current];
            current++;
            bool wrap = (current >= quarterNote.stepsCount);

            if (wrap) {
                current = 0;
            }

            quarterNote.stepIndex = current;

            if (step.state) {

                if (trackNoteStates[trackIndex].active) {
                    triggerStepOff(*trackNoteStates[trackIndex].step);
                }
                triggerStepOn(step);

                trackNoteStates[trackIndex].remainingTicks = step.length;
                trackNoteStates[trackIndex].step = &step;
                trackNoteStates[trackIndex].active = true;
            }

            return wrap;
        }

        void processNoteOffs()
        {
            for (uint8_t i = 0; i < trackCounts; i++) {

                if (!trackNoteStates[i].active) {
                    continue;
                }

                trackNoteStates[i].remainingTicks--;

                if (trackNoteStates[i].remainingTicks <= 0) {
                    triggerStepOff(*trackNoteStates[i].step);
                    trackNoteStates[i].active = false;
                }
            }
        }

        void triggerStepOff(Step& step)
        {
            // MIDI note off / stop voice
            // Serial.print("NOTE OFF : ");
            // Serial.println(millis());
        }


        void triggerStepOn(Step& step)
        {
            // MIDI / GPIO / synth trigger
            // Serial.print("NOTE ON : ");
            // Serial.println(millis());
        }
    
    private:
        HTimer& timer;
        uint8_t currentTick = 0;
        TaskHandle_t xHandle = nullptr;
        static SequencerTimer* instance;

        uint8_t currentPattern = 0;
        uint8_t currentQuarterNote = 0;

        // uint8_t patternCounts = 0;

        // uint8_t pendingPatternChange = 0;
        int8_t nextPattern = 0;


        TrackNoteState trackNoteStates[Constants::MAX_TRACKS];
};

SequencerTimer* SequencerTimer::instance = nullptr;
