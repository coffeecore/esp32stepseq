#pragma once

#include "Display/Display.h"
#include "Display/DisplayMode.h"
#include "SequencerTimer.h"

class QuarterNoteDisplayMode : public DisplayMode
{
public:
    QuarterNoteDisplayMode(
        Display& _display,
        SequencerTimer& _sequencerTimer
    )
        : DisplayMode(_display),
        sequencerTimer(_sequencerTimer)
    {
    }

    void handleEvent() override
    {
    }

    private:
        SequencerTimer& sequencerTimer;
};
