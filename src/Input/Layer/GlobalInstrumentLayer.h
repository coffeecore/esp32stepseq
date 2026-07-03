#pragma once

#include "Layer.h"
#include "Input/InputMode.h"

class GlobalInstrumentLayer : public Layer
{
public:
    using Layer::Layer;

    void applyEncoderMapping() override
    {
        Track& track = sequencerTimer.tracks[sequencerTimer.selectedTrack];

        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder0, 0, 11, false);

        rotaryEncoders.setEncoderBoundaries(ControlId::Encoder1, 0, 11, false);
    }

    void applyEncoderValues() override
    {
        Track& track = sequencerTimer.tracks[sequencerTimer.selectedTrack];

        rotaryEncoders.setEncoderValue(ControlId::Encoder0, track.instrument);

        rotaryEncoders.setEncoderValue(ControlId::Encoder1, sequencerTimer.selectedInstrument);
    }

    void onEncoder(const InputEvent& inputEvent) override
    {
        if (inputEvent.control == ControlId::Encoder0) {
            sequencerTimer.setTrackInstrument(sequencerTimer.selectedTrack, inputEvent.value);
        }

        /** @implements  choose instrument global*/
        if (inputEvent.control == ControlId::Encoder1) {
            sequencerTimer.setSelectedInstrument(inputEvent.value);
        }
    }
};
