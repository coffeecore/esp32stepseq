#pragma once

#include "Inputs/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Inputs/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Inputs/InputContext.h"
#include "Inputs/Layers/Layer.h"
#include "Inputs/Layers/LayerContext.h"
#include "Inputs/Layers/LayerGroup.h"
#include "Inputs/Layers/Sequencer/GlobalLayer.h"
#include "Inputs/Layers/Sequencer/StepEditLayer.h"
#include "Inputs/Layers/Sequencer/StepLengthLayer.h"
#include "Inputs/Layers/Sequencer/StepInstrumentLayer.h"
#include "Inputs/Layers/Sequencer/QuarterNoteLengthLayer.h"
#include "Inputs/Layers/Sequencer/NavigationLayer.h"
#include "Inputs/Layers/Sequencer/ModalLayer.h"
#include "Inputs/Layers/Sequencer/GlobalInstrumentLayer.h"
#include "Inputs/Layers/Sequencer/TrackLayer.h"

namespace Inputs::Layers
{

class MainLayerGroup : public LayerGroup
{
public:
    Sequencer::GlobalLayer global;
    Sequencer::StepEditLayer stepEdit;
    Sequencer::StepLengthLayer stepLength;
    Sequencer::StepInstrumentLayer stepInstrument;
    Sequencer::QuarterNoteLengthLayer quarterNoteLength;
    Sequencer::NavigationLayer navigation;
    Sequencer::ModalLayer modal;
    Sequencer::GlobalInstrumentLayer globalInstrument;
    Sequencer::TrackLayer track;

    MainLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, Display::UIState& _ui)
        : LayerGroup(_inputContext, _layerContext, _ui)
        , global(_inputContext, _layerContext, _ui)
        , stepEdit(_inputContext, _layerContext, _ui)
        , stepLength(_inputContext, _layerContext, _ui)
        , stepInstrument(_inputContext, _layerContext, _ui)
        , quarterNoteLength(_inputContext, _layerContext, _ui)
        , navigation(_inputContext, _layerContext, _ui)
        , modal(_inputContext, _layerContext, _ui)
        , globalInstrument(_inputContext, _layerContext, _ui)
        , track(_inputContext, _layerContext, _ui)
    {
    }

    Layer* layer() override
    {
        if (uiState.uiOverlay == Display::UIOverlay::Confirm) {
            Serial.println("MODAL LAYER");
            return &modal;
        }

        if (inputContext.stepHeld) {
            if (inputContext.fnMask & FN1) {
                Serial.println("QUARTER NOTE LENGTH LAYER");
                return &quarterNoteLength;
            }

            if (inputContext.fnMask & FN2) {
                Serial.println("STEP LENGTH LAYER");
                return &stepLength;
            }

            if (inputContext.fnMask & FN3) {
                Serial.println("STEP INSTRUMENT LAYER");
                return &stepInstrument;
            }

            Serial.println("STEP EDIT LAYER");
            return &stepEdit;
        }

        if (inputContext.fnMask & FN1) {
            Serial.println("NAVIGATION LAYER");
            return &navigation;
        }

        if (inputContext.fnMask & FN2) {
            Serial.println("TRACK LAYER");
            return &track;
        }

        if (inputContext.fnMask & FN3) {
            Serial.println("GLOBAL INSTRUMENT LAYER");
            return &globalInstrument;
        }

        Serial.println("GLOBAL LAYER");
        return &global;
    }
};
} // namespace Input::Layer
