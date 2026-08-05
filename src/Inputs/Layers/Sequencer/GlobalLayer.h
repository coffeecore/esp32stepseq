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

        void onButtonTap(const InputEvent& event) override
        {
            switch (event.control) {
                case ControlId::Fn2:
                    layerContext.sequencer.toggleTrackMute(uiState.selectedTrack);

                    return;

                case ControlId::Fn3:
                    uiState.workspace = Display::Workspace::InstrumentMenu;
                    uiState.uiOverlay = Display::UIOverlay::Menu;

                    return;
            }

            Common::GlobalLayer::onButtonTap(event);
        }

        void onButtonHold(const InputEvent& event) override
        {
            switch (event.control) {
                case ControlId::Fn1:
                    inputContext.confirmAction = ConfirmAction::DeleteQuarterNote;
                    uiState.openConfirm("Delete quarter note ?");

                    return;

                case ControlId::Fn2:
                    uiState.autoScroll = !uiState.autoScroll;

                    return;
            }

            Common::GlobalLayer::onButtonTap(event);
        }
    };
} // namespace Inputs::Layers::Sequencer
