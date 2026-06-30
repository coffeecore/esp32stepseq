#pragma once

#include "InputEvent.h"
class Input;


enum class InputModes
{
    None,
    Main,
    Step,
    QuarterNote
};

using FnMask = uint8_t;

enum class ModalState
{
    None,
    Confirm
};

typedef struct 
{
    FnMask pressedMask = 0; // Fn encore enfoncés maintenant

    bool holdTriggered = false;
    bool modifierUsed = false;
    bool holdConsumed = false;

    FnMask pressCombo = 0; // tous les Fn ayant participé à la séquence jusqu'au relâchement complet
    FnMask holdCombo = 0;
} FnState;

typedef struct 
{
    bool pressed = false;
    bool holdTriggered = false;
    bool stepUsed = false;

    ControlId stepId;

    FnMask fnCombo = 0;
} StepState;

constexpr bool isFn(ControlId c)
{
    return c >= ControlId::Fn0 &&
           c <= ControlId::Fn7;
}

constexpr bool isStep(ControlId c)
{
    return c >= ControlId::Step0 &&
           c <= ControlId::Step15;
}

constexpr bool isEncoder(ControlId c)
{
    return c == ControlId::Encoder0 ||
           c == ControlId::Encoder1;
}

constexpr uint8_t fnIndex(ControlId c)
{
    return static_cast<uint8_t>(c)
         - static_cast<uint8_t>(ControlId::Fn0);
}

constexpr FnMask fnBit(ControlId c)
{
    return static_cast<FnMask>(1u << fnIndex(c));
}

constexpr FnMask FN0 = 1u << 0;
constexpr FnMask FN1 = 1u << 1;
constexpr FnMask FN2 = 1u << 2;
constexpr FnMask FN3 = 1u << 3;
constexpr FnMask FN4 = 1u << 4;
constexpr FnMask FN5 = 1u << 5;
constexpr FnMask FN6 = 1u << 6;
constexpr FnMask FN7 = 1u << 7;

class InputMode
{
    protected:
        Input& input;
        TaskHandle_t displayTaskHandle = nullptr;

        FnState fnState;
        StepState stepState;
        ModalState modalState = ModalState::None;

    public:
        explicit InputMode(Input& _input)
            : input(_input)
        {
        }

        virtual void onEnter() {}

        virtual void onExit() {}

        virtual void handleEvent(const InputEvent& event)
        {
            updateStates(event);      // uniquement l'état physique

            if (modalState != ModalState::None)
            {
                modalAction(event);   // peut ouvrir/fermer une modale
            }
            else
            {
                dispatchActions(event); // callbacks utilisateur
            }

            cleanupStates(event);     // reset des flags temporaires
        }

        virtual void updateStates(const InputEvent& event)
        {
            switch(event.type) {
                case InputEventType::ButtonPressed:
                {
                    // We save pressed Fn key in bitmask
                    // If step key, if bitmask Fn set we take it
                    if (isFn(event.control)) {
                        fnState.pressedMask |= fnBit(event.control);
                        fnState.pressCombo |= fnBit(event.control);
                    } else if (isStep(event.control)) {
                        stepState.pressed = true;
                        stepState.stepId = event.control;

                        // Snapshot de la combinaison
                        stepState.fnCombo = fnState.pressedMask;
                    }

                    break;
                }

                case InputEventType::ButtonHold:
                {
                    // Just check if Fn key is hold
                    // We get bitmask to know if many pressed
                    if (isFn(event.control)) {
                        if (fnState.modifierUsed) {
                            break;
                        }
                        fnState.holdTriggered = true;

                        // Snapshot de la combinaison
                        fnState.holdCombo = fnState.pressedMask;

                        // Le hold a effectué son action,
                        // il ne faudra rien faire au release.
                    } else if(isStep(event.control)) {
                        stepState.holdTriggered = true;
                    }

                    break;
                }

                case InputEventType::ButtonReleased:
                {
                    if (isFn(event.control)) {
                        fnState.pressedMask &= ~fnBit(event.control);
                    }

                    break;
                }

                default:
                    break;
            }
        }


