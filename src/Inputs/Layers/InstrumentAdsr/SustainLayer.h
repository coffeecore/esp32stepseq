#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"

namespace Inputs::Layers::InstrumentAdsr
{

class SustainLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
    }

    void applyEncoderValues() override
    {
        Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, adsr.sustainLvl);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

            adsr.sustainLvl = inputEvent.value;
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        if (event.control != ControlId::Fn6) {
            return;
        }

        Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

        adsr.state ^= SUSTAIN;
    }
};
} // namespace Input::Layer::InstrumentAdsr
