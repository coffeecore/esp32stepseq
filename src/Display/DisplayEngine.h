#pragma once

class Display;   // forward declaration OK

class UIState;
class Sequencer;

#include "Screen/Screen.h"
#include "Screen/SequencerScreen.h"

class DisplayEngine
{
    public:
        U8G2& u8g2;
        UIState& ui;
        Sequencer& sequencer;

        SequencerScreen sequencerScreen;

        Screen* currentScreen = nullptr;

        TaskHandle_t taskHandle = nullptr;

        explicit DisplayEngine(U8G2& u8g2, UIState& uiState, Sequencer& seq)
            : u8g2(u8g2),
            ui(uiState),
            sequencer(seq),
            sequencerScreen(uiState, seq, u8g2)
        {
            currentScreen = &sequencerScreen;
        }

        TaskHandle_t getTaskHandle() const
        {
            return taskHandle;
        }

        void resolveScreen()
        {
            switch (ui.workspace)
            {
                case Workspace::Sequencer:
                    currentScreen = &sequencerScreen;
                    break;
            }
        }

        void draw()
        {
            resolveScreen();

            u8g2.clearBuffer();

            if (ui.confirm.active) {
                drawConfirm();
                u8g2.sendBuffer();
                return;
            }

            if (currentScreen) {
                currentScreen->draw();
            }

            u8g2.sendBuffer();
        }

        static void taskLoop(void* pv)
        {
            DisplayEngine* self = static_cast<DisplayEngine*>(pv);

            for (;;)
            {
                uint32_t pending = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                while (pending--) {
                }
                self->draw();
            }
        }

        void drawConfirm()
        {
            u8g2.drawFrame(10,18,108,28);

            u8g2.setFont(u8g2_font_5x8_tf);

            u8g2.drawStr(20,32,ui.confirm.text);

            u8g2.drawStr(20,45,"YES");
            u8g2.drawStr(80,45,"NO");
        }

        void begin()
        {
            xTaskCreatePinnedToCore(
                taskLoop,
                "DisplayTask",
                8192,
                this,
                1,
                &taskHandle,
                0
            );

            ui.displayTask = taskHandle;
            u8g2.setFont(u8g2_font_5x8_tf);
        }
};
