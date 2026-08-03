#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"

namespace Inputs::Layers::InstrumentAdsr
{

class GlobalInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, Constants::NUMBER_OF_INSTRUMENTS - 1, false);
    }

    void applyEncoderValues() override
    {
        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, uiState.selectedInstrument);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder1) {
            uiState.selectedInstrument = inputEvent.value;
        }
    }
};
} // namespace Input::Layer::InstrumentAdsr
