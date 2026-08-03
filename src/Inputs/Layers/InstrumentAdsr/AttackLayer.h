#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"

namespace Inputs::Layers::InstrumentAdsr
{
class AttackLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 1000, false, 250);
    }

    void applyEncoderValues() override
    {
        Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, adsr.attackMs);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

            adsr.attackMs = inputEvent.value;
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        if (event.control != ControlId::Fn4) {
            return;
        }

        Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

        adsr.state ^= ATTACK;
    }
};
} // namespace Input::Layer::InstrumentAdsr
