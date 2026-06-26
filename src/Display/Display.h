#pragma once

#include "Arduino.h"
#include "Constants.h"
#include "SequencerTimer.h"
#include <U8g2lib.h>
// #include "InputMode.h"
#include "DisplayMode.h"

struct ConfirmDialog
{
    bool active = false;
    const char* text = nullptr;
};

class Display
{
    private:
        TaskHandle_t xHandle = nullptr;
        SequencerTimer& sequencerTimer;
        DisplayMode* modes[4] = {};
        DisplayMode* currentMode = nullptr;

    public:
        U8G2& u8g2;
        uint8_t displayedTrack = 0;
        ConfirmDialog confirm;

        Display(
            SequencerTimer& _sequencerTimer,
            U8G2& u8g2
        ): 
        sequencerTimer(_sequencerTimer),
        u8g2(u8g2)
        {
        }

        void drawConfirm()
        {
            u8g2.drawFrame(10,18,108,28);

            u8g2.setFont(u8g2_font_5x8_tf);

            u8g2.drawStr(20,32,confirm.text);

            u8g2.drawStr(20,45,"YES");
            u8g2.drawStr(80,45,"NO");
        }

        void showConfirm(const char* text)
        {
            confirm.active = true;
            confirm.text = text;
            updateDisplay();
        }

        void hideConfirm()
        {
            confirm.active = false;
            updateDisplay();
        }

        TaskHandle_t getTaskHandle() const
        {
            return xHandle;
        }

        void setMode(DisplayModes mode)
        {
            currentMode = modes[static_cast<uint8_t>(mode)];

            updateDisplay();
        }

        void updateDisplay()
        {
            if (xHandle != nullptr) {
                xTaskNotifyGive(xHandle);
            }
        }

        void registerMode(DisplayModes mode, DisplayMode* instance)
        {
            modes[static_cast<uint8_t>(mode)] = instance;
        }

        void begin()
        {
            Serial.println("Start display");
            xTaskCreatePinnedToCore(
                controlTask,
                "DisplayTask",
                8192,
                this,
                1,
                &xHandle,
                0
            );

            u8g2.setFont(u8g2_font_5x8_tf);
            setMode(DisplayModes::Main);
        }

        static void controlTask(void* pvParameters)
        {
            Display* display = static_cast<Display*>(pvParameters);
            for (;;)
            {
                uint32_t pending = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

                while (pending--)
                {
                // sleep until notified
                    // ulTaskNotifyTake(
                    //     pdTRUE,
                    //     portMAX_DELAY
                    // );

                    if (display->currentMode) {
                        display->clear();
                        display->currentMode->handleEvent();
                        display->draw();
                    }
                }
            }
        }

        void clear()
        {
            u8g2.clearBuffer();
        }

        void draw()
        {
            if (confirm.active) {
                clear();
                drawConfirm();
            }

            u8g2.sendBuffer();
        }

        void drawMode(const char* buffer)
        {
            u8g2.drawButtonUTF8(60, 30, U8G2_BTN_BW1, 0, 2, 2, buffer);
        }
};
