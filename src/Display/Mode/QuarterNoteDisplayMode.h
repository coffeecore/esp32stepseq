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
        // Serial.println("[Display][MAIN MODE] Handle");

        display.clear();

        // display.drawSequencer();
        // display.drawTrack();
        // display.drawPattern();

        display.drawMode("Quarter note mode");

        display.draw();
    }

    private:
        SequencerTimer& sequencerTimer;
};
