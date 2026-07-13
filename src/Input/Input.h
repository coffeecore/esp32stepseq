#pragma once

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include "Constants.h"
#include "SequencerTimer.h"
#include <Keypad.h>
#include "InputEvent.h"
#include "RotaryEncoder.h"
#include "Input/InputEngine.h"

class Input
{
    public:
        Keypad pad;
        InputEvent ringBuffer[32];
        uint8_t writeIndex = 0;
        uint8_t readIndex = 0;
        RotaryEncoder& rotaryEncoders;

        Input(SequencerTimer& _sequencerTimer, InputEngine& _inputEngine, RotaryEncoder& _rotaryEncoders):
            sequencerTimer(_sequencerTimer),
            pad(
                makeKeymap(Constants::KEY_MATRIX),
                Constants::ROWS_PINS,
                Constants::COLS_PINS,
                sizeof(Constants::ROWS_PINS) / sizeof(Constants::ROWS_PINS[0]),
                sizeof(Constants::COLS_PINS) / sizeof(Constants::COLS_PINS[0])
            ),
            inputEngine(_inputEngine),
            rotaryEncoders(_rotaryEncoders)
        {
        }

        uint8_t getButtonIndex(char key) const
        {
            return key - 'A';
        }

        void begin()
        {
            instance = this;

            xTaskCreatePinnedToCore(
                inputTask,
                "InputTask",
                8192,
                this,
                1,
                &xHandleInput,
                0
            );

            xTaskCreatePinnedToCore(
                inputManagerTask,
                "InputManagerTask",
                8192,
                this,
                1,
                &xHandleInputManager,
                0
            );
        }

        static void inputManagerTask(void* pvParameters)
        {
            Input* input = static_cast<Input*>(pvParameters);
            for (;;) {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

                while(true) {

                    portENTER_CRITICAL(&input->mux);
                    if (input->readIndex == input->writeIndex) {
                        portEXIT_CRITICAL(&input->mux);
                        break;
                    }
                    InputEvent e = input->ringBuffer[input->readIndex];

                    input->readIndex = (input->readIndex + 1) % 32;

                    portEXIT_CRITICAL(&input->mux);

                    input->inputEngine.handleEvent(e);
                }

                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        void pushEvent(const InputEvent e)
        {
            portENTER_CRITICAL(&mux);
            uint8_t next = (writeIndex + 1) % 32;

            if (next == readIndex) {
                Serial.println("BUFFER OVERFLOW");
                portEXIT_CRITICAL(&mux);
                return;
            }
            ringBuffer[writeIndex] = e;

            writeIndex = (writeIndex + 1) % 32;

            portEXIT_CRITICAL(&mux);
            xTaskNotifyGive(xHandleInputManager);
        }

        static void inputTask(void* pvParameters)
        {
            Input* input = static_cast<Input*>(pvParameters);
            for (;;) {
                for (uint8_t i = 0;i<Constants::NUMBER_OF_ROTARY_ENCODERS;i++) {
                    if (0 != input->rotaryEncoders.rotaryEncoders[i].encoderChanged()) {
                        InputEvent e;
                        e.id = i;
                        e.type = InputEventType::EncoderTurned;
                        Serial.println("JJDDJJDJDJDJD");
                        Serial.println(input->rotaryEncoders.rotaryEncoders[i].readEncoder());
                        Serial.println(input->rotaryEncoders.minValue[i]);
                        Serial.println(input->rotaryEncoders.maxValue[i]);
                        e.value = input->rotaryEncoders.rotaryEncoders[i].readEncoder();
                        e.delta = input->rotaryEncoders.getDelta(input->rotaryEncoders.encoderToControl(i), e.value);
                        instance->pushEvent(e);
                    }
                }

                if (input->pad.getKeys()) {
                    for (byte i = 0; i < LIST_MAX; i++) {
                        if ( input->pad.key[i].stateChanged ) {
                            char key = input->pad.key[i].kchar;

                            KeyState ks = input->pad.key[i].kstate;

                            InputEvent e;
                            e.id = input->getButtonIndex(key);

                            if (ks == KeyState::PRESSED) {
                                e.type = InputEventType::ButtonPressed;
                                instance->pushEvent(e);
                            }
                            if (ks == KeyState::HOLD) {
                                e.type = InputEventType::ButtonHold;
                                instance->pushEvent(e);
                            }
                            if (ks == KeyState::RELEASED) {
                                e.type = InputEventType::ButtonReleased;
                                instance->pushEvent(e);
                            }
                        }
                    }
                }

                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    
    private:
        TaskHandle_t xHandleInput = nullptr;
        TaskHandle_t xHandleInputManager = nullptr;
        SequencerTimer& sequencerTimer;
        static Input* instance;
        InputEngine& inputEngine;
        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;     
};
Input* Input::instance = nullptr;
