#pragma once

#include "Input/InputMode.h"
#include "Sequencer/Sequencer.h"
#include "Input/RotaryEncoder.h"
#include "Display/Workspace.h"
#include "Input/InputContext.h"
#include "Input/Layer/Layer.h"
#include "Input/Layer/LayerContext.h"
#include "Input/Layer/LayerGroup.h"
#include "Input/Layer/InstrumentAdsr/AttackLayer.h"

class InstrumentAdsrLayerGroup: public LayerGroup
{
    public:
        AttackLayer attackLayer;

        InstrumentAdsrLayerGroup(InputContext& _inputContext, LayerContext& _layerContext, UIState& _ui)
            : LayerGroup(_inputContext, _layerContext, _ui),
            attackLayer(_inputContext, _layerContext, _ui)
        {
        }

        Layer* layer() override
        {
            // if (uiState.uiOverlay == UIOverlay::Confirm) {
            //     Serial.println("MODAL LAYER");
            //     return &modal;
            // }

            // if (inputContext.stepHeld) {
            //     if (inputContext.fnMask & FN1) {
            //         Serial.println("QUARTER NOTE LENGTH LAYER");
            //         return &quarterNoteLength;

            //     }

            //     if (inputContext.fnMask & FN2) {
            //         Serial.println("STEP LENGTH LAYER");
            //         return &stepLength;

            //     }

            //     if (inputContext.fnMask & FN3) {
            //         Serial.println("STEP INSTRUMENT LAYER");
            //         return &stepInstrument;

            //     }

            //     Serial.println("STEP EDIT LAYER");
            //     return &stepEdit;

            // }

            // if (inputContext.fnMask & FN1) {
            //     Serial.println("NAVIGATION LAYER");
            //     return &navigation;

            // }

            // if (inputContext.fnMask & FN2) {
            //     Serial.println("TRACK LAYER");
            //     return &track;

            // }

            // if (inputContext.fnMask & FN3) {
            //     Serial.println("GLOBAL INSTRUMENT LAYER");
            //     return &globalInstrument;

            // }

            if (inputContext.fnMask & FN4) {
                Serial.println("ADSR ATTACK LAYER");

                return &attackLayer;
            }
            // if (inputContext.fnMask & FN5) return &decay;
            // if (inputContext.fnMask & FN6) return &sustain;
            // if (inputContext.fnMask & FN7) return &release;
            
            return nullptr;
        }
};
