#pragma once

#include "Display/Workspace.h"
#include "Sequencer/Sequencer.h"
#include "Display/Screen/Screen.h"
#include "Audio/Notes.h"

class InstrumentAdsrScreen : public MenuScreen
{
public:
    using MenuScreen::MenuScreen;

    void draw() override
    {
        char title[32];

        snprintf(title, sizeof(title),
                 "Instrument %u",
                 uiState.selectedInstrument + 1);

        // Title
        u8g2.drawStr(
            0,
            8,
            title
        );

        u8g2.drawHLine(
            0,
            10,
            128
        );

        
    }
};
