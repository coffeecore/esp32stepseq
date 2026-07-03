#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class NavigationLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        uint8_t Tcount = max<uint8_t>(1, sequencerTimer.trackCounts);
        if (0 < Tcount-1) {
            rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, Tcount-1, true);
        }

        QuarterNote& quarterNote = sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote];
        uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
        if (0 < stepsCount-1) {
            rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, -1, stepsCount, true);
        }
    }

    void applyEncoderValues() override
    {
        uint8_t Tcount = max<uint8_t>(1, sequencerTimer.trackCounts);
        if (0 < Tcount-1) {
            rotaryEncoders.setEncoderValue(ControlId::Encoder0, sequencerTimer.selectedTrack);
        }

        QuarterNote& quarterNote = sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote];
        uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
        if (0 < stepsCount-1) {
            rotaryEncoders.setEncoderValue(ControlId::Encoder1, sequencerTimer.selectedStep);
        }
    }

    void onEncoder(const InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            sequencerTimer.setSelectedTrack(inputEvent.value);

            if (inputEvent.value >= (display.displayedTrack+2)) {
                display.displayedTrack = inputEvent.value-1;
            }

            if (inputEvent.value <= display.displayedTrack) {
                display.displayedTrack = inputEvent.value;
            }

            if (sequencerTimer.selectedStep >= sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount) {
                sequencerTimer.setSelectedStep(sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount-1);
            }

        }

        if (inputEvent.control == ControlId::Encoder1) {
            if (inputEvent.value < 0) {
                sequencerTimer.setSelectedQuarterNote(sequencerTimer.selectedQuarterNote-1);
                sequencerTimer.setSelectedStep(sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount-1);
            } else if (inputEvent.value >= sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount) {
                sequencerTimer.setSelectedQuarterNote(sequencerTimer.selectedQuarterNote+1);
                sequencerTimer.setSelectedStep(0);
            } else {
                sequencerTimer.setSelectedStep(inputEvent.value);
            }
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
    }

    void onButtonHold(const InputEvent& event) override
    {
    }
};