        virtual void dispatchActions(const InputEvent& event)
        {
            switch(event.type) {
                case InputEventType::ButtonPressed:
                {
                    // We save pressed Fn key in bitmask
                    // If step key, if bitmask Fn set we take it
                    if (isFn(event.control)) {
                        fnPressedAction(event, fnState.pressedMask);

                    } else if (isStep(event.control)) {
                        stepPressedAction(event, stepState.fnCombo);

                        if (stepState.fnCombo != 0) {
                            fnState.modifierUsed = true;
                            stepState.stepUsed = true;
                        }
                    }

                    break;
                }

                case InputEventType::ButtonHold:
                {
                    // Just check if Fn key is hold
                    // We get bitmask to know if many pressed
                    if (isFn(event.control)) {
                        if (!fnState.modifierUsed) {
                        // Snapshot de la combinaison
                        fnHoldAction(event, fnState.holdCombo);

                        // Le hold a effectué son action,
                        // il ne faudra rien faire au release.
                        fnState.holdConsumed = true;
                        }
                    } else if(isStep(event.control)) {
                        stepHoldAction(event, stepState.fnCombo);
                    }

                    break;
                }

                case InputEventType::ButtonReleased:
                {
                    if (isStep(event.control) &&
                        stepState.pressed &&
                        event.control == stepState.stepId
                    )
                    {
                        if (!stepState.stepUsed) {
                            if (stepState.holdTriggered) {
                                stepHoldReleaseAction(event, stepState.fnCombo);
                            } else {
                                stepReleaseAction(event, stepState.fnCombo);
                            }
                        }
                    } else if (isFn(event.control)) {
                        if (fnState.pressedMask == 0) {
                            if (!fnState.modifierUsed)
                            {
                                if (fnState.holdTriggered)
                                {
                                    if (!fnState.holdConsumed)
                                    {
                                        fnHoldReleaseAction(event, fnState.holdCombo);
                                    }
                                }
                                else
                                {
                                    fnReleaseAction(event, fnState.pressCombo);
                                }
                            }

                            fnReleasedAlways(event, fnState.pressCombo);
                        }
                    }

                    break;
                }

                case InputEventType::EncoderTurned:
                {
                    // We have bitmask because we pressed fn key before, we can do something based on or not
                    encoderAction(event, fnState.pressedMask);

                    // We use rotary, we mustnot execute fn hold action
                    if (fnState.pressedMask != 0) {
                        fnState.modifierUsed = true;
                    }

                    if (stepState.pressed) {
                        stepState.stepUsed = true;
                    }

                    break;
                }


                default:
                    break;
            }
        }

        void cleanupStates(const InputEvent& event)
        {
            if (event.type != InputEventType::ButtonReleased)
                return;

            if (isStep(event.control) &&
                stepState.pressed &&
                event.control == stepState.stepId)
            {
                stepState.pressed = false;
                stepState.stepUsed = false;
                stepState.holdTriggered = false;
                stepState.fnCombo = 0;
            }
            else if (isFn(event.control))
            {
                if (fnState.pressedMask == 0)
                {
                    fnState.modifierUsed = false;
                    fnState.holdTriggered = false;
                    fnState.holdConsumed = false;
                    fnState.pressCombo = 0;
                    fnState.holdCombo = 0;
                }
            }
        }

        void attachDisplayTask(TaskHandle_t handle)
        {
            displayTaskHandle = handle;
        }

        void notifyDisplay()
        {
            if (displayTaskHandle != nullptr) {
                xTaskNotifyGive(displayTaskHandle);
            }
        }
    
    protected:
        virtual void stepReleasedAlways(const InputEvent& event, FnMask fnMask)
        {
        }
        virtual void fnReleasedAlways(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (releasedFn == ControlId::Fn0) {
            //     // Le dernier Fn relâché est Fn0
            // }

            // if (fnMask == FN0) {
            //     // La combinaison utilisée était uniquement Fn0
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // La combinaison utilisée était Fn0 + Fn2
            // }
        }

        virtual void stepReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedStep = event.control;

            // if (releasedStep == ControlId::Step3) {
            //     // Relâchement du Step 3
            // }

            // if (fnMask == 0) {
            //     // Aucun Fn utilisé
            // }

            // if (fnMask == FN1) {
            //     // Step utilisé avec Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Step utilisé avec Fn0 + Fn2
            // }
        }

        virtual void fnReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (fnMask == (FN0 | FN2)) {
            //     // La combinaison utilisée était Fn0 + Fn2
            // }

            // if (releasedFn == ControlId::Fn2) {
            //     // Le dernier Fn relâché est Fn2
            // }
        }

