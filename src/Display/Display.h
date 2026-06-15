#pragma once

#include "Arduino.h"
#include "Constants.h"
#include "SequencerTimer.h"
#include <U8g2lib.h>
// #include "InputMode.h"
#include "DisplayMode.h"

class Display
{
    public:
        Display(
            SequencerTimer& _sequencerTimer,
            U8G2& u8g2
        ): 
        sequencerTimer(_sequencerTimer),
        u8g2(u8g2)
        {
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
            u8g2.setFont(u8g2_font_6x12_tf);
            setMode(DisplayModes::Main);
            // u8g2.clearDisplay();
            // redraw();
            // requestRedraw();
        }

        // void requestRedraw()
        // {
        //     if (xHandle != nullptr) {
        //         xTaskNotifyGive(xHandle);
        //     }
        // }

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

                    // display->redraw();
                    if (display->currentMode) {
                        display->currentMode->handleEvent();
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
            u8g2.sendBuffer();
        }

        // void drawMode()
        // {
        //     char buf[21];
        //     snprintf(
        //         buf,
        //         sizeof(buf),
        //         "%s",
        //         currentMode
        //     );
        //     u8g2.drawStr(DisplayMode::toString(currentMode));
        // }

        // void redraw()
        // {
        //     u8g2.clearBuffer();

        //     // for (uint8_t i = 0;i<sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].stepCounts;i++) {
        //     //     // Serial.println("step");
        //     //     // Serial.println(sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].steps[i].state);

        //     //     char buf[21];
        //     //     snprintf(
        //     //         buf,
        //     //         sizeof(buf),
        //     //         "%d",
        //     //         sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].steps[i].state
        //     //     );
        //     //     u8g2.drawStr(0+(i*15), 30, buf);
        //     // }

        //     drawSequencer();
        //     drawTrack();
        //     drawPattern();

        //     u8g2.sendBuffer();
        // }

        void drawSequencer()
        {
            char buffer[21];

            snprintf(
                buffer,
                sizeof(buffer),
                "%3d %3d %1d%1d",
                sequencerTimer.volume,
                sequencerTimer.bpm,
                sequencerTimer.selectedTrack,
                sequencerTimer.selectedPattern
            );

            // u8g2.setFont(u8g2_font_6x12_tf);
            u8g2.drawStr(0, 14, buffer);
        }

        void drawTrack()
        {
            char buffer[21];

            Track& track = sequencerTimer.getTrack(sequencerTimer.selectedTrack);

            snprintf(
                buffer,
                sizeof(buffer),
                "%3d %3d",
                track.volume,
                track.transpose
            );

            // u8g2.setFont(u8g2_font_6x12_tf);
            u8g2.drawStr(0, 30, buffer);
        }

        void drawPattern()
        {
            // u8g2.setFont(u8g2_font_6x12_tf);
            for (uint8_t i = 0;i<4;i++) {
                drawQuarterNote(i);
            }
        }

        void drawQuarterNote(uint8_t quarterNoteIndex)
        {
            for (uint8_t j = 0;j<sequencerTimer.getQuarterNote(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex).stepsCount;j++) {
                // char buffer[21];
                // snprintf(
                //     buffer,
                //     sizeof(buffer),
                //     "%d",
                //     sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].steps[i].state
                // );
                // u8g2.drawStr(0+(i*15), 30, buffer);
                    drawStep(quarterNoteIndex, j, sequencerTimer.getStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex, j));
                }
            }

        void drawStep(uint8_t indexQ, uint8_t index, Step& step)
        {
            char buffer[21];
            snprintf(
                buffer,
                sizeof(buffer),
                "%d",
                step.state
            );
            u8g2.drawStr(0+(index*15), 30+(indexQ*12), buffer);
        }

    private:
        TaskHandle_t xHandle = nullptr;
        SequencerTimer& sequencerTimer;
        U8G2& u8g2;
        DisplayMode* modes[4] = {};
        DisplayMode* currentMode = nullptr;

};
