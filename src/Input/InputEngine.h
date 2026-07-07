#pragma once

#include "Input/Layer/Layer.h"
#include "Input/Layer/LayerGroup.h"
#include "Input/Layer/MainLayerGroup.h"
#include "Input/InputMode.h"
#include "Display/Workspace.h"
#include "Input/Layer/LayerContext.h"

class InputEngine
{
    public:
        InputContext ctx;

        UIState& ui;

        LayerContext layerContext;

        SequencerTimer& sequencerTimer;
        RotaryEncoder& rotaryEncoders;

        MainLayerGroup mainLayerGroup;

        LayerGroup* currentLayerGroup = nullptr;

        Layer* pressedLayer[Constants::NUMBER_OF_BUTTONS] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};

        explicit InputEngine(SequencerTimer& _sequencerTimer, RotaryEncoder& _rotaryEncoders, UIState& _ui)
            :
            ui(_ui),
            layerContext(_sequencerTimer, _rotaryEncoders),
            sequencerTimer(_sequencerTimer),
            rotaryEncoders(_rotaryEncoders),
            mainLayerGroup(ctx, layerContext, ui)
        {
        }

        void begin()
        {
            resolveLayer();
            currentLayerGroup->layer()->applyEncoderMapping();
            currentLayerGroup->layer()->applyEncoderValues();
            ui.requestRedraw();
        }

        void resolveLayer()
        {
            switch (ui.workspace)
            {
                case Workspace::Sequencer:
                    currentLayerGroup = &mainLayerGroup;

                    break;
            }
        }

    void handleEvent(InputEvent& event)
    {
        switch (event.type)
        {
            case InputEventType::EncoderTurned:
            {
                event.control = currentLayerGroup->encoderToControl(event.id);
                break;
            }
            case InputEventType::ButtonPressed:
            case InputEventType::ButtonHold:
            case InputEventType::ButtonReleased:
            {
                event.control = currentLayerGroup->buttonToControl(event.id);
                break;
            }
        }

        Layer* currentLayer = currentLayerGroup->layer();

        updateContext(event);

        int8_t fn = fnIndex(event.control);

        switch (event.type)
        {
            case InputEventType::ButtonPressed:
            {
                if (fn >= 0) {
                    pressedLayer[fn] = currentLayer;
                }

                if (isStep(event.control))
                {
                    // Toutes les Fn actuellement appuyées deviennent des modificateurs
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        if (ctx.fnMask & (1 << i))
                            ctx.fnState[i].usedAsModifier = true;
                    }

                    currentLayer->onStepPressed(event);
                }

                break;
            }

            case InputEventType::ButtonHold:
            {
                if (fn >= 0)
                {
                    if (!ctx.fnState[fn].consumed) {
                        pressedLayer[fn]->onButtonHold(event);

                        ctx.fnState[fn].holdTriggered= true;
                    }
                }

                break;
            }

            case InputEventType::ButtonReleased:
            {
                if (isStep(event.control))
                {
                    if (!ctx.stepUsedAsModifier){
                        currentLayer->onStepReleased(event);
                    }

                    ctx.stepUsedAsModifier = false;
                }

                if (fn >= 0)
                {
                    if (!ctx.fnState[fn].usedAsModifier &&
                        !ctx.fnState[fn].holdTriggered)
                    {
                        pressedLayer[fn]->onButtonTap(event);
                    }

                    ctx.fnState[fn].usedAsModifier = false;
                    ctx.fnState[fn].holdTriggered = false;
                    ctx.fnState[fn].consumed = false;
                }

                break;
            }

            case InputEventType::EncoderTurned:
            {

                // Toutes les Fn appuyées servent de modificateur
                for (uint8_t i = 0; i < 8; i++)
                {
                    if (ctx.fnMask & (1 << i))
                        ctx.fnState[i].usedAsModifier = true;
                }

                if (ctx.stepHeld) {
                    ctx.stepUsedAsModifier = true;
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    if (ctx.fnMask & (1 << i)) {
                        ctx.fnState[i].consumed = true;
                    }
                }

                currentLayer->onEncoder(event);
                break;
            }
        }

        resolveLayer();
        currentLayer = currentLayerGroup->layer();

        currentLayer->applyEncoderMapping();
        currentLayer->applyEncoderValues();

        ui.requestRedraw();
    }

    void updateContext(const InputEvent& event)
    {
        switch (event.type)
        {
            case InputEventType::ButtonPressed:

                if (isFn(event.control))
                    ctx.fnMask |= fnBit(event.control);

                if (isStep(event.control))
                {
                    ctx.stepHeld = true;
                    ctx.stepId = event.control;
                }

                break;

            case InputEventType::ButtonReleased:

                if (isFn(event.control))
                    ctx.fnMask &= ~fnBit(event.control);

                if (isStep(event.control))
                    ctx.stepHeld = false;

                break;

            default:
                break;
        }
    }
};
