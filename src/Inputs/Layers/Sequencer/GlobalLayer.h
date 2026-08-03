#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include "Display/Workspace.h"
#include "Inputs/Layers/Common/GloabalLayer.h"

namespace Inputs::Layers::Sequencer
{
class GlobalLayer : public Common::GlobalLayer
{
public:
    using Common::GlobalLayer::GlobalLayer;

    // void applyEncoderMapping() override
    // {
    //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
    //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 1, 999, false);
    // }

    // void applyEncoderValues() override
    // {
    //     layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, layerContext.sequencer.volume);
    //     layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, layerContext.sequencer.bpm);
    // }

    // void onEncoder(InputEvent& inputEvent) override
    // {
    //     if (inputEvent.control == ControlId::Encoder0) {
    //         layerContext.sequencer.setVolume(inputEvent.value);
    //     }

    //     if (inputEvent.control == ControlId::Encoder1) {
    //         layerContext.sequencer.setBpm(inputEvent.value);
    //     }
    // }

    void onButtonTap(const InputEvent& event) override
    {
        switch (event.control) {
            // case ControlId::Fn0:
            //     layerContext.sequencer.togglePause();

            //     break;

            // case ControlId::Fn1:
            //     layerContext.sequencer.addQuarterNote();

                // break;
            case ControlId::Fn2:
                layerContext.sequencer.toggleTrackMute(uiState.selectedTrack);

                return;

            case ControlId::Fn3:
                uiState.workspace = Display::Workspace::InstrumentMenu;
                uiState.uiOverlay = Display::UIOverlay::Menu;

                return;
        }
    }

    void onButtonHold(const InputEvent& event) override
    {
        switch (event.control) {
            // case ControlId::Fn0:
            //     layerContext.sequencer.toggleStop();

            //     break;

            case ControlId::Fn1:
                inputContext.confirmAction = ConfirmAction::DeleteQuarterNote;
                uiState.openConfirm("Delete quarter note ?");

                return;

            case ControlId::Fn2:
                uiState.autoScroll = !uiState.autoScroll;

                return;
        }
    }
};
} // namespace Input::Layer::Sequencer
