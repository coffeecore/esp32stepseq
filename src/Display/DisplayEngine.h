#pragma once

class Display;   // forward declaration OK

class UIState;
class Sequencer;

#include "Screen/Screen.h"
#include "Screen/SequencerScreen.h"
#include "Screen/InstrumentMenuScreen.h"
#include "Screen/InstrumentAdsrScreen.h"
#include "Audio/IAudioEngine.h"

class DisplayEngine
{
    public:
        U8G2& u8g2;
        UIState& ui;
        UIOverlay previousOverlay;
        SequencerScreen sequencerScreen;
        InstrumentMenuScreen instrumentMenuScreen;
        InstrumentAdsrScreen instrumentAdsrScreen;
        Screen* currentScreen = nullptr;
        TaskHandle_t taskHandle = nullptr;

        explicit DisplayEngine(U8G2& u8g2, UIState& uiState, Sequencer& seq, IAudioEngine& audioEngine, MenuManager& menuManager)
            : u8g2(u8g2),
            ui(uiState),
            previousOverlay(uiState.uiOverlay),
            sequencerScreen(uiState, seq, u8g2, audioEngine),
            instrumentMenuScreen(uiState, seq, u8g2, audioEngine, menuManager),
            instrumentAdsrScreen(uiState, seq, u8g2, audioEngine, menuManager)
            
        {
            currentScreen = &sequencerScreen;
        }

        TaskHandle_t getTaskHandle() const
        {
            return taskHandle;
        }

        void resolveScreen()
        {
            Screen* nextScreen = nullptr;

            switch (ui.workspace)
            {
                case Workspace::Sequencer:
                    nextScreen = &sequencerScreen;
                    break;

                case Workspace::InstrumentMenu:
                    nextScreen = &instrumentMenuScreen;
                    break;

                case Workspace::InstrumentAdsr:
                    nextScreen = &instrumentAdsrScreen;
                    break;
            }

            const bool screenChanged = (nextScreen != currentScreen);
            const bool overlayChanged = (previousOverlay != ui.uiOverlay);

            if (!screenChanged && !overlayChanged) {
                return;
            }

            // L'overlay a change (quel qu'il soit) : l'ecran courant est
            // notifie qu'il en sort AVANT qu'on ne bascule currentScreen,
            // que ce basculement ait lieu ou non. Chaque ecran filtre en
            // interne sur l'overlay qui l'interesse (cf. InstrumentMenuScreen).
            if (overlayChanged && currentScreen) {
                currentScreen->onExit(previousOverlay);
            }

            if (screenChanged) {
                currentScreen = nextScreen;
            }

            if (overlayChanged) {
                currentScreen->onEnter(ui.uiOverlay);
            }

            previousOverlay = ui.uiOverlay;
        }

        void draw()
        {
            resolveScreen();

            u8g2.clearBuffer();

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
