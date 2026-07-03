#pragma once

#include "Input/Layer/Layer.h"
#include "Layer/GlobalLayer.h"
#include "Layer/StepEditLayer.h"
#include "Layer/ModalLayer.h"
#include "Layer/NavigationLayer.h"
#include "Layer/QuarterNoteLengthLayer.h"
#include "Layer/StepInstrumentLayer.h"
#include "Layer/StepLengthLayer.h"
#include "Layer/GlobalInstrumentLayer.h"
#include "Input/InputMode.h"

class InputEngine
{
    public:
        InputContext ctx;

        GlobalLayer global;
        StepEditLayer stepEdit;
        StepLengthLayer stepLength;
        StepInstrumentLayer stepInstrument;
        QuarterNoteLengthLayer quarterNoteLength;
        NavigationLayer navigation;
        ModalLayer modal;
        GlobalInstrumentLayer globalInstrument;

        Layer* currentLayer = &global;

        SequencerTimer& sequencerTimer;
        Display& display;
        RotaryEncoder& rotaryEncoders;

        explicit InputEngine(SequencerTimer& _sequencerTimer, Display& _display, RotaryEncoder& _rotaryEncoders)
            :
            sequencerTimer(_sequencerTimer),
            display(_display),
            rotaryEncoders(_rotaryEncoders),
            global(ctx, _rotaryEncoders, _sequencerTimer, _display),
            stepEdit(ctx, _rotaryEncoders, _sequencerTimer, _display),
            stepLength(ctx, _rotaryEncoders, _sequencerTimer, _display),
            stepInstrument(ctx, _rotaryEncoders, _sequencerTimer, _display),
            quarterNoteLength(ctx, _rotaryEncoders, _sequencerTimer, _display),
            navigation(ctx, _rotaryEncoders, _sequencerTimer, _display),
            modal(ctx, _rotaryEncoders, _sequencerTimer, _display),
            globalInstrument(ctx, _rotaryEncoders, _sequencerTimer, _display)
        {
        }

        void begin()
        {
            resolveLayer();
            currentLayer->applyEncoderMapping();
            currentLayer->applyEncoderValues();
        }

        void resolveLayer()
        {
            if (ctx.modal != ModalState::None)
            {
                currentLayer = &modal;
                return;
            }

            if (ctx.stepHeld)
            {
                if (ctx.fnMask & FN1)
                {
                    currentLayer = &quarterNoteLength;

                    return;
                }
                if (ctx.fnMask & FN2)
                {
                    currentLayer = &stepLength;

                    return;
                }

                if (ctx.fnMask & FN3)
                {
                    currentLayer = &stepInstrument;

                    return;
                }

                currentLayer = &stepEdit;

                return;
            }

            if (ctx.fnMask & FN1)
            {
                currentLayer = &navigation;

                return;
            }

            if (ctx.fnMask & FN3)
            {
                currentLayer = &globalInstrument;

                return;
            }

            currentLayer = &global;
        }

    void handleEvent(const InputEvent& event)
    {
        Serial.println("InputEngine : handleEvent");
        updateContext(event);

        int8_t fn = fnIndex(event.control);

        switch (event.type)
        {
            case InputEventType::ButtonPressed:

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

            case InputEventType::ButtonHold:
                if (fn >= 0)
                {
                    currentLayer->onButtonHold(event);

                    ctx.fnState[fn].holdTriggered= true;
                }

                break;

            case InputEventType::ButtonReleased:

                if (isStep(event.control))
                {
                    if (!ctx.stepUsedAsModifier)
                        currentLayer->onStepReleased(event);

                    ctx.stepUsedAsModifier = false;
                }

                if (fn >= 0)
                {
                    if (!ctx.fnState[fn].usedAsModifier &&
                        !ctx.fnState[fn].holdTriggered)
                    {
                        currentLayer->onButtonTap(event);
                    }

                    ctx.fnState[fn].usedAsModifier = false;
                    ctx.fnState[fn].holdTriggered = false;
                }

                break;

            case InputEventType::EncoderTurned:

                // Toutes les Fn appuyées servent de modificateur
                for (uint8_t i = 0; i < 8; i++)
                {
                    if (ctx.fnMask & (1 << i))
                        ctx.fnState[i].usedAsModifier = true;
                }

                if (ctx.stepHeld)
                    ctx.stepUsedAsModifier = true;

                currentLayer->onEncoder(event);
                break;
        }

        resolveLayer();

        currentLayer->applyEncoderMapping();
        currentLayer->applyEncoderValues();
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
