#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class GlobalLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 1, 999, false);
    }

    void applyEncoderValues() override
    {
        rotaryEncoders.setEncoderValue(ControlId::Encoder0, sequencerTimer.volume);
        rotaryEncoders.setEncoderValue(ControlId::Encoder1, sequencerTimer.bpm);
    }

    void onEncoder(const InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            /** @implements  set sequencer volume */
            sequencerTimer.setVolume(inputEvent.value);
        }

        if (inputEvent.control == ControlId::Encoder1) {
            /** @implements  set sequencer bpm */
            sequencerTimer.setBpm(inputEvent.value);
        }
    }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                sequencerTimer.togglePause();

                break;

            case ControlId::Fn1:
                Serial.println("Add quarterNote");
                sequencerTimer.addQuarterNote();

                break;
        }
    }

    void onButtonHold(const InputEvent& event) override
    {
        switch (event.control)
        {
            case ControlId::Fn0:
                sequencerTimer.toggleStop();

                break;

            case ControlId::Fn1:
                inputContext.confirmAction = ConfirmAction::DeleteQuarterNote;
                inputContext.modal = ModalState::Confirm;
                uiState.confirm.text = "Delete quarter note ?";
                uiState.confirm.active = true;

                break;

            case ControlId::Fn2:
                sequencerTimer.toggleTrackMute(sequencerTimer.selectedTrack);

                break;
        }
    }
};
