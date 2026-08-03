#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Inputs/InputContext.h"
#include "Inputs/Layers/Layer.h"
#include "Inputs/Layers/LayerContext.h"

namespace Inputs::Layers
{

class LayerGroup
{
public:
    InputContext& inputContext;
    Display::UIState& uiState;
    LayerContext& layerContext;

    explicit LayerGroup(InputContext& _inputContext, LayerContext& _layerContext, Display::UIState& _ui)
        : inputContext(_inputContext)
        , layerContext(_layerContext)
        , uiState(_ui)
    {
    }

    virtual Layer* layer() = 0;

    virtual ControlId buttonToControl(uint8_t button)
    {
        switch (button) {
            case 0:
                return ControlId::Fn0;
            case 1:
                return ControlId::Fn1;
            case 2:
                return ControlId::Fn2;
            case 3:
                return ControlId::Fn3;
            case 4:
                return ControlId::Fn4;
            case 5:
                return ControlId::Fn5;
            case 6:
                return ControlId::Fn6;
            case 7:
                return ControlId::Fn7;

            case 8:
                return ControlId::Step0;
            case 9:
                return ControlId::Step1;
            case 10:
                return ControlId::Step2;
            case 11:
                return ControlId::Step3;
            case 12:
                return ControlId::Step4;
            case 13:
                return ControlId::Step5;
            case 14:
                return ControlId::Step6;
            case 15:
                return ControlId::Step7;
            case 16:
                return ControlId::Step8;
            case 17:
                return ControlId::Step9;
            case 18:
                return ControlId::Step10;
            case 19:
                return ControlId::Step11;

            default:
                return ControlId::None;
        }
    }

    virtual ControlId encoderToControl(uint8_t encoder)
    {
        switch (encoder) {
            case 0:
                return ControlId::Encoder0;
            case 1:
                return ControlId::Encoder1;
            default:
                return ControlId::None;
        }
    }
};
} // namespace Input::Layer
