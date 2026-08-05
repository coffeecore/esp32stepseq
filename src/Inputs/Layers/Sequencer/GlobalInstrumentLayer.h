#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include <Sequencer/Sequencer.h>

namespace Inputs::Layers::Sequencer
{
    class GlobalInstrumentLayer : public Layer
    {
    public:
        using Layer::Layer;

        void applyEncoderMapping() override
        {
            ::Sequencer::Track& track = layerContext.sequencer.tracks[uiState.selectedTrack];

            layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, Constants::NUMBER_OF_INSTRUMENTS - 1, false);

            layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, Constants::NUMBER_OF_INSTRUMENTS - 1, false);
        }

        void applyEncoderValues() override
        {
            ::Sequencer::Track& track = layerContext.sequencer.tracks[uiState.selectedTrack];

            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, track.instrument);

            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, uiState.selectedInstrument);
        }

        void onEncoder(InputEvent& inputEvent) override
        {
            if (inputEvent.control == ControlId::Encoder0) {
                layerContext.sequencer.setTrackInstrument(uiState.selectedTrack, inputEvent.value);
            }

            if (inputEvent.control == ControlId::Encoder1) {
                uiState.selectedInstrument = inputEvent.value;
            }
        }
    };
} // namespace Inputs::Layers::Sequencer
