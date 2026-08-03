#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include "Display/Workspace.h"
#include "Inputs/Layers/Common/GloabalLayer.h"

namespace Inputs::Layers::InstrumentAdsr
{

class GlobalLayer : public Common::GlobalLayer
{
public:
    using Common::GlobalLayer::GlobalLayer;

    // void applyEncoderMapping() override
    // {
    //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
    //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 1, 999, false);
    // }

    // void applyEncoderValues() override
    // {
    //     layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, layerContext.sequencer.volume);
    //     layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, layerContext.sequencer.bpm);
    // }

    // void onEncoder(InputEvent& inputEvent) override
    // {
    //     if (inputEvent.control == ControlId::Encoder0) {
    //         layerContext.sequencer.setVolume(inputEvent.value);
    //     }

    //     if (inputEvent.control == ControlId::Encoder1) {
    //         layerContext.sequencer.setBpm(inputEvent.value);
    //     }
    // }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control) {
            case ControlId::Fn2:
                layerContext.menuManager.back();

                return;
        }

        
    }

    void onStepPressed(const InputEvent& inputEvent) override
    {
    }
};
} // namespace Input::Layer::InstrumentAdsr
