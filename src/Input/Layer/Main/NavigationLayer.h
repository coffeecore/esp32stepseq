#pragma once

#include "Input/Layer/Layer.h"
#include "Input/InputMode.h"

class NavigationLayer : public Layer
{
    public:
        using Layer::Layer;

        uint8_t previousEncoderValue = 0;
        
        void applyEncoderMapping() override
        {
            uint8_t Tcount = max<uint8_t>(1, layerContext.sequencerTimer.trackCounts);
            if (0 < Tcount-1) {
                layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, Tcount-1, true);
            }
            
            QuarterNote& quarterNote = layerContext.sequencerTimer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
            uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
            if (0 < stepsCount-1) {
                layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, stepsCount-1, false);
            }
        }
        
        void applyEncoderValues() override
        {
            uint8_t Tcount = max<uint8_t>(1, layerContext.sequencerTimer.trackCounts);
            if (0 < Tcount-1) {
                layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder0, uiState.selectedTrack);
            }
            
            QuarterNote& quarterNote = layerContext.sequencerTimer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
            uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
            if (0 < stepsCount-1) {
                layerContext.rotaryEncoders.setEncoderValue(ControlId::Encoder1, uiState.selectedStep);
                previousEncoderValue = uiState.selectedStep;
            }
        }
        
        void onEncoder(InputEvent& inputEvent) override
        {
            if (inputEvent.control == ControlId::Encoder0) {
                uiState.selectedTrack = inputEvent.value;
                
                if (uiState.selectedTrack >= uiState.displayedTrack + Constants::NUMBER_OF_DISPLAYED_TRACKS) {
                    uiState.displayedTrack = uiState.selectedTrack - 1;
                } else if (uiState.selectedTrack < uiState.displayedTrack) {
                    uiState.displayedTrack = uiState.selectedTrack;
                }
                
                QuarterNote& quarterNote = layerContext.sequencerTimer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
                
                if (uiState.selectedStep >= quarterNote.stepsCount) {
                    uiState.selectedStep = quarterNote.stepsCount - 1;
                }
            }
            
            if (inputEvent.control == ControlId::Encoder1) {
                if (inputEvent.delta > 0) {
                    while (inputEvent.delta--) {
                        nextStep();
                    }
                } else if (inputEvent.delta < 0) {
                    while (inputEvent.delta++) {
                        previousStep();
                    }
                }
            }
        }
        
        void onButtonTap(const InputEvent& event) override
        {
        }
        
        void onButtonHold(const InputEvent& event) override
        {
        }

    private:
        void nextQuarterNote()
        {
            if (uiState.selectedQuarterNote + 1 >= layerContext.sequencerTimer.quarterNoteCounts) {
                uiState.selectedQuarterNote = 0;
            } else {
                uiState.selectedQuarterNote++;
            }
        }
        
        void previousQuarterNote()
        {
            if (uiState.selectedQuarterNote == 0) {
                uiState.selectedQuarterNote =
                layerContext.sequencerTimer.quarterNoteCounts - 1;
            } else {
                uiState.selectedQuarterNote--;
            }
        }
        
        void nextStep()
        {
            QuarterNote& quarterNote = layerContext.sequencerTimer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
            
            uiState.selectedStep++;
            
            if (uiState.selectedStep >= quarterNote.stepsCount) {
                nextQuarterNote();
                uiState.selectedStep = 0;
            }
        }
        
        void previousStep()
        {
            if (uiState.selectedStep > 0) {
                uiState.selectedStep--;
                return;
            }
            
            previousQuarterNote();
            
            QuarterNote& quarterNote = layerContext.sequencerTimer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
            
            uiState.selectedStep = quarterNote.stepsCount - 1;
        }
};
