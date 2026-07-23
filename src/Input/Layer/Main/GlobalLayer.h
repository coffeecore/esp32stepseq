#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"
#include "Display/Workspace.h"

class GlobalLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 1, 999, false);
    }

    void applyEncoderValues() override
    {
        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, layerContext.sequencer.volume);
        layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, layerContext.sequencer.bpm);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            layerContext.sequencer.setVolume(inputEvent.value);
        }

        if (inputEvent.control == ControlId::Encoder1) {
            layerContext.sequencer.setBpm(inputEvent.value);
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                layerContext.sequencer.togglePause();

                break;

            case ControlId::Fn1:
                layerContext.sequencer.addQuarterNote();

                break;
            case ControlId::Fn2:
                layerContext.sequencer.toggleTrackMute(uiState.selectedTrack);

                break;

            case ControlId::Fn3:
                uiState.workspace = Workspace::Instrument;
                uiState.uiOverlay = UIOverlay::Menu;

                break;
        }
    }

    void onButtonHold(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                layerContext.sequencer.toggleStop();

                break;

            case ControlId::Fn1:
                inputContext.confirmAction = ConfirmAction::DeleteQuarterNote;
                uiState.openConfirm("Delete quarter note ?");

                break;

            case ControlId::Fn2:
                uiState.autoScroll = !uiState.autoScroll;

                break;
        }
    }
};
