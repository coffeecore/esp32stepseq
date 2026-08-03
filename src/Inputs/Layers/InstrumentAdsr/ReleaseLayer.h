#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"

namespace Inputs::Layers::InstrumentAdsr
{

class ReleaseLayer : public Layer
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

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, adsr.releaseMs);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

            adsr.releaseMs = inputEvent.value;
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        if (event.control != ControlId::Fn7) {
            return;
        }

        Adsr& adsr = layerContext.audioEngine.instruments[uiState.selectedInstrument].adsr;

        adsr.state ^= RELEASE;
    }
};
} // namespace Input::Layer::InstrumentAdsr
