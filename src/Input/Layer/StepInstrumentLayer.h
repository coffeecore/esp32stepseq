#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class StepInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 11, true);
    }

    void applyEncoderValues() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

        rotaryEncoders.setEncoderValue(ControlId::Encoder1, step.instrument);
    }

    void onEncoder(const InputEvent& inputEvent) override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

        if (inputEvent.control == ControlId::Encoder1) {
            sequencerTimer.setStepInstrument(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, inputEvent.value);
        }
    }
};
