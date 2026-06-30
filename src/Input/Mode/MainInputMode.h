#pragma once

#include "SequencerTimer.h"
#include "Input/Input.h"
#include "Input/InputMode.h"
#include "Display/Display.h"

enum class ConfirmAction
{
    None,
    DeleteQuarterNote,
    ClearScreen
};

class MainInputMode : public InputMode
{
    public:
        ConfirmAction confirmAction = ConfirmAction::None;

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
        }

        void stepReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case 0:
                {
                    /** @implements toggle step state */
                    uint8_t row = (static_cast<uint8_t>(event.control)) / 4;
                    uint8_t col = (static_cast<uint8_t>(event.control)) % 4;
                    sequencerTimer.toggleStep(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col);

                    break;
                }
                case FN1:
                {
                    /** @implements change quarter note length */
                    uint8_t row = (static_cast<uint8_t>(event.control)) / 4;
                    uint8_t col = (static_cast<uint8_t>(event.control)) % 4;
                    sequencerTimer.setQuarterNoteStepsCount(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col + 1);

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
                case FN1:
                {
                    /** 
                     * @implements set encoder 0 value and range to choose track
                     */
                    uint8_t Tcount = max<uint8_t>(1, sequencerTimer.trackCounts);
                    if (0 < Tcount-1) {
                        input.setEncoderBoundaries(ControlId::Encoder0, 0, Tcount-1, true);
                        input.setEncoderValue(ControlId::Encoder0, sequencerTimer.selectedTrack);
                    }

                    /**
                     * @implements set encoder 1 value and range to choose step
                     */
                    QuarterNote& quarterNote = sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote];
                    uint8_t stepsCount = max<uint8_t>(1, quarterNote.stepsCount);
                    if (0 < stepsCount-1) {
                        input.setEncoderBoundaries(ControlId::Encoder1, -1, stepsCount, true);
                        input.setEncoderValue(ControlId::Encoder1, sequencerTimer.selectedStep);
                    }

                    break;
                }
                case FN2:
                {
                    Track& track = sequencerTimer.tracks[sequencerTimer.selectedTrack];
                    /** @implements  set encoder 0 value and range for track volume */
                    input.setEncoderBoundaries(ControlId::Encoder0, 0, 255, false);
                    input.setEncoderValue(ControlId::Encoder0, track.volume);

                    /** @implements  set encoder 1 value and range for track transpose */
                    input.setEncoderBoundaries(ControlId::Encoder1, -12, 12, false);
                    input.setEncoderValue(ControlId::Encoder1, sequencerTimer.track.transpose);
                    break;
                }

                case FN3:
                {
                    Track& track = sequencerTimer.tracks[sequencerTimer.selectedTrack];
                    /** @implements  set encoder 0 value and range to choose instrument on track*/
                    input.setEncoderBoundaries(ControlId::Encoder0, 0, 11, false);
                    input.setEncoderValue(ControlId::Encoder0, track.instrument);

                    /** @implements  set encoder 0 value and range to choose instrument global*/
                    input.setEncoderBoundaries(ControlId::Encoder1, 0, 11, false);
                    input.setEncoderValue(ControlId::Encoder1, sequencerTimer.selectedInstrument);
                    break;
                }
            
                default:
                    break;
            }
        }

        void fnReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN0:
                {
                    /** @implements toggle play pause */
                    sequencerTimer.togglePause();

                    break;
                }
                case FN1:
                {
                    /** @implements add quarter note */
                    sequencerTimer.addQuarterNote();

                    break;
                }
                case FN3:
                {
                    /** @todo Go to instrument edition screen */
                    break;
                }             
                default:
                    break;
            }
        }

        void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN0:
                {
                    /** @implements toggle play stop */
                    sequencerTimer.toggleStop();

                    break;
                }
                case FN1:
                {
                    /** @implements remove last quarter note confirm */
                    confirmAction = ConfirmAction::DeleteQuarterNote;
                    modalState = ModalState::Confirm;

                    display.showConfirm("Delete quarter note ?");
                    break;
                }                
                case FN2:
                {
                    /** @todo Mute selected track */
                    break;
                }
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
                    case FN2:
                    {
                        /** @implements change step length */
                        if (encoder == ControlId::Encoder0) {
                            sequencerTimer.setStepLength(display.displayedTrack + row, sequencerTimer.selectedQuarterNote, col, event.value);
                        }

                        break;
                    }
                    case FN3:
                    {
                        /** @implements change step instrument */
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
                        /** @implements  set sequencer volume */
                        sequencerTimer.setVolume(event.value);
                    }

                    if (encoder == ControlId::Encoder1) {
                        /** @implements  set sequencer bpm */
                        sequencerTimer.setBpm(event.value);
                    }

                    break;

                case FN1:
                {
                    /** @implements  choose track */
                    if (encoder == ControlId::Encoder0) {
                        sequencerTimer.setSelectedTrack(event.value);

                        if (event.value >= (display.displayedTrack+2)) {
                            display.displayedTrack = event.value-1;
                        }

                        if (event.value <= display.displayedTrack) {
                            display.displayedTrack = event.value;
                        }

                        if (sequencerTimer.selectedStep >= sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount) {
                            sequencerTimer.setSelectedStep(sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount-1);
                        }
                    }

                    /** @implements  choose quarter note */
                    if (encoder == ControlId::Encoder1) {
                        if (event.value < 0) {
                            sequencerTimer.setSelectedQuarterNote(sequencerTimer.selectedQuarterNote-1);
                            sequencerTimer.setSelectedStep(sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount-1);

                            break;
                        }
                        if (event.value >= sequencerTimer.tracks[sequencerTimer.selectedTrack].quarterNotes[sequencerTimer.selectedQuarterNote].stepsCount) {
                            sequencerTimer.setSelectedQuarterNote(sequencerTimer.selectedQuarterNote+1);
                            sequencerTimer.setSelectedStep(0);

                            break;
                        }

                        sequencerTimer.setSelectedStep(event.value);

                        break;
                    }
                }

                case FN2:
                    /** @implements  set track volume */
                    if (encoder == ControlId::Encoder0) {
                        sequencerTimer.setTrackVolume(sequencerTimer.selectedTrack, event.value);
                    }
                    /** @implements  set track transpose */
                    if (encoder == ControlId::Encoder1) {
                        sequencerTimer.setTrackTranspose(sequencerTimer.selectedTrack, event.value);
                    }
                    break;

                case FN3:
                    /** @implements  choose instrument on track*/
                    if (encoder == ControlId::Encoder0) {
                        sequencerTimer.setTrackInstrument(sequencerTimer.selectedTrack, event.value);
                    }

                    /** @implements  choose instrument global*/
                    if (encoder == ControlId::Encoder1) {
                        sequencerTimer.setSelectedInstrument(event.value);
                    }

                    break;
            
            default:
                break;
            }
        }

        void fnHoldAction(const InputEvent& event, FnMask fnMask) override
        {
            switch (fnMask)
            {
                case FN4:
                {
                    /** @implements clear entire screen confirm */
                    confirmAction = ConfirmAction::DeleteQuarterNote;
                    modalState = ModalState::Confirm;

                    display.showConfirm("Clear screen ?");
                    break;
                }
                
                default:
                    break;
            }
        }


        void modalAction(const InputEvent& event) override
        {
            if (event.type != InputEventType::ButtonReleased)
                return;

            if (event.control == ControlId::Fn0) // YES
            {
                switch (confirmAction)
                {
                    case ConfirmAction::DeleteQuarterNote:
                        sequencerTimer.removeQuarterNote();
                        break;
                    
                    case ConfirmAction::ClearScreen:
                        /**
                         * @todo clear screen
                         */
                        // sequencerTimer.cleanScreen(display.displayedTrack);
                        break;
                    
                    default:
                        break;
                }

                modalState = ModalState::None;
                confirmAction = ConfirmAction::None;

                display.hideConfirm();
                display.updateDisplay();

                return;
            }


            if (event.control == ControlId::Fn1) // NO
            {
                modalState = ModalState::None;
                confirmAction = ConfirmAction::None;
                display.hideConfirm();
                display.updateDisplay();
            }
        }
        

    private:
        SequencerTimer& sequencerTimer;
        Display& display;
        InputModes pendingMode = InputModes::None;
};



/*

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
    /** @todo remove last quarter note *
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
*/
