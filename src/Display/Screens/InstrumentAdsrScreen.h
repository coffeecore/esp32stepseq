#pragma once

#include "Display/Workspace.h"
#include "Sequencer/Sequencer.h"
#include "Display/Screens/Screen.h"
#include "Audio/Notes.h"

namespace Display::Screens
{
    class InstrumentAdsrScreen : public MenuScreen
    {
    public:
        using MenuScreen::MenuScreen;

        void draw() override
        {
            char title[32];

            snprintf(title, sizeof(title), "Instrument %u", uiState.selectedInstrument + 1);

            // Title
            u8g2.drawStr(0, 8, title);

            u8g2.drawHLine(0, 10, 128);

            constexpr uint8_t FIRST_ROW = 21;
            constexpr uint8_t ROW_HEIGHT = 8;

            uint8_t y = FIRST_ROW + (0 * ROW_HEIGHT);

            u8g2.drawStr(0, y, "Attack");
        }
    };
}
