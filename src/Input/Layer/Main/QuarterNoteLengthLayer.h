#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class QuarterNoteLengthLayer : public Layer
{
    public:
        using Layer::Layer;

        void onStepReleased(const InputEvent& inputEvent) override
        {
            uint8_t row = (static_cast<uint8_t>(inputEvent.control)) / 4;
            uint8_t col = (static_cast<uint8_t>(inputEvent.control)) % 4;

            layerContext.sequencer.setQuarterNoteStepsCount(uiState.displayedTrack + row, uiState.selectedQuarterNote, col + 1);  
        }
};
