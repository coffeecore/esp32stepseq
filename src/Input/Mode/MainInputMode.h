#pragma once

#include "SequencerTimer.h"
#include "Input/Input.h"
#include "Input/InputMode.h"
#include "Display/Display.h"


class MainInputMode : public InputMode
{
    public:
        MainInputMode(
            Input& _input,
            SequencerTimer& _sequencerTimer,
            Display& _display
        )
            : InputMode(_input),
            sequencerTimer(_sequencerTimer),
            display(_display)
        {
        }

        void initEncoder()
        {
            input.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
            input.setEncoderValue(ControlId::Encoder0, sequencerTimer.volume);

            input.setEncoderBoundaries(ControlId::Encoder1, 1, 999, false);
            input.setEncoderValue(ControlId::Encoder1, sequencerTimer.bpm);
        }

        void onEnter() override
        {
            Serial.println("=== MAIN MODE ===");
            initEncoder();
        }

        void onExit() override
        {
            Serial.println(">>> MAIN MODE <<<");
        }

    protected:
        void fnReleasedAlways(const InputEvent& event, FnMask fnMask) override
        {
            // Le test avec & est donc généralement celui qu'on utilise pour savoir si une touche particulière faisait partie d'une combinaison.
            // if (fnMask & FN4) {
            //     initEncoder();
            // }

            // if (fnMask & FN5) {
            //     initEncoder();
            // }
            initEncoder();
        }

        void stepReleasedAlways(const InputEvent& event, FnMask fnMask) override
        {
            initEncoder();
        }

        void stepPressedAction(const InputEvent& event, FnMask fnMask) override
        {
            ControlId pressedStep = event.control;

            switch (fnMask)
            {
                case 0:
                    /** @todo set encoder 0 value and range to choose note */
                    /** @todo set encoder 1 value and range to choose octave */

                    break;
                case FN4:
                    /** @todo set encoder 0 value and range to choose length */
                    /** @todo set encoder 1 value and range to choose instrument */

                    break;
            
            default:
                break;
            }
        }

        void stepReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case 0:
                {
                    /** @todo toggle step state */
                    uint8_t row = (static_cast<uint8_t>(event.control)) / 4;
                    uint8_t col = (static_cast<uint8_t>(event.control)) % 4;
                    sequencerTimer.toggleStep(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col);

                    break;
                }
                default:
                    break;
            }
        }

        void fnPressedAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN4:
                {
                    /** @todo set encoder 0 value and range to choose track */
                    /** @todo set encoder 1 value and range to choose quarter note */
                    uint8_t Tcount = max<uint8_t>(1, sequencerTimer.trackCounts);
                    if (0 < Tcount-1) {
                        input.setEncoderBoundaries(ControlId::Encoder0, 0, Tcount-1, true);
                        input.setEncoderValue(ControlId::Encoder0, sequencerTimer.selectedTrack);
                    }

                    uint8_t Qcount = max<uint8_t>(1, sequencerTimer.quarterNoteCounts);
                    if (0 < Qcount-1) {
                        input.setEncoderBoundaries(ControlId::Encoder1, 0, Qcount-1, true);
                        input.setEncoderValue(ControlId::Encoder1, sequencerTimer.selectedQuarterNote);
                    }

                    break;
                }
                case FN5:
                    /** @todo  set encoder 0 value and range for track volume */
                    /** @todo  set encoder 1 value and range for track transpose */
                    break;

                case FN6:
                    /** @todo  set encoder 0 value and range to choose instrument */
                    break;
            
                default:
                    break;
            }
        }

        void fnReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN0:
                    /** @todo toggle play pause */

                    break;
                case FN1:
                    /** @todo loop through play mode */

                    break;
                case FN4:
                    /** @todo Go to steps edition screen */
                    break;

                case FN4 | FN7:
                    /** @todo add quarter note */
                    break;

                case FN5:
                    /** @todo Go to track edition screen */
                    break;
                
                case FN6:
                    /** @todo Go to instrument edition screen */
                    break;
                case FN7:
                    /** @todo clear entire screen */
                    break;
                case FN5 | FN7:
                    /** @todo clear display part selected track */
                    break;
                
                default:
                    break;
            }
        }

        void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN0:
                    /** @todo toggle play stop */

                    break;
                
                case FN4:
                    /** @todo Go to quarter note edition screen */
                    break;
                
                case FN5:
                    /** @todo Mute selected track */
                    break;

                case FN4 | FN7:
                    /** @todo remove last quarter note */
                    break;

                case FN5 | FN7:
                    /** @todo clear display selected track */
                    break;
                
                default:
                    break;
            }
        }

        void encoderAction(const InputEvent& event, FnMask fnMask)
        {
            ControlId encoder = event.control;

            switch (fnMask)
            {
                case 0:
                    if (encoder == ControlId::Encoder0) {
                        /** @todo  set sequencer volume */
                        sequencerTimer.setVolume(event.value);
                    }

                    if (encoder == ControlId::Encoder1) {
                        /** @todo  set sequencer bpm */
                        sequencerTimer.setBpm(event.value);
                    }

                    break;

                case FN4:
                    if (encoder == ControlId::Encoder0) {
                        /** @todo  choose track */

                        sequencerTimer.setSelectedTrack(event.value);

                        Serial.println("SELECT");
                        Serial.println(sequencerTimer.selectedTrack);
                        if (
                            event.value >= (display.displayedTrack+2)
                        ) {
                            display.displayedTrack = event.value-1;
                        }

                        if (event.value <= display.displayedTrack) {
                            display.displayedTrack = event.value;
                        }

                    }

                    if (encoder == ControlId::Encoder1) {
                        /** @todo  choose quarter note */
                        sequencerTimer.setSelectedQuarterNote(event.value);
                    }
                    break;

                case FN5:
                    if (encoder == ControlId::Encoder0) {
                        /** @todo  set track volume */
                        sequencerTimer.setTrackVolume(sequencerTimer.selectedTrack, event.value);
                    }

                    if (encoder == ControlId::Encoder1) {
                        /** @todo  set track transpose */
                        sequencerTimer.setTrackTranspose(sequencerTimer.selectedTrack, event.value);
                    }
                    break;

                case FN6:
                    if (encoder == ControlId::Encoder0) {
                        /** @todo  choose instrument */
                    }

                    break;
            
            default:
                break;
            }
        }
        

    private:
        SequencerTimer& sequencerTimer;
        Display& display;
        InputModes pendingMode = InputModes::None;
};
