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
        U8G2& u8g2;
        uint8_t displayedTrack = 0;

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
            // u8g2.setFont(u8g2_font_6x12_tf);
            u8g2.setFont(u8g2_font_5x8_tf);
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

        void drawMode(const char* buffer)
        {
            u8g2.drawButtonUTF8(60, 30, U8G2_BTN_BW1, 0, 2, 2, buffer);
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
            // char buffer[30];

            // snprintf(
            //     buffer,
            //     sizeof(buffer),
            //     "%3d%3d%+03d %02X %1d%3d%+03d%+03d%01XZ",
            //     sequencerTimer.volume,
            //     sequencerTimer.bpm,
            //     // sequencerTimer.pan,
            //     // sequencerTimer.selectedPattern
            //     sequencerTimer.selectedQuarterNote,

            //     sequencerTimer.selectedTrack,
            //     sequencerTimer.tracks[sequencerTimer.selectedTrack].volume,
            //     sequencerTimer.tracks[sequencerTimer.selectedTrack].transpose,
            //     // sequencerTimer.tracks[sequencerTimer.selectedTrack].pan,
            //     sequencerTimer.tracks[sequencerTimer.selectedTrack].instrument
            // );

            // // u8g2.setFont(u8g2_font_6x12_tf);
            // // u8g2.drawStr(0, 6, buffer);
            // u8g2.drawStr(0, u8g2.getAscent() + 0 * u8g2.getMaxCharHeight(), buffer);

            // u8g2.drawFrame(0, 16, 128, 12);
            // u8g2.drawFrame(0, 28, 128, 12);
            // u8g2.drawFrame(0, 40, 128, 12);
            // u8g2.drawFrame(0, 52, 128, 12);

            // for (uint8_t i=0;i<4;i++) {
            //     for (uint8_t j=0;j<4;j++) {
            //         u8g2.drawFrame(32*j, 16+(12*i), 32, 12);
            //     }
            // }
        }

        void drawTrack()
        {
            // char buffer[21];

            // // Track& track = sequencerTimer.getTrack(sequencerTimer.selectedTrack);
            // Track& track = sequencerTimer.tracks[sequencerTimer.selectedTrack];

            // snprintf(
            //     buffer,
            //     sizeof(buffer),
            //     "%3d %3d",
            //     track.volume,
            //     track.transpose
            // );

            // // u8g2.setFont(u8g2_font_6x12_tf);
            // u8g2.drawStr(0, 30, buffer);

            // for (uint8_t i=0;i<Constants::MAX_TRACKS;i++) {
            //     char buffer[21];
            //      Track& track = sequencerTimer.tracks[i];

            //     snprintf(
            //         buffer,
            //         sizeof(buffer),
            //         "Tr%1d %01X",
            //         i,
            //         track.instrument
            //     );

            //     // u8g2.drawStr(i*32, 14, buffer);
            //     u8g2.drawStr(i*32, u8g2.getAscent() + 1 * u8g2.getMaxCharHeight(), buffer);
            // }
        }

        void drawPattern()
        {
            // u8g2.setFont(u8g2_font_6x12_tf);
            // for (uint8_t i = 0;i<4;i++) {
            //     drawQuarterNote(i);
            // }
            // for (uint8_t i = 0;i<Constants::MAX_TRACKS;i++) {
            //     drawQuarterNote(i);
            // }
        }

        // void drawQuarterNote(uint8_t quarterNoteIndex)
        // {
        //     // for (uint8_t j = 0;j<sequencerTimer.getQuarterNote(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex).stepsCount;j++) {
        //     for (uint8_t j = 0;j<sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[quarterNoteIndex].stepsCount;j++) {
        //         // char buffer[21];
        //         // snprintf(
        //         //     buffer,
        //         //     sizeof(buffer),
        //         //     "%d",
        //         //     sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].steps[i].state
        //         // );
        //         // u8g2.drawStr(0+(i*15), 30, buffer);
        //             drawStep(quarterNoteIndex, j, sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[quarterNoteIndex].steps[j]);
        //             // drawStep(quarterNoteIndex, j, sequencerTimer.getStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex, j));
        //         }
        //     }
        void drawQuarterNote(uint8_t trackIndex)
        {
            // Serial.println("DRAW QUARTER");
            // Serial.println(trackIndex);
            // Serial.println(sequencerTimer.selectedQuarterNote);
            // Serial.println(sequencerTimer.tracks[trackIndex].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount);
            // for (uint8_t j = 0;j<sequencerTimer.getQuarterNote(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex).stepsCount;j++) {
            // for (uint8_t j = 0;j<sequencerTimer.tracks[trackIndex].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount;j++) {
            //     // char buffer[21];
            //     // snprintf(
            //     //     buffer,
            //     //     sizeof(buffer),
            //     //     "%d",
            //     //     sequencerTimer.getTrack(sequencerTimer.getCurrentTrack()).patterns[sequencerTimer.getCurrentPattern()].steps[i].state
            //     // );
            //     // u8g2.drawStr(0+(i*15), 30, buffer);
            //         drawStep(j, trackIndex, sequencerTimer.tracks[trackIndex].quarterNotes[sequencerTimer.selectedQuarterNote].steps[j]);
            //         // drawStep(quarterNoteIndex, j, sequencerTimer.getStep(sequencerTimer.selectedTrack, sequencerTimer.selectedPattern, quarterNoteIndex, j));
            //     }
        }

        void drawStep(uint8_t indexQ, uint8_t index, Step& step)
        {
            // QuarterNote& qn = sequencerTimer.tracks[index].quarterNotes[sequencerTimer.selectedQuarterNote];
            // uint8_t sc = qn.stepsCount;
            // uint8_t height = 48 / sc;

            // uint8_t len = (height-2) * step.length / qn.ticksByStep;

            // Serial.println("Track");
            // Serial.println(index);
            // Serial.println("SC");
            // Serial.println(sc);
            // Serial.println("HIGH");
            // Serial.println(height);
            // Serial.println("LEN");
            // Serial.println(len);
            // // u8g2.drawFrame(index*32, 16+(indexQ * height), 32, height);

            // char buffer[7];
            // // snprintf(
            // //     buffer,
            // //     sizeof(buffer),
            // //     "%d",
            // //     step.state
            // // );
            // // u8g2.drawStr((index*32), 24+(indexQ*8), buffer);
            // if (step.state) {
                
            //     u8g2.drawLine(index*32, 16+(indexQ * height)+1, index*32, 16+(indexQ * height)+len);
            // //     u8g2.drawFrame((index*32), 17+(indexQ*12), 3, 10);
            //     snprintf(
            //         buffer,
            //         sizeof(buffer),
            //         "%s%01X",
            //         step.noteStr,
            //         14
            //     );

            //     u8g2.drawStr((index*32)+2, 24+(indexQ * height), buffer);
            // } else {
            //     u8g2.drawStr((index*32)+2, 24+(indexQ * height), "----");
            // //     // u8g2.drawBox((index*32), 24+(indexQ*8)+2, 3, 4);
            // //     u8g2.drawLine((index*32), 17+(indexQ*12)+4, (index*32)+2,17+(indexQ*12)+4);
            // //     // u8g2.drawGlyph((index*32), 16+(indexQ*8)+8, '.');
            // //     // u8g2.drawPixel((index*32), 16+(indexQ*8)+8);
            // }
            // u8g2.drawStr((index*32)+5, 25+(indexQ*12), "C#4B");
            // u8g2.drawStr((index*32)+5, 25+(indexQ*12), "C");
            // u8g2.drawStr((index*32)+10, 25+(indexQ*12), "#");
            // u8g2.drawStr((index*32)+15, 25+(indexQ*12), "4");
            // u8g2.drawStr((index*32)+20, 25+(indexQ*12), "B");
                        // u8g2.setFont(u8g2_font_4x6_tf);

            // u8g2.drawStr((index*32)+25, 22+(indexQ*12), "s");
            // u8g2.drawStr((index*32)+25, 25+(indexQ*12), "a");

                        // u8g2.setFont(u8g2_font_5x8_tf);

        }

    private:
        TaskHandle_t xHandle = nullptr;
        SequencerTimer& sequencerTimer;
        DisplayMode* modes[4] = {};
        DisplayMode* currentMode = nullptr;

};
