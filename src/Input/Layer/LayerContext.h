#pragma once

#include "Input/InputMode.h"
#include "SequencerTimer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"

class LayerContext
{
    public:
        RotaryEncoder& rotaryEncoders;
        SequencerTimer& sequencerTimer;

        LayerContext(SequencerTimer& seq, RotaryEncoder& enc)
            : rotaryEncoders(enc),
            sequencerTimer(seq)
        {
        }
};