        virtual void fnHoldAction(const InputEvent& event, FnMask fnMask)
        {
            // auto heldFn = event.control;

            // if (fnMask == FN1) {
            //     // Hold sur Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Hold alors que Fn0 + Fn2 sont maintenus
            // }
        }

        virtual void encoderAction(const InputEvent& event, FnMask fnMask)
        {
            // auto encoder = event.control;

            // if (encoder == ControlId::Encoder0) {
            //     // Encoder 0 tourné
            // }

            // if (encoder == ControlId::Encoder1) {
            //     // Encoder 1 tourné
            // }

            // if (fnMask == 0) {
            //     // Encoder seul
            // }

            // if (fnMask == FN1) {
            //     // Encoder avec Fn1 maintenu
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Encoder avec Fn0 + Fn2 maintenus
            // }

            // if (stepState.pressed) {
            //     // un step est maintenu
            // }

            // if (stepState.stepId == ControlId::Step3) {
            //     // c'est Step3
            // }
        }

        virtual void stepHoldAction(const InputEvent& event, FnMask fnMask)
        {
            // auto heldStep = event.control;

            // if (heldStep == ControlId::Step8) {
            //     // Hold sur Step8
            // }

            // if (fnMask == FN3) {
            //     // Hold sur Step8 avec Fn3
            // }
        }

        virtual void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedFn = event.control;

            // if (fnMask == FN1) {
            //     // Fin du hold de Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Fin du hold de la combinaison Fn0 + Fn2
            // }
        }

        virtual void stepHoldReleaseAction(const InputEvent& event, FnMask fnMask)
        {
            // auto releasedStep = event.control;

            // if (releasedStep == ControlId::Step8) {
            //     // Fin du hold sur Step8
            // }

            // if (fnMask == FN3) {
            //     // Fin du hold Step8 + Fn3
            // }
        }

        virtual void fnPressedAction(const InputEvent& event, FnMask fnMask)
        {
            // auto pressedFn = event.control;

            // if (fnMask == (FN0 | FN2)) {
            //     // Fn0 + Fn2 actuellement maintenus
            // }

            // if (pressedFn == ControlId::Fn2) {
            //     // c'est Fn2 qui vient d'être appuyé
            // }
        }

        virtual void stepPressedAction(const InputEvent& event, FnMask fnMask)
        {
            // auto pressedStep = event.control;

            // if (pressedStep == ControlId::Step3) {
            //     // C'est Step3 qui vient d'être pressé
            // }

            // if (fnMask == 0) {
            //     // Step pressé sans Fn
            // }

            // if (fnMask == FN1) {
            //     // Step pressé avec Fn1
            // }

            // if (fnMask == (FN0 | FN2)) {
            //     // Step pressé avec Fn0 + Fn2
            // }
        }

        virtual void modalAction(const InputEvent& event)
        {
        }
};


