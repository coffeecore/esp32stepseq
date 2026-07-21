#pragma once

#include "Display/Workspace.h"
#include "Sequencer/Sequencer.h"
#include "Display/Screen/Screen.h"
#include "Audio/Notes.h"

class InstrumentScreen : public MenuScreen
{
public:
    Menu menu;

    InstrumentScreen(UIState& u,
                     Sequencer& s,
                     U8G2& d,
                     IAudioEngine& a, MenuManager& m)
        : MenuScreen(u, s, d, a)
    {
    }

    void draw() override
    {
    }
};
