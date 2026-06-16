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
