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

    void handleEvent(const InputEvent& event) override
    {
        switch(event.type) {
            case InputEventType::ButtonPressed:
            {
                // We save pressed Fn key in bitmask
                // If step key, if bitmask Fn set we take it
                if (isFn(event.control)) {
                    fnState.pressedMask |= fnBit(event.control);
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
                }

                break;
            }

            case InputEventType::ButtonReleased:
            {
                // If a step released and was the pressed step
                // We have fnCombo so we can do something different 
                if (isStep(event.control) &&
                    stepState.pressed &&
                    event.control == stepState.stepId) {
                    executeStepAction(event, stepState.fnCombo);

                    stepState.pressed = false;
                    stepState.fnCombo = 0;
                // if fn key released, remove from bitmask
                } else if (isFn(event.control)) {
                    fnState.pressedMask &= ~fnBit(event.control);

                    // Dernier Fn relâché
                    // If no bit mask, fn key hold and we didnt press step key we execute hold action offn key
                    if (fnState.pressedMask == 0) {
                        if (fnState.holdTriggered &&
                            !fnState.modifierUsed) {
                            // executeHoldAction(
                            //     fnState.holdCombo);
                        }

                        fnState.holdTriggered = false;
                        fnState.modifierUsed = false;
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
                // executeEncoderAction(
                //     event.control,
                //     delta,
                //     fnState.pressedMask);

                // We use rotary, we mustnot execute fn hold action
                if (fnState.pressedMask != 0) {
                    fnState.modifierUsed = true;
                }

                break;
            }


            default:
                break;
        }
    }

    void executeStepAction(InputEvent event, FnMask fnMask)
    {
        switch(fnMask)
        {
            case 0:
                uint8_t row = static_cast<uint8_t>(event.control) / 4;
                uint8_t col = static_cast<uint8_t>(event.control) % 4;
                sequencerTimer.toggleStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, row, col);

                break;

            case FN4:
                selectPattern(step);
                break;

            case FN5:
                selectTrack(step);
                break;

            case (FN4 | FN5):
                selectBank(step);
                break;

            default:
                break;
        }
    }

    private:
        SequencerTimer& sequencerTimer;
        Display& display;
        InputModes pendingMode = InputModes::None;
};
