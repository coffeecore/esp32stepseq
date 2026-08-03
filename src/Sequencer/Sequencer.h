#pragma once

#include "Arduino.h"
#include "Constants.h"
#include "HTimer.h"
#include "Display/Workspace.h"
#include "Audio/IAudioEngine.h"
#include "Audio/VoiceHandle.h"
#include "Audio/PlayNoteRequest.h"

namespace Sequencer
{
typedef struct
{
    bool state = false;

    uint8_t note = 60;

    uint8_t length = 6;

    int8_t instrument = -1;

    Command commands[2];

    uint8_t commandCount = 0;
} Step;

typedef struct
{
    uint8_t stepIndex = 0;
    uint8_t ticksByStep = 6;

    uint8_t nextStepTick = 0;

    Step steps[Constants::NUMBER_OF_STEPS];
    uint8_t stepsCount = Constants::NUMBER_OF_STEPS;

} QuarterNote;

typedef struct
{
    int8_t remainingTicks = 0;
    // Step* step = nullptr;
    bool active = false;
    VoiceHandle voiceHandle;
} TrackNoteState;

typedef struct
{
    QuarterNote quarterNotes[Constants::NUMBER_OF_QUARTER_NOTES];
    int8_t transpose = -12;
    uint8_t volume = 255;
    bool mute = false;

    uint8_t instrument = 0;
} Track;

enum class PlayState
{
    Play,
    Pause,
    Stop
};

class Sequencer
{
private:
    HTimer& timer;
    Display::UIState& uiState;
    IAudioEngine& audioEngine;
    uint8_t currentTick = 0;
    TaskHandle_t xHandle = nullptr;
    static Sequencer* instance;
    uint8_t currentPattern = 0;
    uint8_t currentQuarterNote = 0;
    int8_t nextPattern = 0;
    TrackNoteState trackNoteStates[Constants::NUMBER_OF_TRACKS];

public:
    uint8_t volume = 127;
    uint16_t bpm = Constants::DEFAULT_BPM;
    uint8_t trackCounts = 0;
    uint8_t quarterNoteCounts = 0;
    PlayState playState = PlayState::Play;
    Track tracks[Constants::NUMBER_OF_TRACKS];
    uint8_t ppqn = Constants::DEFAULT_PPQN;

    Sequencer(HTimer& _timer, Display::UIState& _uiState, IAudioEngine& _audioEngine)
        : timer(_timer)
        , uiState(_uiState)
        , audioEngine(_audioEngine)
    {
    }

    void begin()
    {
        Serial.println("Start sequencer");
        instance = this;

        xTaskCreatePinnedToCore(sequencerTask, "SeqTaskTimer", 8192, this, 2, &xHandle, 0);
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

    void setStepNoteMidi(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex, uint8_t value)
    {
        Step& step = tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex];

        step.note = value;
        // midiToName(value, step.noteStr, sizeof(step.noteStr));
        // step.noteFreq = midiToFreq(value);
    }

    void setStepNote(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex, uint8_t note, int8_t octave)
    {
        Step& step = tracks[trackIndex].quarterNotes[quarterNoteIndex].steps[stepIndex];

        uint8_t value = note + (octave + 1) * 12;

        step.note = value;
        // midiToName(value, step.noteStr, sizeof(step.noteStr));
        // step.noteFreq = midiToFreq(value);
    }

    void setStepLength(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex, uint8_t value)
    {
        QuarterNote& qn = tracks[trackIndex].quarterNotes[quarterNoteIndex];
        Step& step = qn.steps[stepIndex];

        if (value > qn.ticksByStep || value == 0) {
            return;
        }

        step.length = value;
    }

    void setStepInstrument(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex, int8_t value)
    {
        QuarterNote& qn = tracks[trackIndex].quarterNotes[quarterNoteIndex];
        Step& step = qn.steps[stepIndex];

        if (value >= 12 || value < -1) {
            return;
        }

        step.instrument = value;
    }

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
            _ppqn = 12;
        }

        ppqn = _ppqn;

