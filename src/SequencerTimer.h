#pragma once

#include "Arduino.h"
#include "Constants.h"
#include "HTimer.h"

typedef struct {
    bool state = false;
    uint8_t note = 127;
    uint8_t length = 6;
} Step;

typedef struct {
    uint8_t stepIndex = 0;
    uint8_t ticksByStep = 6;

    uint8_t nextStepTick = 0;

    Step steps[4];
    uint8_t stepsCount = 0;

} QuarterNote;

typedef struct {
    QuarterNote quarterNotes[4];
    uint8_t quarterNotesCount = 4;
} Pattern;

typedef struct {
    int8_t remainingTicks = 0;
    Step* step = nullptr;
    bool active = false;
} TrackNoteState;

typedef struct {
    Pattern patterns[16];
    int8_t transpose = 0;
    uint8_t volume = 255;
} Track;

class SequencerTimer
{
    public:
        uint8_t volume = 127;
        uint16_t bpm = Constants::DEFAULT_BPM;
        uint8_t selectedTrack = 0;
        uint8_t selectedPattern = 0;
        uint8_t selectedQuarterNote = 0;
        uint8_t selectedStep = 0;

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

        /**
         * -- Getter/Setter selected ---
         */
        void setSelectedTrack(uint8_t trackIndex)
        {
            selectedTrack = trackIndex;
        }

        void setSelectedPattern(uint8_t patternIndex)
        {
            if (patternIndex >= patternCounts) {
                return;
            }

            selectedPattern = patternIndex;
        }

        void setSelectedQuarterNote(uint8_t quarterNoteIndex)
        {
            if (quarterNoteIndex >= 4) {
                return;
            }

            selectedQuarterNote = quarterNoteIndex;
        }

