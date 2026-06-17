#pragma once

#include "InputEvent.h"
class Input;


enum class InputModes
{
    None,
    Main,
    Step
};

using FnMask = uint8_t;

typedef struct 
{
    FnMask pressedMask = 0;

    bool holdTriggered = false;
    bool modifierUsed = false;

    FnMask pressCombo = 0;
    FnMask holdCombo = 0;
} FnState;

typedef struct 
{
    bool pressed = false;
    bool holdTriggered = false;
    bool stepUsed = false;

    ControlId stepId;

    FnMask fnCombo = 0;
} StepState;

constexpr bool isFn(ControlId c)
{
    return c >= ControlId::Fn0 &&
           c <= ControlId::Fn7;
}

constexpr bool isStep(ControlId c)
{
    return c >= ControlId::Step0 &&
           c <= ControlId::Step3;
}

constexpr bool isEncoder(ControlId c)
{
    return c == ControlId::Encoder0 ||
           c == ControlId::Encoder1;
}

constexpr uint8_t fnIndex(ControlId c)
{
    return static_cast<uint8_t>(c)
         - static_cast<uint8_t>(ControlId::Fn0);
}

constexpr FnMask fnBit(ControlId c)
{
    return static_cast<FnMask>(1u << fnIndex(c));
}

constexpr FnMask FN0 = 1u << 0;
constexpr FnMask FN1 = 1u << 1;
constexpr FnMask FN2 = 1u << 2;
constexpr FnMask FN3 = 1u << 3;
constexpr FnMask FN4 = 1u << 4;
constexpr FnMask FN5 = 1u << 5;
constexpr FnMask FN6 = 1u << 6;
constexpr FnMask FN7 = 1u << 7;

class InputMode
{
    public:
        explicit InputMode(Input& _input)
            : input(_input)
        {
        }

        virtual void onEnter() {}

        virtual void onExit() {}

        virtual void handleEvent(const InputEvent& event)
        {
            switch(event.type) {
                case InputEventType::ButtonPressed:
                {
                    // We save pressed Fn key in bitmask
                    // If step key, if bitmask Fn set we take it
                    if (isFn(event.control)) {
                        fnState.pressedMask |= fnBit(event.control);
                        fnState.pressCombo |= fnBit(event.control);
                    } else if (isStep(event.control)) {
                        stepState.pressed = true;
                        stepState.stepId = event.control;

                        // Snapshot de la combinaison
                        stepState.fnCombo = fnState.pressedMask;

                        if (stepState.fnCombo != 0) {
                            fnState.modifierUsed = true;
                        }
                    }

                    break;
                }

                case InputEventType::ButtonHold:
                {
                    // Just check if Fn key is hold
                    // We get bitmask to know if many pressed
                    if (isFn(event.control)) {
                        fnState.holdTriggered = true;

                        // Snapshot de la combinaison
                        fnState.holdCombo = fnState.pressedMask;

                        if (!fnState.modifierUsed) {
                            fnHoldAction(event, fnState.holdCombo);
                        }
                    } else if(isStep(event.control)) {
                        stepState.holdTriggered = true;

                        stepHoldAction(event, stepState.fnCombo);
                    }

                    break;
                }

                case InputEventType::ButtonReleased:
                {
                    // Serial.println("RELEASE");
                    // Serial.println(isFn(event.control));
                    // Serial.println(static_cast<uint8_t>(event.control));
                    // If a step released and was the pressed step
                    // We have fnCombo so we can do something different 
                    if (isStep(event.control) &&
                        stepState.pressed &&
                        event.control == stepState.stepId
                    )
                    {
                        if (!stepState.stepUsed) {
                            if (stepState.holdTriggered) {
                                stepHoldReleaseAction(event, stepState.fnCombo);
                            } else {
                                stepReleaseAction(event, stepState.fnCombo);
                            }
                        }

                        stepState.pressed = false;
                        stepState.holdTriggered = false;
                        stepState.fnCombo = 0;
                        stepState.stepUsed = false;
                    } else if (isFn(event.control)) {
                        fnState.pressedMask &= ~fnBit(event.control);

                        // Dernier Fn relâché
                        // If no bit mask, fn key hold and we didnt press step key we execute hold action of fn key
                        if (fnState.pressedMask == 0) {
                            // Serial.println("holdtrigger");
                            // Serial.println(fnState.holdTriggered);
                            // Serial.println("modifierUsed");
                            // Serial.println(fnState.modifierUsed);

                            if (fnState.holdTriggered && !fnState.modifierUsed) {
                                fnHoldReleaseAction(
                                    event,
                                    fnState.holdCombo);
                            } else if (!fnState.modifierUsed) {
                                // Serial.println("RELAFN");
                                // Serial.println(static_cast<uint8_t>(fnState.pressCombo));
                                fnReleaseAction(
                                    event,
                                    fnState.pressCombo);
                            }

                            fnState.holdTriggered = false;
                            fnState.modifierUsed = false;
                            fnState.pressCombo = 0;
                            fnState.holdCombo = 0;
                        }
                    }

                    break;
                }

                case InputEventType::EncoderTurned:
                {
                    // No encoder nothing to do
                    if (!isEncoder(event.control)) {
                        break;
                    }

                    // We have bitmask because we pressed fn key before, we can do something based on or not
                    encoderAction(event, fnState.pressedMask);

                    // We use rotary, we mustnot execute fn hold action
                    if (fnState.pressedMask != 0) {
                        fnState.modifierUsed = true;
                    }

                    if (stepState.pressed) {
                        stepState.stepUsed = true;
                    }

                    break;
                }


                default:
                    break;
            }
        }

        void attachDisplayTask(TaskHandle_t handle)
        {
            displayTaskHandle = handle;
        }

        void notifyDisplay()
        {
            if (displayTaskHandle != nullptr) {
                xTaskNotifyGive(displayTaskHandle);
            }
        }
    
    protected:
        virtual void stepReleaseAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Release][step]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

        virtual void fnReleaseAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Realse][fn]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

        virtual void fnHoldAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Hold][fn]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

        virtual void encoderAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Encoder]]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
            Serial.println("Value");
            Serial.println(event.value);
        }

        virtual void stepHoldAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Hold][step]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

        virtual void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Release][hold][fn]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

        virtual void stepHoldReleaseAction(const InputEvent& event, FnMask fnMask) {
            Serial.println("[INPUT][Release][hold][step]");
            Serial.println(static_cast<uint8_t>(event.control));
            Serial.println("Mask");
            Serial.println(fnMask);
            Serial.println("Step pressed");
            Serial.println(static_cast<uint8_t>(stepState.stepId));
        }

    protected:
        Input& input;
        TaskHandle_t displayTaskHandle = nullptr;

        FnState fnState;
        StepState stepState;
};
