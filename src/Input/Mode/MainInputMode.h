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
            /**
             * @todo constant for volume min max, bpm min max
             */
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
            initEncoder();
        }

        void stepReleasedAlways(const InputEvent& event, FnMask fnMask) override
        {
            initEncoder();
        }

        void stepPressedAction(const InputEvent& event, FnMask fnMask) override
        {
            ControlId pressedStep = event.control;

            uint8_t row = (static_cast<uint8_t>(event.control)) / 4;
            uint8_t col = (static_cast<uint8_t>(event.control)) % 4;

            QuarterNote& qn = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote];

            Step& step = qn.steps[col];

            switch (fnMask)
            {
                case 0:
                {
                    /**
                     * @todo constant for note range
                     * @implemented set encoder 0 value and range to choose note
                     */
                    input.setEncoderBoundaries(ControlId::Encoder0, 0, 11, true);
                    input.setEncoderValue(ControlId::Encoder0, step.note % 12);
                    /**
                     * @todo constant for octave range
                     * @implemented set encoder 1 value and range to choose octave
                     */
                    input.setEncoderBoundaries(ControlId::Encoder1, -1, 9, true);
                    input.setEncoderValue(ControlId::Encoder1, step.note / 12 - 1);

                    break;
                }
                case FN4:
                    /** @todo set encoder 0 value and range to choose length */
                    input.setEncoderBoundaries(ControlId::Encoder0, 1, qn.ticksByStep, false);
                    input.setEncoderValue(ControlId::Encoder0, step.length);

                    /** 
                     * @todo constant max instrument number
                     * @todo set encoder 1 value and range to choose instrument
                     */
                    input.setEncoderBoundaries(ControlId::Encoder1, -1, 15, false);
                    input.setEncoderValue(ControlId::Encoder1, step.instrument);

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
                    /** @implemented toggle play pause */
                    sequencerTimer.togglePause();

                    break;
                case FN1:
                    /** @todo loop through play mode */

                    break;
                case FN4:
                    /** @todo Go to steps edition screen */
                    break;

                case FN4 | FN7:
                    /** @implemented add quarter note */
                    sequencerTimer.addQuarterNote();
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
                    /** @implemented toggle play stop */
                    sequencerTimer.toggleStop();

                    break;
                
                case FN4:
                    /** @todo Go to quarter note edition screen */
                    break;
                
                case FN5:
                    /** @todo Mute selected track */
                    break;

                case FN4 | FN7:
                    /** @todo remove last quarter note */
                    sequencerTimer.removeQuarterNote();
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

            if (stepState.pressed) {
                uint8_t row = (static_cast<uint8_t>(stepState.stepId)) / 4;
                uint8_t col = (static_cast<uint8_t>(stepState.stepId)) % 4;

                Step& step = sequencerTimer.tracks[display.displayedTrack + row].quarterNotes[sequencerTimer.selectedQuarterNote].steps[col];

                switch (fnMask)
                {
                    case 0:
                    {   
                        if (encoder == ControlId::Encoder0) {
                            sequencerTimer.setStepNote(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, event.value, step.note / 12 - 1);
                        }

                        if (encoder == ControlId::Encoder1) {
                            sequencerTimer.setStepNote(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, step.note % 12, event.value);
                        }


                        break;
                    }
                    case FN4:
                    {
                        if (encoder == ControlId::Encoder0) {
                            sequencerTimer.setStepLength(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, event.value);
                        }

                        if (encoder == ControlId::Encoder1) {
                            sequencerTimer.setStepInstrument(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, event.value);
                        }

                        break;
                    }
                    default:
                        break;
                }
                
                return;
            }

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





Vu ta structure, ne réécris rien. Le plus simple est d'ajouter un petit état de confirmation dans MainInputMode et d'intercepter seulement les actions qui doivent confirmer.

Tu as déjà une architecture avec InputMode, donc fais-le dedans.

1) Ajoute un état privé dans MainInputMode

En bas :

private:
    SequencerTimer& sequencerTimer;
    Display& display;
    InputModes pendingMode = InputModes::None;

    enum class ConfirmAction {
        None,
        DeleteQuarterNote,
        ClearTrack
    };

    ConfirmAction confirmAction = ConfirmAction::None;
