#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include <Sequencer/Sequencer.h>

namespace Inputs::Layers::Sequencer
{
class StepInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        ::Sequencer::Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row]
                         .quarterNotes[uiState.selectedQuarterNote]
                         .steps[col];

        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 11, true);
    }

    void applyEncoderValues() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        ::Sequencer::Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row]
                         .quarterNotes[uiState.selectedQuarterNote]
                         .steps[col];

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, step.instrument);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        ::Sequencer::Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row]
                         .quarterNotes[uiState.selectedQuarterNote]
                         .steps[col];

        if (inputEvent.control == ControlId::Encoder1) {
            layerContext.sequencer.setStepInstrument(uiState.displayedTrack + row, uiState.selectedQuarterNote, col,
                                                     inputEvent.value);
        }
    }
};
} // namespace Input::Layer::Sequencer
