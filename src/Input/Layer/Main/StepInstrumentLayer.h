#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class StepInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = layerContext.sequencerTimer.tracks[uiState.displayedTrack + row].quarterNotes[uiState.selectedQuarterNote].steps[col];

        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 11, true);
    }

    void applyEncoderValues() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = layerContext.sequencerTimer.tracks[uiState.displayedTrack + row].quarterNotes[uiState.selectedQuarterNote].steps[col];

        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, step.instrument);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        Step& step = layerContext.sequencerTimer.tracks[uiState.displayedTrack + row].quarterNotes[uiState.selectedQuarterNote].steps[col];

        if (inputEvent.control == ControlId::Encoder1) {
            layerContext.sequencerTimer.setStepInstrument(uiState.displayedTrack + row, uiState.selectedQuarterNote, col, inputEvent.value);
        }
    }
};