2) Ajoute une fonction d'affichage

Dans MainInputMode :

void showConfirm(const char* text)
{
    display.u8g2.clearBuffer();

    display.u8g2.drawFrame(10, 18, 108, 28);
    display.u8g2.setFont(u8g2_font_6x10_tf);

    display.u8g2.drawStr(20, 32, text);
    display.u8g2.drawStr(20, 45, "YES");
    display.u8g2.drawStr(80, 45, "NO");

    display.u8g2.sendBuffer();
}

(adapte juste le nom de ton objet U8g2 dans Display)

3) Quand tu fais ton delete, au lieu de supprimer

Tu as actuellement :

case FN4 | FN7:
    /** @todo remove last quarter note */
    sequencerTimer.removeQuarterNote();
    break;

Tu remplaces juste par :

case FN4 | FN7:
    confirmAction = ConfirmAction::DeleteQuarterNote;
    showConfirm("DELETE?");
    break;
4) Tu récupères la réponse avec un bouton

Dans fnReleaseAction au début :

if (confirmAction != ConfirmAction::None)
{
    if (fnMask == FN0)   // exemple : YES
    {
        if (confirmAction == ConfirmAction::DeleteQuarterNote)
        {
            sequencerTimer.removeQuarterNote();
        }

        confirmAction = ConfirmAction::None;
        return;
    }

    if (fnMask == FN1)   // NO
    {
        confirmAction = ConfirmAction::None;
        return;
    }
}

Avantage : tu ne touches pas à ton système de FnMask, InputMode, StepState, etc.
Tu ajoutes juste une couche UI au-dessus.

Vu ton code, je ferais même un petit ModalState dans InputMode plus tard, mais là ça suffit sans




Oui, un `ModalState` serait plus propre dans ton cas. Ton `InputMode` gère déjà des états d'entrée (`Fn`, `Step`, encodeurs), donc au lieu d'ajouter plein de `if (confirmAction...)`, tu ajoutes un **état UI temporaire**.

L'idée :

```
NORMAL
  |
  | FN4+FN7
  v
CONFIRM_DELETE
  |
  +--> YES -> suppression -> NORMAL
  |
  +--> NO  -> NORMAL
```

### Dans `InputMode.h`

Ajoute un enum :

```cpp
enum class ModalState
{
    None,
    Confirm
};
```

Puis dans `InputMode` :

```cpp
ModalState modalState = ModalState::None;
```

et éventuellement :

```cpp
virtual void modalAction(const InputEvent& event, FnMask fnMask)
{
}
```

---

Ensuite dans `handleEvent()` tu ajoutes juste au début :

```cpp
if (modalState != ModalState::None)
{
    modalAction(event, fnState.pressedMask);
    return;
}
```

Ça veut dire : si une fenêtre est ouverte, les touches ne tombent pas dans le reste du système.

---

Dans `MainInputMode` :

```cpp
enum class ConfirmAction
{
    None,
    DeleteQuarterNote
};

ConfirmAction confirmAction = ConfirmAction::None;
```

Quand tu demandes confirmation :

```cpp
case FN4 | FN7:

    modalState = ModalState::Confirm;
    confirmAction = ConfirmAction::DeleteQuarterNote;

    showConfirm("DELETE?");
    break;
```

---

Et tu implémentes seulement :

```cpp
void modalAction(const InputEvent& event, FnMask fnMask) override
{
    if (event.type != InputEventType::ButtonReleased)
        return;

    if (fnMask == FN0) // YES
    {
        if (confirmAction == ConfirmAction::DeleteQuarterNote)
        {
            sequencerTimer.removeQuarterNote();
        }

        modalState = ModalState::None;
        confirmAction = ConfirmAction::None;
    }


    if (fnMask == FN1) // NO
    {
        modalState = ModalState::None;
        confirmAction = ConfirmAction::None;
    }
}
```

---

L'intérêt par rapport au simple `if` :

* pas besoin de polluer `fnReleaseAction()`
* impossible de modifier un step pendant une confirmation
* tu pourras réutiliser pour :

  * `CLEAR TRACK?`
  * `LOAD SONG?`
  * `SAVE?`
  * `RESET?`

Ton architecture est déjà proche d'un tracker, donc les modales séparées sont une bonne évolution.
