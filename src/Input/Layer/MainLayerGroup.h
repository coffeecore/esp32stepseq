#pragma once

#include "Input/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"
#include "Input/Layer/LayerContext.h"
#include "Input/Layer/LayerGroup.h"
#include "Input/Layer/Main/GlobalLayer.h"
#include "Input/Layer/Main/StepEditLayer.h"
#include "Input/Layer/Main/StepLengthLayer.h"
#include "Input/Layer/Main/StepInstrumentLayer.h"
#include "Input/Layer/Main/QuarterNoteLengthLayer.h"
#include "Input/Layer/Main/NavigationLayer.h"
#include "Input/Layer/Main/ModalLayer.h"
#include "Input/Layer/Main/GlobalInstrumentLayer.h"
#include "Input/Layer/Main/TrackLayer.h"

class MainLayerGroup: public LayerGroup
{
    public:
        GlobalLayer global;
        StepEditLayer stepEdit;
        StepLengthLayer stepLength;
        StepInstrumentLayer stepInstrument;
        QuarterNoteLengthLayer quarterNoteLength;
        NavigationLayer navigation;
        ModalLayer modal;
        GlobalInstrumentLayer globalInstrument;
        TrackLayer track;

        MainLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, UIState& _ui)
            : LayerGroup(_inputContext, _layerContext, _ui),
            global(_inputContext, _layerContext, _ui),
            stepEdit(_inputContext, _layerContext, _ui),
            stepLength(_inputContext, _layerContext, _ui),
            stepInstrument(_inputContext, _layerContext, _ui),
            quarterNoteLength(_inputContext, _layerContext, _ui),
            navigation(_inputContext, _layerContext, _ui),
            modal(_inputContext, _layerContext, _ui),
            globalInstrument(_inputContext, _layerContext, _ui),
            track(_inputContext, _layerContext, _ui)
        {
        }

        Layer* layer() override
        {
            if (uiState.uiOverlay == UIOverlay::Confirm) {
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
