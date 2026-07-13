#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class TrackLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, -12, 12, false);
    }

    void applyEncoderValues() override
    {
        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder0, layerContext.sequencer.tracks[uiState.selectedTrack].volume);
        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, layerContext.sequencer.tracks[uiState.selectedTrack].transpose);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            layerContext.sequencer.setTrackVolume(uiState.selectedTrack,inputEvent.value);
        }

        if (inputEvent.control == ControlId::Encoder1) {
            Serial.println("DEBUG TRANPOSE");
            Serial.println(inputEvent.value);
            layerContext.sequencer.setTrackTranspose(uiState.selectedTrack, inputEvent.value);
        }
    }
};
