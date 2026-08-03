#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Inputs/InputContext.h"
#include "Display/Workspace.h"
#include "Inputs/Layers/LayerContext.h"

namespace Inputs::Layers
{
class Layer
{
public:
    InputContext& inputContext;
    LayerContext& layerContext;
    Display::UIState& uiState;

    explicit Layer(InputContext& _inputContext, LayerContext& _layerContext, Display::UIState& ui)
        : inputContext(_inputContext)
        , layerContext(_layerContext)
        , uiState(ui)
    {
    }

    virtual void onStepPressed(const InputEvent& inputEvent)
    {
    }

    virtual void onStepReleased(const InputEvent& inputEvent)
    {
    }

    virtual void onEncoder(InputEvent& inputEvent)
    {
    }

    virtual void applyEncoderMapping()
    {
    }

    virtual void applyEncoderValues()
    {
    }

    // Appelé immédiatement au press d'un Fn (symétrique à
    // onStepPressed pour les Step). Contrairement à onButtonTap,
    // ne dépend ni du release ni de la durée du maintien.
    virtual void onButtonPressed(const InputEvent& event)
    {
    }

    virtual void onButtonTap(const InputEvent& event)
    {
    }

    virtual void onButtonHold(const InputEvent& event)
    {
    }

    // Permet à un layer de dire que le hold ne doit pas être pris
    // en compte pour ce bouton: onButtonHold ne sera pas appelé et
    // holdTriggered ne sera pas positionné, donc un onButtonTap
    // pourra toujours être déclenché au relâchement, quelle que
    // soit la durée du maintien.
    virtual bool wantsButtonHold(const InputEvent& event) const
    {
        return true;
    }

    virtual void onEnter(Layer* previous)
    {
    }

    virtual void onExit(Layer* next)
    {
    }
};
} // namespace Input::Layer
