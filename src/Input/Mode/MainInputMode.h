#pragma once

#include "SequencerTimer.h"
#include "Input/Input.h"
#include "Input/InputMode.h"
#include "Display/Display.h"


class MainInputMode : public InputMode
{
    public:
        MainInputMode(
            Input& _input,
            SequencerTimer& _sequencerTimer,
            Display& _display
        )
            : InputMode(_input),
            sequencerTimer(_sequencerTimer),
            display(_display)
        {
        }

        void onEnter() override
        {
            Serial.println("=== MAIN MODE ===");

            input.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
            input.setEncoderValue(ControlId::Encoder1, sequencerTimer.volume);
        }

        void onExit() override
        {
            Serial.println(">>> MAIN MODE <<<");
        }

    protected:
        void stepReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch(fnMask)
            {
                case 0:
                {
                    uint8_t row = static_cast<uint8_t>(event.control) / 4;
                    uint8_t col = static_cast<uint8_t>(event.control) % 4;
                    sequencerTimer.toggleStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, row, col);

                    break;
                }

                case FN4:
                    // selectPattern(step);
                    break;

                case FN5:
                    // selectTrack(step);
                    break;

                case (FN4 | FN5):
                    // selectBank(step);
                    break;

                default:
                    break;
            }
        }

        void fnReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
        }

        void fnHoldAction(const InputEvent& event, FnMask fnMask) override
        {
        }

        void encoderAction(const InputEvent& event, FnMask fnMask) override
        {
            if (stepState.pressed)
            {
                return;
            }
            // global fn pas step
        }

        void stepHoldAction(const InputEvent& event, FnMask fnMask) override
        {
        }

        void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
        }

        void stepHoldReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
        }

    private:
        SequencerTimer& sequencerTimer;
        Display& display;
        InputModes pendingMode = InputModes::None;
};



// Oui, et je pense qu'il faut même le faire maintenant, sinon tu vas te retrouver avec une logique spéciale pour les Fn et une autre pour les Steps.

// Je te conseillerais de distinguer les états de hold des Fn et des Steps.

// Par exemple :

// ```cpp
// struct StepState
// {
//     bool pressed = false;
//     bool holdTriggered = false;

//     ControlId stepId;
//     FnMask fnCombo = 0;
// };
// ```

// Puis dans le handler :

// ```cpp
// case InputEventType::ButtonHold:
// {
//     //----------------------------------
//     // FN HOLD
//     //----------------------------------

//     if (isFn(event.control))
//     {
//         fnState.holdTriggered = true;
//         fnState.holdCombo = fnState.pressedMask;

//         if (!fnState.modifierUsed)
//         {
//             executeFnHoldAction(
//                 event.control,
//                 fnState.holdCombo);
//         }
//     }

//     //----------------------------------
//     // STEP HOLD
//     //----------------------------------

//     else if (isStep(event.control))
//     {
//         stepState.holdTriggered = true;

//         executeStepHoldAction(
//             event.control,
//             stepState.fnCombo);
//     }

//     break;
// }
// ```

// Ensuite au relâchement du Step :

// ```cpp
// if (isStep(event.control) &&
//     stepState.pressed &&
//     event.control == stepState.stepId)
// {
//     if (stepState.holdTriggered)
//     {
//         executeStepHoldReleasedAction(
//             event.control,
//             stepState.fnCombo);
//     }
//     else
//     {
//         executeStepAction(
//             event,
//             stepState.fnCombo);
//     }

//     stepState.pressed = false;
//     stepState.holdTriggered = false;
//     stepState.fnCombo = 0;
// }
// ```

// Ce qui donne :

// ### Appui court

// ```text
// Step2 down
// Step2 up
// ```

// →

// ```cpp
// executeStepAction(Step2, 0);
// ```

// ### Hold simple

// ```text
// Step2 down
// hold
// Step2 up
// ```

// →

// ```cpp
// executeStepHoldAction(Step2, 0);
// executeStepHoldReleasedAction(Step2, 0);
// ```

// ### Hold avec modifier

// ```text
// Fn4 down
// Step2 down
// hold
// Step2 up
// Fn4 up
// ```

// →

// ```cpp
// executeStepHoldAction(Step2, FN4);
// executeStepHoldReleasedAction(Step2, FN4);
// ```

// Le point important est que tu utilises déjà :

// ```cpp
// stepState.fnCombo = fnState.pressedMask;
// ```

// au moment du `ButtonPressed` du Step. Donc ton hold de Step bénéficie automatiquement de la combinaison Fn active au moment où le Step a été pressé, même si les Fn changent ensuite.

// Ça te permet de faire des choses du genre :

// ```cpp
// executeStepHoldAction(step, FN4);        // édition pattern
// executeStepHoldAction(step, FN5);        // édition track
// executeStepHoldAction(step, FN4 | FN5);  // édition bank
// executeStepHoldAction(step, 0);          // mute, preview, etc.
// ```

// avec exactement la même mécanique que tes `executeStepAction()`.
