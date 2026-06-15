#pragma once

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include "Constants.h"
#include "SequencerTimer.h"
#include <Bounce2.h>
#include <Keypad.h>
#include "InputEvent.h"
#include "Display/Display.h"
#include "InputMode.h"

class Input
{
    public:
        Keypad pad;

        InputEvent ringBuffer[32];
        uint8_t writeIndex = 0;
        uint8_t readIndex = 0;

        Input(SequencerTimer& _sequencerTimer):
            sequencerTimer(_sequencerTimer),
            pad(
                makeKeymap(Constants::KEY_MATRIX),
                Constants::ROWS_PINS,
                Constants::COLS_PINS,
                sizeof(Constants::ROWS_PINS) / sizeof(Constants::ROWS_PINS[0]),
                sizeof(Constants::COLS_PINS) / sizeof(Constants::COLS_PINS[0])
            )
        {
        }

        void attachDisplayTask(TaskHandle_t handle)
        {
            displayTaskHandle = handle;
        }

        void registerMode(InputModes mode, InputMode* instance)
        {
            modes[static_cast<uint8_t>(mode)] = instance;
        }

        uint8_t getButtonIndex(char key) const
        {
            return key - 'A';
        }

        uint8_t getRow(char key) const
        {
            return getButtonIndex(key) / Constants::COLS;
        }

        uint8_t getRow(uint8_t btn) const
        {
            return btn / Constants::COLS;
        }

        uint8_t getColumn(char key) const
        {
            return getButtonIndex(key) % Constants::COLS;
        }

        uint8_t getColumn(uint8_t btn) const
        {
            return btn % Constants::COLS;
        }

        int8_t getStep(char key) const
        {
            uint8_t buttonIndex = getButtonIndex(key);

            if (buttonIndex < 4) {
                return -1;
            }

            return buttonIndex - 4;
        }

        int8_t getStep(uint8_t buttonIndex) const
        {
            if (buttonIndex < 4) {
                return -1;
            }

            return buttonIndex - 4;
        }

        void setMode(InputModes mode)
        {
            portENTER_CRITICAL(&mux);
            // Reinit ring buffer
            readIndex = 0;
            writeIndex = 0;

            if (currentMode) {
                currentMode->onExit();
            }

            currentMode = modes[static_cast<uint8_t>(mode)];

            if (currentMode) {
                currentMode->onEnter();
            }
            portEXIT_CRITICAL(&mux);
        }

        // void setDisplayMode(DisplayModes mode)
        // {
        //     display.setMode(mode);
        // }

        void setEncoderBoundaries(ControlId encoder, long minEncoderValue, long maxEncoderValue, bool circleValues )
        {
            rotaryEncoders[controlToEncoder(encoder)].setBoundaries(minEncoderValue, maxEncoderValue, circleValues);
        }

        void setEncoderValue(ControlId encoder, long newValue )
        {
            rotaryEncoders[controlToEncoder(encoder)].setEncoderValue(newValue);
        }

        void begin()
        {
            instance = this;

            for (uint8_t i = 0; i < Constants::NUMBER_OF_ROTARY_ENCODERS; i++) {
                rotaryEncoderButtons[i].attach(Constants::ROTARY_ENCODERS_PIN[i][2], INPUT_PULLUP);
                rotaryEncoderButtons[i].interval(5);
                rotaryEncoderButtons[i].setPressedState(LOW);
            }

            for (uint8_t i = 0;i < Constants::NUMBER_OF_ROTARY_ENCODERS; i++) {
                rotaryEncoders[i] = AiEsp32RotaryEncoder(Constants::ROTARY_ENCODERS_PIN[i][0], Constants::ROTARY_ENCODERS_PIN[i][1], Constants::ROTARY_ENCODERS_PIN[i][2], -1, 4);

                rotaryEncoders[i].begin();
                rotaryEncoders[i].setup(readEncoderISR);
                rotaryEncoders[i].setAcceleration(200);
            }

            setMode(InputModes::Main);


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

        static void IRAM_ATTR readEncoderISR()
        {
            if (instance != nullptr) {
                for (uint8_t i = 0;i < Constants::NUMBER_OF_ROTARY_ENCODERS; i++) {
                    instance->rotaryEncoders[i].readEncoder_ISR();
                }
            }
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

                    if (input->currentMode) {
                        input->currentMode->handleEvent(e);

                        instance->updateDisplay();
                    }
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
                return;
            }
            ringBuffer[writeIndex] = e;

            writeIndex = (writeIndex + 1) % 32;

            portEXIT_CRITICAL(&mux);
            xTaskNotifyGive(xHandleInputManager);
        }

        ControlId buttonToControl(uint8_t button)
        {
            switch (button)
            {
                case 0: return ControlId::Fn0;
                case 1: return ControlId::Fn1;
                case 2: return ControlId::Fn2;
                case 3: return ControlId::Fn3;
                case 4: return ControlId::Fn4;
                case 5: return ControlId::Fn5;
                case 6: return ControlId::Fn6;
                case 7: return ControlId::Fn7;
                default: return ControlId::None;
            }
        }

        ControlId encoderToControl(uint8_t encoder)
        {
            switch (encoder)
            {
                case 0: return ControlId::Encoder0;
                case 1: return ControlId::Encoder1;
                default: return ControlId::None;
            }
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

        static void inputTask(void* pvParameters)
        {
            Input* input = static_cast<Input*>(pvParameters);
            for (;;) {
                for (uint8_t i = 0;i<Constants::NUMBER_OF_ROTARY_ENCODERS;i++) {
                    input->rotaryEncoderButtons[i].update();
                    if (input->rotaryEncoders[i].encoderChanged()) {
                        InputEvent e;
                        e.id = i;
                        e.control = input->encoderToControl(i);
                        e.type = InputEventType::EncoderTurned;
                        e.value = input->rotaryEncoders[i].readEncoder();
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
                            e.control = input->buttonToControl(input->getButtonIndex(key));

                            if (ks == KeyState::PRESSED) {
                                e.type = InputEventType::ButtonPressed;
                                Serial.println("Event marix");
                                Serial.println(i);
                                Serial.println(key);
                                instance->pushEvent(e);
                            }
                            if (ks == KeyState::HOLD) {
                                InputEvent e;
                                e.type = InputEventType::ButtonHold;
                                instance->pushEvent(e);
                            }
                            if (ks == KeyState::RELEASED) {
                                InputEvent e;
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
        // Display& display;
        static Input* instance;
        AiEsp32RotaryEncoder rotaryEncoders[Constants::NUMBER_OF_ROTARY_ENCODERS];
        Bounce2::Button rotaryEncoderButtons[Constants::NUMBER_OF_ROTARY_ENCODERS];

        InputMode* currentMode = nullptr;

        InputMode* modes[4] = {};

        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;     
        
        TaskHandle_t displayTaskHandle = nullptr;

        void updateDisplay()
        {
            if (displayTaskHandle != nullptr) {
                xTaskNotifyGive(displayTaskHandle);
            }
        }
};
Input* Input::instance = nullptr;
