#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Inputs/InputContext.h"
#include "Inputs/Layers/Layer.h"
#include "Inputs/Layers/LayerContext.h"
#include "Inputs/Layers/LayerGroup.h"
#include "Inputs/Layers/Menu/MenuLayer.h"

namespace Inputs::Layers
{

class MenuLayerGroup : public LayerGroup
{
public:
    Menu::MenuLayer menu;

    MenuLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, Display::UIState& _ui)
        : LayerGroup(_inputContext, _layerContext, _ui)
        , menu(_inputContext, _layerContext, _ui)
    {
    }

    Layer* layer() override
    {
        Serial.println("MENU LAYER");
        return &menu;
    }
};
} // namespace Input::Layer
