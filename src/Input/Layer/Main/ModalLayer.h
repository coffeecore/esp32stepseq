#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class ModalLayer : public Layer
{
    using Layer::Layer;

    public:
        void onButtonTap(const InputEvent& event) override
        {
            switch (event.control)
            {
                case ControlId::Fn0:
                    switch (inputContext.confirmAction)
                    {
                        case ConfirmAction::DeleteQuarterNote:
                            layerContext.sequencer.removeQuarterNote();

                            break;
                    }

                    inputContext.confirmAction = ConfirmAction::None;
                    uiState.closeConfirm();

                    break;

                case ControlId::Fn1:
                    inputContext.confirmAction = ConfirmAction::None;
                    uiState.closeConfirm();

                    break;

                case ControlId::Fn2:
                    // ...
                    break;

                case ControlId::Fn3:
                    // ...
                    break;
            }
        }
};
