#pragma once

#include "Input/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"
#include "Input/Layer/LayerContext.h"
#include "Input/Layer/LayerGroup.h"
#include "Input/Layer/Menu/MenuLayer.h"

class MenuLayerGroup: public LayerGroup
{
    public:
        MenuLayer menu;

        MenuLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, UIState& _ui)
            : LayerGroup(_inputContext, _layerContext, _ui),
            menu(_inputContext, _layerContext, _ui)
        {
        }

        Layer* layer() override
        {   
            Serial.println("MENU LAYER");
            return &menu;
        }
};
