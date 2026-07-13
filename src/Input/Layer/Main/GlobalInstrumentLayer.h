#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class GlobalInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        Track& track = layerContext.sequencer.tracks[uiState.selectedTrack];

        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, Constants::NUMBER_OF_INSTRUMENTS-1, false);

        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, Constants::NUMBER_OF_INSTRUMENTS-1, false);
    }

    void applyEncoderValues() override
    {
        Track& track = layerContext.sequencer.tracks[uiState.selectedTrack];

        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder0, track.instrument);

        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, uiState.selectedInstrument);
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
