#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Inputs/InputContext.h"
#include "Inputs/Layers/Layer.h"
#include "Menu/MenuManager.h"

namespace Inputs::Layers
{

class LayerContext
{
public:
    RotaryEncoder& rotaryEncoders;
    ::Sequencer::Sequencer& sequencer;
    MenuManager& menuManager;
    IAudioEngine& audioEngine;

    LayerContext(::Sequencer::Sequencer& sequencer, RotaryEncoder& rotaryEncoders, MenuManager& menuManager,
                 IAudioEngine& audioEngine)
        : rotaryEncoders(rotaryEncoders)
        , sequencer(sequencer)
        , menuManager(menuManager)
        , audioEngine(audioEngine)
    {
    }
};
} // namespace Input::Layer