        timer.setTickDurationInMicroSeconds(computeTickDurationInMicroSeconds());
    }

    void setQuarterNoteStepsCount(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t length)
    {
        if (trackIndex >= trackCounts) {
            return;
        }

        if (quarterNoteIndex >= quarterNoteCounts) {
            return;
        }

        QuarterNote& quarterNote = tracks[trackIndex].quarterNotes[quarterNoteIndex];

        quarterNote.stepsCount = length;
    }

    void setVolume(uint8_t _volume)
    {
        volume = _volume;
    }

    void togglePause()
    {
        if (playState == PlayState::Play) {
            playState = PlayState::Pause;

            return;
        }

        playState = PlayState::Play;
    }

    void toggleStop()
    {
        if (playState != PlayState::Stop) {
            playState = PlayState::Stop;

            currentTick = 0;
            currentQuarterNote = 0;

            return;
        }
    }

    void addTrack()
    {
        if (trackCounts >= Constants::NUMBER_OF_TRACKS) {
            return;
        }

        trackCounts++;
    }

    void addQuarterNote()
    {
        if (quarterNoteCounts < Constants::NUMBER_OF_QUARTER_NOTES) {
            quarterNoteCounts++;
        }
    }

    void removeQuarterNote()
    {
        if (quarterNoteCounts > 0) {
            quarterNoteCounts--;
        }
    }

    void addStep(uint8_t trackIndex, uint8_t quarterNoteIndex)
    {
        if (trackIndex >= trackCounts) {
            return;
        }
        if (quarterNoteIndex >= quarterNoteCounts) {
            return;
        }

        QuarterNote& quarterNote = tracks[trackIndex].quarterNotes[quarterNoteIndex];

        if (quarterNote.stepsCount >= Constants::NUMBER_OF_STEPS) {
            return;
        }
        quarterNote.stepsCount++;

        quarterNote.ticksByStep = ppqn / quarterNote.stepsCount;

        for (uint8_t i = 0; i < quarterNote.stepsCount; i++) {
            Step& step = quarterNote.steps[i];

            step.length = quarterNote.ticksByStep;
        }
    }

    void setTrackVolume(uint8_t _trackIndex, uint8_t _volume)
    {
        if (_trackIndex >= trackCounts) {
            return;
        }

        Track& track = tracks[_trackIndex];

        track.volume = _volume;
    }

    void setTrackTranspose(uint8_t _trackIndex, int8_t _transpose)
    {
        if (_trackIndex >= trackCounts) {
            return;
        }

        if (_transpose < -12) {
            _transpose = -12;
        }

        if (_transpose > 12) {
            _transpose = 12;
        }

        Track& track = tracks[_trackIndex];

        track.transpose = _transpose;
    }

    void setTrackInstrument(uint8_t _trackIndex, int8_t _instrument)
    {
        if (_trackIndex >= trackCounts) {
            return;
        }

        if (_instrument >= 12) {
            return;
        }

        Track& track = tracks[_trackIndex];

        track.instrument = _instrument;
    }

    void toggleTrackMute(uint8_t _trackIndex)
    {
        if (_trackIndex >= trackCounts) {
            return;
        }

        Track& track = tracks[_trackIndex];

        track.mute = !track.mute;
    }

    void toggleStep(uint8_t trackIndex, uint8_t quarterNoteIndex, uint8_t stepIndex)
    {
        if (quarterNoteIndex >= quarterNoteCounts) {
            return;
        }

        QuarterNote& quarterNote = tracks[trackIndex].quarterNotes[quarterNoteIndex];

        if (quarterNote.stepsCount <= stepIndex) {
            return;
        }

        Step& step = quarterNote.steps[stepIndex];

        step.state = !step.state;
    }

    static void sequencerTask(void* pvParameters)
    {
        Sequencer* seq = static_cast<Sequencer*>(pvParameters);
        for (;;) {
            uint32_t pending = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            while (pending--) {
                if (seq->playState != PlayState::Play) {
                    continue;
                }

                bool quarterChanged = false;
                if (seq->currentTick >= seq->ppqn) {
                    seq->currentTick = 0;
                    seq->currentQuarterNote++;
                    quarterChanged = true;

                    if (seq->currentQuarterNote >= seq->quarterNoteCounts) {
                        seq->currentQuarterNote = 0;
                    }
                }

                if (quarterChanged) {
                    for (uint8_t i = 0; i < seq->trackCounts; i++) {
                        QuarterNote& quarterNote = seq->tracks[i].quarterNotes[seq->currentQuarterNote];

                        quarterNote.stepIndex = 0;
                        quarterNote.nextStepTick = 0;
                    }
                }
                if (seq->uiState.autoScroll) {
                    seq->uiState.selectedQuarterNote = seq->currentQuarterNote;
                    seq->uiState.requestRedraw();
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
        Track& track = tracks[trackIndex];
        uint8_t current = quarterNote.stepIndex;
        Step& step = quarterNote.steps[current];

        if (uiState.autoScroll) {
            uiState.selectedStep = current;
            uiState.requestRedraw();
        }

        current++;
        bool wrap = (current >= quarterNote.stepsCount);

        if (wrap) {
            current = 0;
        }

        quarterNote.stepIndex = current;

        if (step.state) {
            TrackNoteState& trackNoteState = trackNoteStates[trackIndex];

            if (trackNoteState.active) {
                triggerStepOff(trackNoteState);
            }
            trackNoteState.remainingTicks = step.length;
            trackNoteState.active = true;

            triggerStepOn(step, track, trackNoteState);
        }

        return wrap;
    }

    void processNoteOffs()
    {
        for (uint8_t i = 0; i < trackCounts; i++) {
            TrackNoteState& trackNoteState = trackNoteStates[i];
            if (!trackNoteState.active) {
                continue;
            }

            trackNoteState.remainingTicks--;

            if (trackNoteState.remainingTicks <= 0) {
                triggerStepOff(trackNoteState);
                trackNoteState.active = false;
            }
        }
    }

    void triggerStepOff(TrackNoteState& trackNoteState)
    {
        // MIDI note off / stop voice
        Serial.print("TRIGGER NOTE OFF : ");
        Serial.println(millis());
        audioEngine.stop(trackNoteState.voiceHandle);
    }

    void triggerStepOn(Step& step, Track& track, TrackNoteState& trackNoteState)
    {
        // MIDI / GPIO / synth trigger
        Serial.print("TRIGGER NOTE ON : ");
        Serial.println(millis());

        PlayNoteRequest request;
        request.note = step.note;
        request.velocity = track.volume;
        request.instrument = step.instrument >= 0 ? step.instrument : track.instrument;
        request.gate = step.length;
        request.commandCount = step.commandCount;
        request.commands = step.commands;
        trackNoteState.voiceHandle = audioEngine.play(request);
    }
};

Sequencer* Sequencer::instance = nullptr;
}
