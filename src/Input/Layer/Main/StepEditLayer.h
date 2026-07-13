#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class StepEditLayer : public Layer
{
    public:
        using Layer::Layer;

        void applyEncoderMapping() override
        {
            layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 11, true);
            layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, -1, 9, false);
        }

        void applyEncoderValues() override
        {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row].quarterNotes[uiState.selectedQuarterNote].steps[col];

            layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder0, step.note % 12);
            layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, step.note / 12 - 1);
        }

        void onEncoder(InputEvent& event) override
        {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row].quarterNotes[uiState.selectedQuarterNote].steps[col];

            if (event.control == ControlId::Encoder0) {
                layerContext.sequencer.setStepNote(uiState.displayedTrack + row, uiState.selectedQuarterNote, col, event.value, step.note / 12 - 1);
            }

            if (event.control == ControlId::Encoder1) {
                layerContext.sequencer.setStepNote(uiState.displayedTrack + row, uiState.selectedQuarterNote, col, step.note % 12, event.value);
            }
        }

        void onStepReleased(const InputEvent& inputEvent) override
        {
            uint8_t row = (static_cast<uint8_t>(inputEvent.control)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputEvent.control)) % 4;

            layerContext.sequencer.toggleStep(uiState.displayedTrack + row, uiState.selectedQuarterNote, col);    
        }
};