        void setSelectedStep(uint8_t stepIndex)
        {
            if (stepIndex >= getQuarterNote(selectedTrack, selectedPattern, selectedQuarterNote).stepsCount) {
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

        void setCurrentPattern(uint8_t _currentPattern)
        {
            if (_currentPattern >= patternCounts) {
                return;
            }

            currentPattern = _currentPattern;
        }

        Pattern& getPattern(uint8_t trackIndex, uint8_t patternIndex)
        {
            return tracks[trackIndex].patterns[patternIndex];
        }

        void setCurrentQuarterNote(uint8_t _currentQuarterNote)
        {
            if (_currentQuarterNote >= 4) {
                return;
            }

            currentQuarterNote = _currentQuarterNote;
        }

        QuarterNote& getQuarterNote(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex)
        {
            return tracks[trackIndex].patterns[patternIndex].quarterNotes[quarterNoteIndex];
        }

        Step& getStep(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
        {
            return tracks[trackIndex].patterns[patternIndex].quarterNotes[quarterNoteIndex].steps[stepIndex];
        }

        void setVolume(uint8_t _volume)
        {
            volume = _volume;
        }

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

        void addPattern()
        {
            if (patternCounts >= 16) {
                return;
            }

            patternCounts++;
        }

        void addStep(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex)
        {
            if (quarterNoteIndex >= getPattern(trackIndex, patternIndex).quarterNotesCount) {
                return;
            }

            if (getPattern(trackIndex, patternIndex).quarterNotes[quarterNoteIndex].stepsCount >= 4) {
                return;
            }

            getPattern(trackIndex, patternIndex).quarterNotes[quarterNoteIndex].stepsCount++;
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

        void toggleStep(uint8_t trackIndex, uint8_t patternIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
        {
            if (quarterNoteIndex >= getPattern(trackIndex, patternIndex).quarterNotesCount) {
                return;
            }

            if (getQuarterNote(trackIndex, patternIndex, quarterNoteIndex).stepsCount <= stepIndex) {
                return;
            }

            getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state = !getStep(trackIndex, patternIndex, quarterNoteIndex, stepIndex).state;
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

                        if (seq->currentQuarterNote >= 4) {
                            seq->currentQuarterNote = 0;

                            seq->currentPattern++;

                            if (seq->currentPattern >= seq->patternCounts) {
                                seq->currentPattern = 0;
                            }
                        }
                    }

                    if (quarterChanged) {
                        for (uint8_t i = 0; i < seq->trackCounts; i++) {
                            QuarterNote& quarterNote = seq->tracks[i].patterns[seq->currentPattern].quarterNotes[seq->currentQuarterNote];

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
            QuarterNote& quarterNote = track.patterns[currentPattern].quarterNotes[currentQuarterNote];

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

        // void queuePattern(uint8_t index)
        // {
        //     nextPattern = index;
        // }

        // void process(uint8_t tick)
        // {
        //     processNoteOffs();

        //     bool wrap = false;

        //     for (uint8_t i = 0; i < trackCounts; i++) {
        //         // Serial.println("[SEQUENCER] PROCESS PATTERN");
        //         wrap = processPattern(tracks[i], tick, i);
        //         if (wrap) pendingPatternChange++;
        //     }

        //     queuePattern()

        //     if (pendingPatternChange == trackCounts && nextPattern >= 0) {
        //         setPattern(nextPattern, tick);
        //         nextPattern = -1;
        //     }
        // }

        // void setPattern(uint8_t index, uint32_t tick)
        // {
        //     currentPattern = index;

        //     for (uint8_t i = 0; i < trackCounts; i++) {

        //         if (index >= patternCounts) {
        //             continue;
        //         }

        //         Pattern& pattern = getPattern(i, index);

        //         pattern.quarterNotes[currentQuarterNote].stepIndex = 0;
        //         pattern.quarterNotes[currentQuarterNote].nextStepTick = tick;
        //     }
        // }

        // bool processPattern(Track& track, uint32_t tick, uint8_t trackIndex)
        // {
        //     Pattern& pattern = track.patterns[currentPattern];

        //     int32_t diff = (int32_t)(tick - pattern.quarterNotes[currentQuarterNote].nextStepTick);
        //     Serial.println("[SEQUENCER] PROCESS PATTERN diff");
        //     Serial.println(diff);
        //     Serial.println(currentQuarterNote);
        //     if (diff < 0) {
        //         return false;
        //     }

        //     pattern.quarterNotes[currentQuarterNote].nextStepTick += pattern.quarterNotes[currentQuarterNote].ticksByStep;

        //     // if (diff > pattern.quarterNotes[currentQuarterNote].ticksByStep * 2) {
        //     //     pattern.quarterNotes[currentQuarterNote].nextStepTick = tick;
        //     // }
        //         Serial.println("[SEQUENCER] ADVANCE PATTERN");
        //     return advancePattern(pattern, trackIndex);
        // }

        // bool advancePattern(Pattern& pattern, uint8_t trackIndex)
        // {
        //     uint8_t next = pattern.quarterNotes[currentQuarterNote].stepIndex + 1;
        //     bool wrap = (next >= pattern.quarterNotes[currentQuarterNote].stepsCount);

        //     if (wrap) {
        //         next = 0;
        //     }

        //     pattern.quarterNotes[currentQuarterNote].stepIndex = next;

        //     Step& step = pattern.quarterNotes[currentQuarterNote].steps[next];

        //     if (step.state) {

        //         if (trackNoteStates[trackIndex].active) {
        //             triggerStepOff(*trackNoteStates[trackIndex].step);
        //         }

        //         triggerStepOn(step);

        //         trackNoteStates[trackIndex].remainingTicks = step.length;
        //         trackNoteStates[trackIndex].step = &step;
        //         trackNoteStates[trackIndex].active = true;
        //     }

        //     return wrap;
        // }

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
            Serial.print("NOTE OFF : ");
            Serial.println(millis());
        }


        void triggerStepOn(Step& step)
        {
            // MIDI / GPIO / synth trigger
            Serial.print("NOTE ON : ");
            Serial.println(millis());
        }
    
    private:
        HTimer& timer;
        uint8_t currentTick = 0;
        TaskHandle_t xHandle = nullptr;
        static SequencerTimer* instance;
        uint8_t ppqn = Constants::DEFAULT_PPQN;

        uint8_t currentPattern = 0;
        uint8_t currentQuarterNote = 0;

        Track tracks[Constants::MAX_TRACKS];
        uint8_t trackCounts = 0;
        uint8_t patternCounts = 0;

        uint8_t pendingPatternChange = 0;
        int8_t nextPattern = 0;


        TrackNoteState trackNoteStates[Constants::MAX_TRACKS];
};

SequencerTimer* SequencerTimer::instance = nullptr;
