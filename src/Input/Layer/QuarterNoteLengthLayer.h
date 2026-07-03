#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class QuarterNoteLengthLayer : public Layer
{
    public:
        using Layer::Layer;

        void onStepReleased(const InputEvent& inputEvent) override
        {
            /** @implements change quarter note length */
            uint8_t row = (static_cast<uint8_t>(inputEvent.control)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputEvent.control)) % 4;
            sequencerTimer.setQuarterNoteStepsCount(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col + 1);  
        }
};
