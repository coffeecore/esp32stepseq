#pragma once

#include "AiEsp32RotaryEncoder.h"
#include <Constants.h>
#include "InputEvent.h"

class RotaryEncoder
{
    public:
        AiEsp32RotaryEncoder rotaryEncoders[Constants::NUMBER_OF_ROTARY_ENCODERS];
        static RotaryEncoder* instance;

        void begin()
        {
            instance = this;

            for (uint8_t i = 0;i < Constants::NUMBER_OF_ROTARY_ENCODERS; i++) {
                rotaryEncoders[i] = AiEsp32RotaryEncoder(Constants::ROTARY_ENCODERS_PIN[i][0], Constants::ROTARY_ENCODERS_PIN[i][1], Constants::ROTARY_ENCODERS_PIN[i][2], -1, 4);

                rotaryEncoders[i].begin();
                rotaryEncoders[i].setAcceleration(0);
            }

            rotaryEncoders[0].setup(readEncoder0ISR);
            rotaryEncoders[1].setup(readEncoder1ISR);
        }

        static void IRAM_ATTR readEncoder0ISR()
        {
            if (instance != nullptr) {
                instance->rotaryEncoders[0].readEncoder_ISR();
            }
        }

        static void IRAM_ATTR readEncoder1ISR()
        {
            if (instance != nullptr) {
                instance->rotaryEncoders[1].readEncoder_ISR();
            }
        }

        void setEncoderBoundaries(ControlId encoder, long minEncoderValue, long maxEncoderValue, bool circleValues )
        {
            rotaryEncoders[controlToEncoder(encoder)].setBoundaries(minEncoderValue, maxEncoderValue, circleValues);
        }

        void setEncoderValue(ControlId encoder, long newValue )
        {
            rotaryEncoders[controlToEncoder(encoder)].setEncoderValue(newValue);
        }

        int8_t controlToEncoder(ControlId encoder)
        {
            switch (encoder)
            {
                case ControlId::Encoder0: return 0;
                case ControlId::Encoder1: return 1;
                default: return -1;
            }
        }

};
RotaryEncoder* RotaryEncoder::instance = nullptr;
