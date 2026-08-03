#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include <Sequencer/Sequencer.h>

namespace Inputs::Layers::Sequencer
{
class StepLengthLayer : public Layer
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

        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 1,
                                                         layerContext.sequencer.tracks[uiState.displayedTrack + row]
                                                             .quarterNotes[uiState.selectedQuarterNote]
                                                             .ticksByStep,
                                                         false);
    }

    void applyEncoderValues() override
    {
        uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
        uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

        ::Sequencer::Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row]
                         .quarterNotes[uiState.selectedQuarterNote]
                         .steps[col];

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, step.length);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            uint8_t row = (static_cast<uint8_t>(inputContext.stepId)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputContext.stepId)) % 4;

            ::Sequencer::Step& step = layerContext.sequencer.tracks[uiState.displayedTrack + row]
                             .quarterNotes[uiState.selectedQuarterNote]
                             .steps[col];

            layerContext.sequencer.setStepLength(uiState.displayedTrack + row, uiState.selectedQuarterNote, col,
                                                 inputEvent.value);
        }
    }
};
} // namespace Input::Layer::Sequencer
