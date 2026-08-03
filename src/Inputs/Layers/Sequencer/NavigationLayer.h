#pragma once

#include "Inputs/Layers/Layer.h"
#include "Inputs/InputMode.h"
#include <Sequencer/Sequencer.h>

namespace Inputs::Layers::Sequencer
{
class NavigationLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        // uint8_t Tcount = max<uint8_t>(1, layerContext.sequencer.trackCounts);
        // if (0 < Tcount-1) {
        //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, Tcount-1, true);
        // }
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 255, true);

        // ::Sequencer::QuarterNote& quarterNote = layerContext.sequencer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
        // uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
        // if (0 < stepsCount-1) {
        //     layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 4, true);
        // }
        layerContext.rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 255, true);
    }

    void applyEncoderValues() override
    {
        uint8_t Tcount = max<uint8_t>(0, layerContext.sequencer.trackCounts);
        if (0 < Tcount - 1) {
            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder0, uiState.selectedTrack);
        }

        ::Sequencer::QuarterNote& quarterNote =
            layerContext.sequencer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];
        uint8_t stepsCount = max<uint8_t>(0, quarterNote.stepsCount);
        if (0 < stepsCount - 1) {
            layerContext.rotaryEncoders.syncEncoder(ControlId::Encoder1, uiState.selectedStep);
        }
    }

    void onEncoder(InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            if (inputEvent.delta > 0) {
                while (inputEvent.delta--) {
                    nextTrack();
                }
            }
            else if (inputEvent.delta < 0) {
                while (inputEvent.delta++) {
                    previousTrack();
                }
            }

            ::Sequencer::QuarterNote& quarterNote =
                layerContext.sequencer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];

            if (uiState.selectedStep >= quarterNote.stepsCount) {
                uiState.selectedStep = quarterNote.stepsCount - 1;
            }
        }

        if (inputEvent.control == ControlId::Encoder1) {
            if (inputEvent.delta > 0) {
                while (inputEvent.delta--) {
                    nextStep();
                }
            }
            else if (inputEvent.delta < 0) {
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
    void nextTrack()
    {
        uint8_t Tcount = max<uint8_t>(1, layerContext.sequencer.trackCounts);

        if (uiState.selectedTrack + 1 >= Tcount) {
            uiState.selectedTrack = 0;
        }
        else {
            uiState.selectedTrack++;
        }

        if (uiState.selectedTrack >= uiState.displayedTrack + Constants::NUMBER_OF_DISPLAYED_TRACKS) {
            uiState.displayedTrack = uiState.selectedTrack - Constants::NUMBER_OF_DISPLAYED_TRACKS + 1;
        }
        else if (uiState.selectedTrack < uiState.displayedTrack) {
            uiState.displayedTrack = uiState.selectedTrack;
        }
    }

    void previousTrack()
    {
        uint8_t Tcount = max<uint8_t>(1, layerContext.sequencer.trackCounts);

        if (uiState.selectedTrack == 0) {
            uiState.selectedTrack = Tcount - 1;
        }
        else {
            uiState.selectedTrack--;
        }

        if (uiState.selectedTrack >= uiState.displayedTrack + Constants::NUMBER_OF_DISPLAYED_TRACKS) {
            uiState.displayedTrack = uiState.selectedTrack - Constants::NUMBER_OF_DISPLAYED_TRACKS + 1;
        }
        else if (uiState.selectedTrack < uiState.displayedTrack) {
            uiState.displayedTrack = uiState.selectedTrack;
        }
    }

    void nextQuarterNote()
    {
        if (uiState.selectedQuarterNote + 1 >= layerContext.sequencer.quarterNoteCounts) {
            uiState.selectedQuarterNote = 0;
        }
        else {
            uiState.selectedQuarterNote++;
        }
    }

    void previousQuarterNote()
    {
        if (uiState.selectedQuarterNote == 0) {
            uiState.selectedQuarterNote = layerContext.sequencer.quarterNoteCounts - 1;
        }
        else {
            uiState.selectedQuarterNote--;
        }
    }

    void nextStep()
    {
        ::Sequencer::QuarterNote& quarterNote =
            layerContext.sequencer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];

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

        ::Sequencer::QuarterNote& quarterNote =
            layerContext.sequencer.tracks[uiState.selectedTrack].quarterNotes[uiState.selectedQuarterNote];

        uiState.selectedStep = quarterNote.stepsCount - 1;
    }
};
} // namespace Input::Layer::Sequencer
