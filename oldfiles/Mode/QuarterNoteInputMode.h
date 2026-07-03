#pragma once

#include "SequencerTimer.h"
#include "Input/Input.h"
#include "Input/InputMode.h"
#include "Display/Display.h"

class QuarterNoteInputMode : public InputMode
{
public:
    QuarterNoteInputMode(
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
        Serial.println("=== QUARTER MODE ===");

        input.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
        input.setEncoderValue(ControlId::Encoder1, sequencerTimer.volume);
    }

    void onExit() override
    {
        Serial.println(">>> QUARTER MODE <<<");
    }

    void fnReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
        //     // Serial.println("HOLDMask");
        //     // Serial.println(fnMask);
            switch(fnMask)
            {
                case FN4:
                {
        //     //         Serial.println("HOLDFN3");
        //     //         Serial.println(static_cast<uint8_t>(event.control));
                    display.setMode(DisplayModes::Main);
                    input.setMode(InputModes::Main);
                    return;
                }
        //     //     case FN2|FN3:
        //     //     {
        //     //         Serial.println("HOLDFN2 3");
        //     //         Serial.println(static_cast<uint8_t>(event.control));

        //     //         return;
        //     //     }
                default:
                    break;
            }
        }

    private:
        SequencerTimer& sequencerTimer;
        Display& display;

        // bool encoderPressed[Constants::NUMBER_OF_ROTARY_ENCODERS] = {};
        // bool buttonHold[20] = {};
};
