#pragma once

#include "SequencerTimer.h"
#include "Input/Input.h"
#include "Input/InputMode.h"
#include "Display/Display.h"

class StepInputMode : public InputMode
{
public:
    StepInputMode(
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
        Serial.println("=== STEP MODE ===");

        input.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
        input.setEncoderValue(ControlId::Encoder1, sequencerTimer.volume);
    }

    void onExit() override
    {
        Serial.println(">>> STEP MODE <<<");
    }

    void handleEvent(const InputEvent& event) override
    {
        switch(event.type) {

            case InputEventType::ButtonPressed:
                Serial.print("[Input][STEP MODE] Button pressed: ");
                Serial.println(event.id);

                break;

            case InputEventType::ButtonHold:
                Serial.print("[Input][STEP MODE] Button hold: ");
                Serial.println(event.id);
                // buttonHold[event.id] = true;

                break;

            case InputEventType::ButtonReleased:
                Serial.print("[Input][STEP MODE] Button relase: ");
                Serial.println(event.id);

                // if (buttonHold[event.id]) {
                //     Serial.print("Button holding: ");
                //     Serial.println(event.id);
                //     Serial.println("Step:");
                //     Serial.println(input.getStep(event.id));
                //     buttonHold[event.id] = false;
                // } else {
                //     sequencerTimer.toggleStep(input.getStep(event.id));
                // }

                break;

            case InputEventType::EncoderTurned:
                Serial.print("[Input][STEP MODE] Encoder value: ");
                Serial.println(event.id);
                Serial.println(event.value);

                // if (encoderPressed[event.id]) {
                //     sequencerTimer.setBpm(event.value);
                // } else {
                //     sequencerTimer.setVolume(event.value);
                // }

                break;
            
            // case InputEventType::EncoderPressed:
            //     Serial.print("[Input][STEP MODE] Encoder pressed: ");
            //     Serial.println(event.id);
            //     // encoderPressed[event.id] = true;
            //     // input.setEncoderBoundaries(event.id, 0, 999, false);
            //     // input.setEncoderValue(event.id, sequencerTimer.getBpm());

            //     break;
            
            // case InputEventType::EncoderReleased:
            //     Serial.print("[Input][STEP MODE] Encoder release: ");
            //     Serial.println(event.id);
            //     // encoderPressed[event.id] = false;
            //     // input.setEncoderBoundaries(event.id, 0, 255, false);
            //     // input.setEncoderValue(event.id, sequencerTimer.getVolume());


            //     break;

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
