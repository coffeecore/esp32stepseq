#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

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
        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder0, layerContext.sequencerTimer.volume);
        layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, layerContext.sequencerTimer.bpm);
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            layerContext.sequencerTimer.setVolume(inputEvent.value);
        }

        if (inputEvent.control == ControlId::Encoder1) {
            layerContext.sequencerTimer.setBpm(inputEvent.value);
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                layerContext.sequencerTimer.togglePause();

                break;

            case ControlId::Fn1:
                layerContext.sequencerTimer.addQuarterNote();

                break;
            case ControlId::Fn2:
                layerContext.sequencerTimer.toggleTrackMute(uiState.selectedTrack);

                break;
        }
    }

    void onButtonHold(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                layerContext.sequencerTimer.toggleStop();

                break;

            case ControlId::Fn1:
                inputContext.confirmAction = ConfirmAction::DeleteQuarterNote;
                inputContext.modal = ModalState::Confirm;
                uiState.confirm.text = "Delete quarter note ?";
                uiState.confirm.active = true;

                break;

            case ControlId::Fn2:
                uiState.autoScroll = !uiState.autoScroll;

                break;
        }
    }
};
