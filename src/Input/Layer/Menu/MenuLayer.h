#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"
#include "Menu/MenuItem.h"

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
            case ControlId::Fn2:
                layerContext.menuManager.back();

                break;

            case ControlId::Fn3:
                layerContext.menuManager.enter();

                break;
        }
    }
};
