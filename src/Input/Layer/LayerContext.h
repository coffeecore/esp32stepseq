#pragma once

#include "Input/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"
#include "Menu/MenuManager.h"

class LayerContext
{
    public:
        RotaryEncoder& rotaryEncoders;
        Sequencer& sequencer;
        MenuManager& menuManager;
        IAudioEngine& audioEngine;

        LayerContext(Sequencer& sequencer, RotaryEncoder& rotaryEncoders, MenuManager& menuManager, IAudioEngine& audioEngine)
            : rotaryEncoders(rotaryEncoders),
            sequencer(sequencer),
            menuManager(menuManager),
            audioEngine(audioEngine)
        {
        }
};
