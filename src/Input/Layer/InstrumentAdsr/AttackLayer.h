#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class AttackLayer : public Layer
{
    public:
        using Layer::Layer;
        
        void applyEncoderMapping() override
        {
            layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, true);
        }
        
        void applyEncoderValues() override
        {
            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, uiState.selectedTrack);
        }
        
        void onEncoder(InputEvent& inputEvent) override
        {
            if (inputEvent.control == ControlId::Encoder0) {
            }
        }
        
        void onButtonTap(const InputEvent& event) override
        {
        }
        
        void onButtonHold(const InputEvent& event) override
        {
        }
};
