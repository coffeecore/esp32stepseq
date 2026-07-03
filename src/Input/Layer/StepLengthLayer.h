#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class StepLengthLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 1, sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].ticksByStep, false);
    }

    void applyEncoderValues() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

        rotaryEncoders.setEncoderValue(ControlId::Encoder0, step.length);     }

    void onEncoder(const InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

            sequencerTimer.setStepLength(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, inputEvent.value);
        }
    }
};
