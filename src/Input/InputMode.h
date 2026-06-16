#pragma once

#include "InputEvent.h"
class Input;


enum class InputModes
{
    None,
    Main,
    Step
};

using FnMask = uint8_t;

typedef struct 
{
    FnMask pressedMask = 0;

    bool holdTriggered = false;
    bool modifierUsed = false;

    FnMask pressCombo = 0;
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
           c <= ControlId::Step3;
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
    public:
        explicit InputMode(Input& _input)
            : input(_input)
        {
        }

        virtual void onEnter() {}

        virtual void onExit() {}

        virtual void handleEvent(const InputEvent& event)
        {
            switch(event.type) {
                case InputEventType::ButtonPressed:
                {
                    // We save pressed Fn key in bitmask
                    // If step key, if bitmask Fn set we take it
                    if (isFn(event.control)) {
                        fnState.pressedMask |= fnBit(event.control);

                        fnState.pressCombo = fnState.pressedMask;
                    } else if (isStep(event.control)) {
                        stepState.pressed = true;
                        stepState.stepId = event.control;

                        // Snapshot de la combinaison
                        stepState.fnCombo = fnState.pressedMask;

                        if (stepState.fnCombo != 0) {
                            fnState.modifierUsed = true;
                        }
                    }

                    break;
                }

                case InputEventType::ButtonHold:
                {
                    // Just check if Fn key is hold
                    // We get bitmask to know if many pressed
                    if (isFn(event.control)) {
                        fnState.holdTriggered = true;

                        // Snapshot de la combinaison
                        fnState.holdCombo = fnState.pressedMask;

                        if (!fnState.modifierUsed) {
                            fnHoldAction(event, fnState.holdCombo);
                        }
                    } else if(isStep(event.control)) {
                        stepState.holdTriggered = true;

                        stepHoldAction(event, stepState.fnCombo);
                    }

                    break;
                }

                case InputEventType::ButtonReleased:
                {
                    // If a step released and was the pressed step
                    // We have fnCombo so we can do something different 
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

                        stepState.pressed = false;
                        stepState.holdTriggered = false;
                        stepState.fnCombo = 0;
                        stepState.stepUsed = false;
                    } else if (isFn(event.control)) {
                        fnState.pressedMask &= ~fnBit(event.control);

                        // Dernier Fn relâché
                        // If no bit mask, fn key hold and we didnt press step key we execute hold action of fn key
                        if (fnState.pressedMask == 0) {
                            if (fnState.holdTriggered && !fnState.modifierUsed) {
                                fnHoldReleaseAction(
                                    event,
                                    fnState.holdCombo);
                            } else if (!fnState.modifierUsed) {
                                fnReleaseAction(
                                    event,
                                    fnState.pressCombo);
                            }

                            fnState.holdTriggered = false;
                            fnState.modifierUsed = false;
                            fnState.pressCombo = 0;
                            fnState.holdCombo = 0;
                        }
                    }

                    break;
                }

                case InputEventType::EncoderTurned:
                {
                    // No encoder nothing to do
                    if (!isEncoder(event.control)) {
                        break;
                    }

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
        virtual void stepReleaseAction(const InputEvent& event, FnMask fnMask) {}

        virtual void fnReleaseAction(const InputEvent& event, FnMask fnMask) {}

        virtual void fnHoldAction(const InputEvent& event, FnMask fnMask) {}

        virtual void encoderAction(const InputEvent& event, FnMask fnMask) {}

        virtual void stepHoldAction(const InputEvent& event, FnMask fnMask) {}

        virtual void fnHoldReleaseAction(const InputEvent& event, FnMask fnMask) {}

        virtual void stepHoldReleaseAction(const InputEvent& event, FnMask fnMask) {}

    protected:
        Input& input;
        TaskHandle_t displayTaskHandle = nullptr;

        FnState fnState;
        StepState stepState;
};

/*
Oui, dans ton cas (groovebox/séquenceur avec beaucoup de combinaisons), le bitmask est finalement la solution la plus propre.

L'idée :

* plusieurs Fn peuvent être maintenus simultanément ;
* un Hold mémorise la combinaison complète ;
* un Step mémorise la combinaison complète au moment de son appui ;
* un encodeur peut utiliser la combinaison Fn courante ;
* une action Hold n'est déclenchée que si aucun Step/Encodeur n'a consommé la combinaison.

---

### État

```cpp
using FnMask = uint8_t; // 8 Fn max

struct FnState
{
    FnMask pressedMask = 0;

    bool holdTriggered = false;
    bool modifierUsed = false;

    FnMask holdCombo = 0;
};

struct StepState
{
    bool pressed = false;

    ControlId stepId;

    FnMask fnCombo = 0;
};

FnState m_fn;
StepState m_step;
```

---

### Helpers

```cpp
constexpr bool isFn(ControlId c)
{
    return c >= ControlId::Fn0 &&
           c <= ControlId::Fn7;
}

constexpr bool isStep(ControlId c)
{
    return c >= ControlId::Step0 &&
           c <= ControlId::Step3;
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

bool anyFnPressed()
{
    return m_fn.pressedMask != 0;
}
```

---

### Définition des combinaisons

```cpp
constexpr FnMask FN0 = 1u << 0;
constexpr FnMask FN1 = 1u << 1;
constexpr FnMask FN2 = 1u << 2;
constexpr FnMask FN3 = 1u << 3;
constexpr FnMask FN4 = 1u << 4;
constexpr FnMask FN5 = 1u << 5;
constexpr FnMask FN6 = 1u << 6;
constexpr FnMask FN7 = 1u << 7;
```

---

### Hold actions

```cpp
void executeHoldAction(FnMask combo)
{
    switch(combo)
    {
        case FN0:
            holdPatternMenu();
            break;

        case FN1:
            holdTrackMenu();
            break;

        case (FN0 | FN1):
            holdBankMenu();
            break;

        case (FN0 | FN2):
            holdProjectMenu();
            break;

        default:
            break;
    }
}
```

---

### Step actions

```cpp
void executeStepAction(ControlId step,
                       FnMask combo)
{
    switch(combo)
    {
        case 0:
            selectStep(step);
            break;

        case FN0:
            selectPattern(step);
            break;

        case FN1:
            selectTrack(step);
            break;

        case (FN0 | FN1):
            selectBank(step);
            break;

        case (FN0 | FN2):
            selectProject(step);
            break;

        default:
            break;
    }
}
```

---

### Encodeur

```cpp
void executeEncoderAction(ControlId encoder,
                          int delta,
                          FnMask combo)
{
    switch(combo)
    {
        case 0:
            encoderNormal(encoder, delta);
            break;

        case FN0:
            encoderPattern(encoder, delta);
            break;

        case FN1:
            encoderTrack(encoder, delta);
            break;

        case (FN0 | FN1):
            encoderBank(encoder, delta);
            break;

        default:
            break;
    }
}
```

---

### Event Handler

```cpp
void handleEvent(InputEventType type,
                 ControlId control,
                 int delta = 0)
{
    switch(type)
    {
        //--------------------------------------
        // BUTTON PRESS
        //--------------------------------------

        case InputEventType::ButtonPressed:
        {
            if (isFn(control))
            {
                m_fn.pressedMask |= fnBit(control);
            }
            else if (isStep(control))
            {
                m_step.pressed = true;
                m_step.stepId = control;

                // Snapshot de la combinaison
                m_step.fnCombo = m_fn.pressedMask;

                if (m_step.fnCombo != 0)
                {
                    m_fn.modifierUsed = true;
                }
            }

            break;
        }

        //--------------------------------------
        // BUTTON HOLD
        //--------------------------------------

        case InputEventType::ButtonHold:
        {
            if (isFn(control))
            {
                m_fn.holdTriggered = true;

                // Snapshot de la combinaison
                m_fn.holdCombo = m_fn.pressedMask;
            }

            break;
        }

        //--------------------------------------
        // BUTTON RELEASE
        //--------------------------------------

        case InputEventType::ButtonRelease:
        {
            //----------------------------------
            // STEP
            //----------------------------------

            if (isStep(control) &&
                m_step.pressed &&
                control == m_step.stepId)
            {
                executeStepAction(
                    control,
                    m_step.fnCombo);

                m_step.pressed = false;
                m_step.fnCombo = 0;
            }

            //----------------------------------
            // FN
            //----------------------------------

            else if (isFn(control))
            {
                m_fn.pressedMask &= ~fnBit(control);

                // Dernier Fn relâché
                if (m_fn.pressedMask == 0)
                {
                    if (m_fn.holdTriggered &&
                        !m_fn.modifierUsed)
                    {
                        executeHoldAction(
                            m_fn.holdCombo);
                    }

                    m_fn.holdTriggered = false;
                    m_fn.modifierUsed = false;
                    m_fn.holdCombo = 0;
                }
            }

            break;
        }

        //--------------------------------------
        // ENCODER
        //--------------------------------------

        case InputEventType::EncoderTurned:
        {
            if (!isEncoder(control))
                break;

            executeEncoderAction(
                control,
                delta,
                m_fn.pressedMask);

            if (m_fn.pressedMask != 0)
            {
                m_fn.modifierUsed = true;
            }

            break;
        }

        default:
            break;
    }
}
```

### Exemples

#### Pattern

```text
Fn0 down
Step2 down
Step2 up
Fn0 up
```

→ `selectPattern(Step2)`

---

#### Bank

```text
Fn0 down
Fn1 down
Step3 down
Step3 up
Fn1 up
Fn0 up
```

→ `selectBank(Step3)`

---

#### Hold simple

```text
Fn0 down
(1 seconde)
Fn0 up
```

→ `holdPatternMenu()`

---

#### Hold combo

```text
Fn0 down
Fn1 down
(1 seconde)
Fn1 up
Fn0 up
```

→ `holdBankMenu()`

---

#### Hold annulé par un Step

```text
Fn0 down
(1 seconde)
Step1 down
Step1 up
Fn0 up
```

→ action Step uniquement, pas d'action Hold. C'est le rôle de `modifierUsed`.
*/
