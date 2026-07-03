#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class StepEditLayer : public Layer
{
    public:
        using Layer::Layer;

        void applyEncoderMapping() override
        {
            rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 11, true);
            rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, -1, 9, false);
        }

        void applyEncoderValues() override
        {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

            rotaryEncoders.setEncoderValue(ControlId::Encoder0, step.note % 12);
            rotaryEncoders.setEncoderValue(ControlId::Encoder1, step.note / 12 - 1);
        }

        void onEncoder(const InputEvent& event) override
        {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

            if (event.control == ControlId::Encoder0) {
                sequencerTimer.setStepNote(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, event.value, step.note / 12 - 1);
            }

            if (event.control == ControlId::Encoder1) {
                sequencerTimer.setStepNote(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, step.note % 12, event.value);
            }
        }

        void onStepReleased(const InputEvent& inputEvent) override
        {
            /** @implements toggle step state */
            uint8_t row = (static_cast<uint8_t>(inputEvent.control)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputEvent.control)) % 4;
            sequencerTimer.toggleStep(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col);    
        }
};
