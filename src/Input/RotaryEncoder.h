#pragma once

#include "AiEsp32RotaryEncoder.h"
#include <Constants.h>
#include "InputEvent.h"

class RotaryEncoder
{
    public:
        AiEsp32RotaryEncoder* rotaryEncoders[Constants::NUMBER_OF_ROTARY_ENCODERS];
        static RotaryEncoder* instance;

        long lastPosition[Constants::NUMBER_OF_ROTARY_ENCODERS];

        // Bornes stockees ici (en plus d'etre passees a la lib) car il en faut
        // la taille de plage pour "deplier" un wrap dans getDirection().
        long minValue[Constants::NUMBER_OF_ROTARY_ENCODERS] = {0};
        long maxValue[Constants::NUMBER_OF_ROTARY_ENCODERS] = {0};

        void begin()
        {
            instance = this;

            for (uint8_t i = 0;i < Constants::NUMBER_OF_ROTARY_ENCODERS; i++) {
                rotaryEncoders[i] = new AiEsp32RotaryEncoder(
                    Constants::ROTARY_ENCODERS_PIN[i][0],
                    Constants::ROTARY_ENCODERS_PIN[i][1],
                    -1,
                    -1,
                    4
                );

                rotaryEncoders[i]->begin();
                // rotaryEncoders[i].setAcceleration(0);
                rotaryEncoders[i]->disableAcceleration();

            }

            rotaryEncoders[0]->setup(readEncoder0ISR);
            rotaryEncoders[1]->setup(readEncoder1ISR);
        }

        static void IRAM_ATTR readEncoder0ISR()
        {
            if (instance != nullptr) {
                instance->rotaryEncoders[0]->readEncoder_ISR();
            }
        }

        static void IRAM_ATTR readEncoder1ISR()
        {
            if (instance != nullptr) {
                instance->rotaryEncoders[1]->readEncoder_ISR();
            }
        }

        void setEncoderBoundaries(ControlId encoder, long minEncoderValue, long maxEncoderValue, bool circleValues )
        {
            int8_t idx = controlToEncoder(encoder);

            minValue[idx] = minEncoderValue;
            maxValue[idx] = maxEncoderValue;

            rotaryEncoders[idx].setBoundaries(minEncoderValue, maxEncoderValue, circleValues);
        }

        void setEncoderValue(ControlId encoder, long newValue )
        {
            rotaryEncoders[controlToEncoder(encoder)].setEncoderValue(newValue);
        }

        void setLastPosition(ControlId encoder, long _lastPosition )
        {
            lastPosition[controlToEncoder(encoder)] = _lastPosition;
        }

        void syncEncoder(ControlId encoder, long value)
        {
            setEncoderValue(encoder, value);
            setLastPosition(encoder, value);
        }

        // Retourne -1, 0 ou 1 selon la direction, en tenant compte du wrap
        // circulaire (ex: 4 -> 0 = +1, 0 -> 4 = -1), puis met a jour
        // lastPosition pour le prochain appel.
        long getDelta(ControlId encoder, long newValue)
        {
            int8_t idx = controlToEncoder(encoder);
            long last = lastPosition[idx];

            if (newValue == last) {
                return 0;
            }

            long rangeSize = maxValue[idx] - minValue[idx] + 1;
            long diff = newValue - last;

            // "Deplie" la difference dans (-rangeSize/2, rangeSize/2] pour
            // que 4->0 (diff brut -4) redevienne +1, et 0->4 (diff brut +4)
            // redevienne -1, sur une plage 0..4 (rangeSize = 5).
            if (rangeSize > 0) {
                if (diff > rangeSize / 2) {
                    diff -= rangeSize;
                } else if (diff < -rangeSize / 2) {
                    diff += rangeSize;
                }
            }

            lastPosition[idx] = newValue;

            return diff;
        }

        long getDirection(ControlId encoder, long newValue)
        {
            long delta = getDelta(encoder, newValue);
            if (delta > 0) return 1;
            if (delta < 0) return -1;
            return 0;
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

        ControlId encoderToControl(int8_t encoder)
        {
            switch (encoder)
            {
                case 0: return ControlId::Encoder0;
                case 1: return ControlId::Encoder1;
                default: return ControlId::None;
            }
        }

};
RotaryEncoder* RotaryEncoder::instance = nullptr;
