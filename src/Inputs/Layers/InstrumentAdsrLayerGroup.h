#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Inputs/InputContext.h"
#include "Inputs/Layers/Layer.h"
#include "Inputs/Layers/LayerContext.h"
#include "Inputs/Layers/LayerGroup.h"
#include "Inputs/Layers/InstrumentAdsr/GlobalLayer.h"
#include "Inputs/Layers/InstrumentAdsr/GlobalInstrumentLayer.h"
#include "Inputs/Layers/InstrumentAdsr/AttackLayer.h"
#include "Inputs/Layers/InstrumentAdsr/DecayLayer.h"
#include "Inputs/Layers/InstrumentAdsr/SustainLayer.h"
#include "Inputs/Layers/InstrumentAdsr/ReleaseLayer.h"

namespace Inputs::Layers
{
class InstrumentAdsrLayerGroup : public LayerGroup
{
public:
    InstrumentAdsr::GlobalLayer globalLayer;
    InstrumentAdsr::GlobalInstrumentLayer globalInstrumentLayer;
    InstrumentAdsr::AttackLayer attackLayer;
    InstrumentAdsr::DecayLayer decayLayer;
    InstrumentAdsr::SustainLayer sustainLayer;
    InstrumentAdsr::ReleaseLayer releaseLayer;

    InstrumentAdsrLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, Display::UIState& _ui)
        : LayerGroup(_inputContext, _layerContext, _ui)
        , globalLayer(_inputContext, _layerContext, _ui)
        , globalInstrumentLayer(_inputContext, _layerContext, _ui)
        , attackLayer(_inputContext, _layerContext, _ui)
        , decayLayer(_inputContext, _layerContext, _ui)
        , sustainLayer(_inputContext, _layerContext, _ui)
        , releaseLayer(_inputContext, _layerContext, _ui)
    {
    }

    Layer* layer() override
    {
        if (inputContext.fnMask & FN3) {
            Serial.println("ADSR GLOBALINSTRUMENT LAYER");

            return &globalInstrumentLayer;
        }

        if (inputContext.fnMask & FN4) {
            Serial.println("ADSR ATTACK LAYER");

            return &attackLayer;
        }

        if (inputContext.fnMask & FN5) {
            Serial.println("ADSR DECAY LAYER");

            return &decayLayer;
        }

        if (inputContext.fnMask & FN6) {
            Serial.println("ADSR SUSTAIN LAYER");

            return &sustainLayer;
        }

        if (inputContext.fnMask & FN7) {
            Serial.println("ADSR RELEASE LAYER");

            return &releaseLayer;
        }

        return &globalLayer;
    }
};

} // namespace Input::Layer
