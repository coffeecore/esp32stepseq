#pragma once

#include "Display/Display.h"
#include "Display/DisplayMode.h"
#include "SequencerTimer.h"

class StepDisplayMode : public DisplayMode
{
public:
    StepDisplayMode(
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