/*Oui. Avec les contraintes que tu as aujourd'hui, je ne mettrais **pas** tout dans `updateStates()`. Je garderais les trois phases, mais avec une vraie séparation des responsabilités.

Je partirais sur ça :

```cpp
handleEvent(event)
{
    updateStates(event);      // uniquement l'état physique

    if (modalState != ModalState::None)
    {
        modalAction(event);   // peut ouvrir/fermer une modale
    }
    else
    {
        dispatchActions(event); // callbacks utilisateur
    }

    cleanupStates(event);     // reset des flags temporaires
}
```

---

# updateStates()

Ne fait **que** l'état matériel.

Exemple :

```cpp
ButtonPressed Fn
    pressedMask |= bit
    pressCombo  |= bit

ButtonPressed Step
    stepState.pressed = true;
    stepState.stepId = event.control;
    stepState.fnCombo = fnState.pressedMask;

ButtonHold Fn
    holdTriggered = true;
    holdCombo = pressedMask;

ButtonHold Step
    stepState.holdTriggered = true;

ButtonReleased Fn
    pressedMask &= ~bit;

ButtonReleased Step
    // rien
```

Rien d'autre.

Pas de reset.

Pas de callback.

---

# dispatchActions()

Là tu reprends quasiment ton ancien code.

Par contre tu remets les protections.

## ButtonPressed

```cpp
Fn
    fnPressedAction()

Step
    stepPressedAction()

    if(stepState.fnCombo)
        fnState.modifierUsed = true;
```

---

## Encoder

```cpp
encoderAction()

if(fnState.pressedMask)
    fnState.modifierUsed = true;

if(stepState.pressed)
    stepState.stepUsed = true;
```

Exactement comme avant.

---

## Hold

```cpp
Fn

if(fnState.modifierUsed)
    return;

fnHoldAction()

fnState.holdConsumed = true;
```

Donc :

```
Fn4
hold
```

→ OK

mais

```
Fn4
tourne encodeur
hold
```

→ pas de hold.

Comme avant.

---

Step :

```cpp
stepHoldAction(...)
```

---

## Release Step

Tu retrouves ton ancien comportement :

```cpp
if(!stepState.stepUsed)
{
    if(stepState.holdTriggered)
        stepHoldReleaseAction(...);
    else
        stepReleaseAction(...);
}

stepReleasedAlways(...);
```

Donc

```
Step
```

=> toggle

---

```
Step
Encoder
Release
```

=> PAS de toggle.

---

```
Fn
Step
Release
```

=> PAS de toggle.

Parce que `modifierUsed` aura été positionné au press du Step avec Fn.

---

## Release Fn

Pareil :

```cpp
if(fnState.pressedMask==0)
{
    if(!fnState.modifierUsed)
    {
        if(fnState.holdTriggered)
        {
            if(!fnState.holdConsumed)
                fnHoldReleaseAction(...);
        }
        else
        {
            fnReleaseAction(...);
        }
    }

    fnReleasedAlways(...);
}
```

---

# cleanupStates()

Seulement ici :

Release Step

```cpp
stepState.pressed=false;
stepState.stepUsed=false;
stepState.holdTriggered=false;
stepState.fnCombo=0;
```

---

Release dernier Fn

```cpp
if(fnState.pressedMask==0)
{
    fnState.modifierUsed=false;
    fnState.holdTriggered=false;
    fnState.holdConsumed=false;
    fnState.pressCombo=0;
    fnState.holdCombo=0;
}
```

```cpp
void cleanupStates(const InputEvent& event)
{
    if (event.type != InputEventType::ButtonReleased)
        return;

    if (isStep(event.control) &&
        stepState.pressed &&
        event.control == stepState.stepId)
    {
        stepState.pressed = false;
        stepState.stepUsed = false;
        stepState.holdTriggered = false;
        stepState.fnCombo = 0;
    }
    else if (isFn(event.control))
    {
        if (fnState.pressedMask == 0)
        {
            fnState.modifierUsed = false;
            fnState.holdTriggered = false;
            fnState.holdConsumed = false;
            fnState.pressCombo = 0;
            fnState.holdCombo = 0;
        }
    }
}
```

---

# Et la modale ?

C'est là que cette architecture devient intéressante.

Pendant le hold :

```
Fn4
Fn7
```

↓

```
ButtonHold
```

↓

```
dispatchActions()
```

↓

```
fnHoldAction()
```

↓

```cpp
modalState = Confirm;
display.showConfirm(...);
```

La boîte apparaît **immédiatement** pendant le maintien.

Ensuite :

```
Fn7 release
Fn4 release
```

Aucun callback Fn ne sera exécuté puisque `modalState != None`.

Mais **updateStates() continue de tourner**, donc `pressedMask` redevient bien à 0.

Ensuite :

```
Fn0
```

↓

`modalAction()`

↓

YES

ou

```
Fn1
```

↓

NO

---

# Ce que ça garantit

Tu conserves tous les comportements que tu voulais :

* ✅ `Fn0 + Fn1` puis hold → la modale apparaît pendant le maintien.
* ✅ `Step` seul → toggle au relâchement.
* ✅ `Fn + Step` → pas de toggle.
* ✅ `Step + Encoder` → pas de toggle.
* ✅ `Fn + Encoder` → l'action de l'encodeur uniquement, pas de `fnReleaseAction()`.
* ✅ `Fn + Step + Encoder` → édition du step uniquement, pas de toggle au relâchement.
* ✅ Les modales n'empêchent jamais la mise à jour des états internes, seulement les callbacks métier.

À mes yeux, c'est la version la plus propre de ton architecture actuelle, sans tout réécrire ni introduire de nouveaux états ou d'énumérations supplémentaires.
*/
