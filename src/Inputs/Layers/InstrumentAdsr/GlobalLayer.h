#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include "Display/Workspace.h"
#include "Inputs/Layers/Common/GloabalLayer.h"

namespace Inputs::Layers::InstrumentAdsr
{

    class GlobalLayer : public Common::GlobalLayer
    {
    public:
        using Common::GlobalLayer::GlobalLayer;

        void onButtonTap(const InputEvent& event) override
        {
            switch (event.control) {
                case ControlId::Fn2:
                    layerContext.menuManager.back();

                    return;
            }

            Common::GlobalLayer::onButtonTap(event);
        }

        void onStepPressed(const InputEvent& inputEvent) override
        {
        }
    };
} // namespace Inputs::Layers::InstrumentAdsr
