#pragma once

#include "Input/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"

class LayerContext
{
    public:
        RotaryEncoder& rotaryEncoders;
        Sequencer& sequencer;

        LayerContext(Sequencer& seq, RotaryEncoder& enc)
            : rotaryEncoders(enc),
            sequencer(seq)
        {
        }
};
