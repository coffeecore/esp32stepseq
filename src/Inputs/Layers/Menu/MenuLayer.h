#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include "Menu/MenuItem.h"

namespace Inputs::Layers::Menu
{

class MenuLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, -999, 999, true);
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, -999, 999, true);
    }

    void applyEncoderValues() override
    {
        MenuItem* menuItem = layerContext.menuManager.currentItem();

        if (nullptr == menuItem) {
            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, 0);
            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, 0);

            return;
        }

        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, layerContext.menuManager.selectedIndex());
        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, layerContext.menuManager.selectedIndex());
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        Serial.println("VALUE");
        Serial.println(inputEvent.delta);
        Serial.println(inputEvent.direction);
        if (inputEvent.direction > 0) {
            Serial.println("NEXT");
            layerContext.menuManager.next();

            return;
        }

        if (inputEvent.direction < 0) {
            Serial.println("PREV");
            layerContext.menuManager.prev();

            return;
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn1:
                uiState.workspace = Display::Workspace::Sequencer;
                uiState.uiOverlay = Display::UIOverlay::None;

                return;
            case ControlId::Fn2:
            {
                bool root = layerContext.menuManager.back();

                if (!root) {
                    if (uiState.workspace == Display::Workspace::InstrumentMenu) {
                        uiState.workspace = Display::Workspace::Sequencer;
                        uiState.uiOverlay = Display::UIOverlay::None;
                    }
                }

                break;
            }

            case ControlId::Fn3:
                layerContext.menuManager.enter();

                break;
        }
    }
};
}
