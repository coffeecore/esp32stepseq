#pragma once

#include "InputEvent.h"
class Input;


enum class InputModes
{
    None,
    Main,
    Step,
    QuarterNote
};

using FnMask = uint8_t;

enum class ModalState
{
    None,
    Confirm
};

typedef struct 
{
    FnMask pressedMask = 0; // Fn encore enfoncés maintenant

    bool holdTriggered = false;
    bool modifierUsed = false;
    bool holdConsumed = false;

    FnMask pressCombo = 0; // tous les Fn ayant participé à la séquence jusqu'au relâchement complet
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
           c <= ControlId::Step15;
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
    protected:
        Input& input;
        TaskHandle_t displayTaskHandle = nullptr;

        FnState fnState;
        StepState stepState;
        ModalState modalState = ModalState::None;

    public:
        explicit InputMode(Input& _input)
            : input(_input)
        {
        }

        virtual void onEnter() {}

        virtual void onExit() {}

        virtual void handleEvent(const InputEvent& event)
        {
            if (modalState != ModalState::None) {
                modalAction(event);

                return;
            }

            switch(event.type) {
                case InputEventType::ButtonPressed:
                {
                    // We save pressed Fn key in bitmask
                    // If step key, if bitmask Fn set we take it
                    if (isFn(event.control)) {
                        fnState.pressedMask |= fnBit(event.control);
                        fnState.pressCombo |= fnBit(event.control);

                        fnPressedAction(event, fnState.pressedMask);

                    } else if (isStep(event.control)) {
                        stepState.pressed = true;
                        stepState.stepId = event.control;

                        // Snapshot de la combinaison
                        stepState.fnCombo = fnState.pressedMask;

                        stepPressedAction(event, stepState.fnCombo);

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
                        if (fnState.modifierUsed) {
                            break;
                        }
                        fnState.holdTriggered = true;

                        // Snapshot de la combinaison
                        fnState.holdCombo = fnState.pressedMask;
                        fnHoldAction(event, fnState.holdCombo);

                        // Le hold a effectué son action,
                        // il ne faudra rien faire au release.
                        fnState.holdConsumed = true;
                    } else if(isStep(event.control)) {
                        stepState.holdTriggered = true;

                        stepHoldAction(event, stepState.fnCombo);
                    }

                    break;
                }

                case InputEventType::ButtonReleased:
                {
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
                        stepReleasedAlways(event, stepState.fnCombo);
                        stepState.pressed = false;
                        stepState.holdTriggered = false;
                        stepState.fnCombo = 0;
                        stepState.stepUsed = false;
                    } else if (isFn(event.control)) {
                        fnState.pressedMask &= ~fnBit(event.control);

                        if (fnState.pressedMask == 0) {
                            if (!fnState.modifierUsed)
                            {
                                if (fnState.holdTriggered)
                                {
                                    if (!fnState.holdConsumed)
                                    {
                                        fnHoldReleaseAction(event, fnState.holdCombo);
                                    }
                                }
                                else
                                {
                                    fnReleaseAction(event, fnState.pressCombo);
                                }
                            }

                            fnReleasedAlways(event, fnState.pressCombo);

                            fnState.holdTriggered = false;
                            fnState.holdConsumed = false;
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
        virtual void stepReleasedAlways(const InputEvent& event, FnMask fnMask)
        {
        }
        virtual void fnReleasedAlways(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (releasedFn == ControlId::Fn0) {
            //     // Le dernier Fn relâché est Fn0
            // }

            // if (fnMask == FN0) {
            //     // La combinaison utilisée était uniquement Fn0
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // La combinaison utilisée était Fn0 + Fn2
            // }
        }

        virtual void stepReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedStep = event.control;

            // if (releasedStep == ControlId::Step3) {
            //     // Relâchement du Step 3
            // }

            // if (fnMask == 0) {
            //     // Aucun Fn utilisé
            // }

            // if (fnMask == FN1) {
            //     // Step utilisé avec Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Step utilisé avec Fn0 + Fn2
            // }
        }

        virtual void fnReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (fnMask == (FN0 | FN2)) {
            //     // La combinaison utilisée était Fn0 + Fn2
            // }

            // if (releasedFn == ControlId::Fn2) {
            //     // Le dernier Fn relâché est Fn2
            // }
        }

        virtual void fnHoldAction(const InputEvent& event, FnMask fnMask)
        {
            // auto heldFn = event.control;

            // if (fnMask == FN1) {
            //     // Hold sur Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Hold alors que Fn0 + Fn2 sont maintenus
            // }
        }

        virtual void encoderAction(const InputEvent& event, FnMask fnMask)
        {
            // auto encoder = event.control;

            // if (encoder == ControlId::Encoder0) {
            //     // Encoder 0 tourné
            // }

            // if (encoder == ControlId::Encoder1) {
            //     // Encoder 1 tourné
            // }

            // if (fnMask == 0) {
            //     // Encoder seul
            // }

            // if (fnMask == FN1) {
            //     // Encoder avec Fn1 maintenu
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Encoder avec Fn0 + Fn2 maintenus
            // }

            // if (stepState.pressed) {
            //     // un step est maintenu
            // }

            // if (stepState.stepId == ControlId::Step3) {
            //     // c'est Step3
            // }
        }

        virtual void stepHoldAction(const InputEvent& event, FnMask fnMask)
        {
            // auto heldStep = event.control;

            // if (heldStep == ControlId::Step8) {
            //     // Hold sur Step8
            // }

            // if (fnMask == FN3) {
            //     // Hold sur Step8 avec Fn3
            // }
        }

        virtual void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (fnMask == FN1) {
            //     // Fin du hold de Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Fin du hold de la combinaison Fn0 + Fn2
            // }
        }

        virtual void stepHoldReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedStep = event.control;

            // if (releasedStep == ControlId::Step8) {
            //     // Fin du hold sur Step8
            // }

            // if (fnMask == FN3) {
            //     // Fin du hold Step8 + Fn3
            // }
        }

        virtual void fnPressedAction(const InputEvent& event, FnMask fnMask)
        {
            // auto pressedFn = event.control;

            // if (fnMask == (FN0 | FN2)) {
            //     // Fn0 + Fn2 actuellement maintenus
            // }

            // if (pressedFn == ControlId::Fn2) {
            //     // c'est Fn2 qui vient d'être appuyé
            // }
        }

        virtual void stepPressedAction(const InputEvent& event, FnMask fnMask)
        {
            // auto pressedStep = event.control;

            // if (pressedStep == ControlId::Step3) {
            //     // C'est Step3 qui vient d'être pressé
            // }

            // if (fnMask == 0) {
            //     // Step pressé sans Fn
            // }

            // if (fnMask == FN1) {
            //     // Step pressé avec Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Step pressé avec Fn0 + Fn2
            // }
        }

        virtual void modalAction(const InputEvent& event)
        {
        }
};
