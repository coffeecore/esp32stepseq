#pragma once

#include "Input/InputMode.h"
#include "SequencerTimer.h"
#include "Input/RotaryEncoder.h"
#include "Input/InputContext.h"
#include "Display/Workspace.h"
#include "Input/Layer/LayerContext.h"

class Layer
{
    public:
        InputContext& inputContext;
        LayerContext& layerContext;
        UIState& uiState;

        explicit Layer(InputContext& _inputContext, LayerContext& _layerContext, UIState& ui)
            : inputContext(_inputContext),
            layerContext(_layerContext),
            uiState(ui)
        {
        }

        virtual void onStepPressed(const InputEvent& inputEvent) {}
        virtual void onStepReleased(const InputEvent& inputEvent) {}

        virtual void onEncoder(InputEvent& inputEvent) {}

        virtual void applyEncoderMapping() {}
        virtual void applyEncoderValues() {}

        virtual void onButtonTap(const InputEvent& event) {}
        virtual void onButtonHold(const InputEvent& event) {}
};
