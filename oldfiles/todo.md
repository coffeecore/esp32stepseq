# Todo

- [x] Ticks system : based on hardware timer (`hw_timer_t` or `esp_timer`)
- [ ] Sequencer
    - [ ] Set BPM
    - [ ] Tracks > Patterns > Steps
    - [ ] Step on and step off based on ticks
    - [ ] Step on length based on number of ticks
    - [ ] Catch up if drift
    - [ ] Tracks
        - [ ] Add a track
        - [ ] Delete a track
        - [ ] Volume
        - [ ] Transpose
        - [ ] Sample
    - [ ] Patterns
        - [ ] Add pattern
        - [ ] Delete pattern
        - [ ] Set number of quarter notes
        - [ ] Set number of steps by quarter notes
    - [ ] Step
        - [ ] Set state
        - [ ] Set note
        - [ ] Set length
- [ ] Display
    - [ ] Boot
    - [ ] Sequencer informations
        - [ ] Display volume
        - [ ] Display BPM
        - [ ] Display current track
        - [ ] Display current pattern
    - [ ] Track
        - [ ] Display volume
        - [ ] Display transpose
        - [ ] Display sample
    - [ ] Step
        - [ ] Display state
        - [ ] Display note
        - [ ] Display length
- [ ] Input
    - [ ] Sequencer
        - [ ] Set volume
        - [ ] Set BPM
        - [ ] Select current track
        - [ ] Select current pattern
    - [ ] Track
        - [ ] Set volume
        - [ ] Set transpose

# Pins

| Fonction | GPIO |
| --- | --- |
| I2C SDA | 21 |
| I2C SCL | 22 |
| | |
| ENC1 CLK | 34 |
| ENC1 DT | 35 |
| ENC1 SW | 32 |
| | |
| ENC2 CLK | 16 |
| ENC2 DT | 17 |
| ENC2 SW | 33 |
| | |
| SD CS | 5 |
| SD MOSI | 23 |
| SD MISO | 19 |
| SD SCK | 18 |
| | |
| I2S BCK | 26 |
| I2S LCK | 25 |
| I2S DIN | 27 |
| | |
| BTN1 | GPA0 |
| BTN2 | GPA1 |
| BTN3 | GPA2 |
| BTN4 | GPA3 |
| BTN5 | GPA4 |
| BTN6 | GPA5 |

⚠️ Détails importants (souvent oubliés)
 - MCP23017
activer pull-up via code (pas physique)
possibilité d’utiliser INT → GPIO 4 (optionnel)
 - KY-040
prévoir debounce logiciel
 - PCM5102
alimentation propre (important pour audio)
GND commun obligatoire



```
docker run --rm -u 1000:1000 -v ${PWD}:/src wokwi/builder-clang-wasm:latest make
```


https://wokwi.com/projects/461268764932519937



Bouton	GPIO
BTN0	13
BTN1	12
BTN2	14
BTN3	27
BTN4	26
BTN5	25
BTN6	33
BTN7	32

Avec INPUT_PULLUP.



~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line -pfiaC -e .pio/build/esp32dev/firmware.elf 0x400d09b8


















# Rewrite inputmode

Celle que je préfère (et de loin dans ton cas), c’est la séparation explicite en **3 étapes dans `handleEvent()`** :

👉 **1. mise à jour des états (toujours exécutée)**
👉 **2. modale (prioritaire, intercepte les actions)**
👉 **3. dispatch vers callbacks métier**

Ça donne ça :

```cpp
virtual void handleEvent(const InputEvent& event)
{
    // 1) Toujours exécuté : état pur (source de vérité)
    updateStates(event);

    // 2) Modale : bloque le reste, mais n'empêche pas les états
    if (modalState != ModalState::None)
    {
        modalAction(event);
        return;
    }

    // 3) Logique métier normale
    dispatchActions(event);
}
```

---

## 🔧 1. `updateStates(event)` (CRUCIAL)

Tu déplaces ici **UNIQUEMENT la mécanique interne** :

* `fnState.pressedMask`
* `fnState.holdTriggered`
* `stepState`
* etc.

Ex :

```cpp
void updateStates(const InputEvent& event)
{
    switch(event.type)
    {
        case InputEventType::ButtonPressed:
            if (isFn(event.control)) {
                fnState.pressedMask |= fnBit(event.control);
                fnState.pressCombo |= fnBit(event.control);
            }
            else if (isStep(event.control)) {
                stepState.pressed = true;
                stepState.stepId = event.control;
                stepState.fnCombo = fnState.pressedMask;
            }
            break;

        case InputEventType::ButtonReleased:
            if (isFn(event.control)) {
                fnState.pressedMask &= ~fnBit(event.control);

                if (fnState.pressedMask == 0) {
                    fnState.holdTriggered = false;
                    fnState.holdConsumed = false;
                    fnState.modifierUsed = false;
                    fnState.pressCombo = 0;
                    fnState.holdCombo = 0;
                }
            }

            if (isStep(event.control)) {
                stepState.pressed = false;
                stepState.holdTriggered = false;
                stepState.stepUsed = false;
                stepState.fnCombo = 0;
            }
            break;

        case InputEventType::ButtonHold:
            if (isFn(event.control)) {
                fnState.holdTriggered = true;
                fnState.holdCombo = fnState.pressedMask;
            }
            else if (isStep(event.control)) {
                stepState.holdTriggered = true;
            }
            break;

        default:
            break;
    }
}
```

👉 Cette fonction devient la **source de vérité physique du hardware**.

---

## 🎯 2. `dispatchActions(event)`

Ici uniquement ton comportement :

```cpp
void dispatchActions(const InputEvent& event)
{
    switch(event.type)
    {
        case InputEventType::ButtonPressed:
            if (isFn(event.control))
                fnPressedAction(event, fnState.pressedMask);
            else if (isStep(event.control))
                stepPressedAction(event, stepState.fnCombo);
            break;

        case InputEventType::ButtonReleased:
            if (isStep(event.control) && stepState.pressed)
            {
                if (stepState.holdTriggered)
                    stepHoldReleaseAction(event, stepState.fnCombo);
                else
                    stepReleaseAction(event, stepState.fnCombo);
            }

            if (isFn(event.control))
            {
                if (fnState.pressedMask == 0)
                {
                    if (fnState.holdTriggered && !fnState.holdConsumed)
                        fnHoldReleaseAction(event, fnState.holdCombo);
                    else
                        fnReleaseAction(event, fnState.pressCombo);

                    fnReleasedAlways(event, fnState.pressCombo);
                }
            }
            break;

        case InputEventType::EncoderTurned:
            if (isEncoder(event.control))
                encoderAction(event, fnState.pressedMask);
            break;

        default:
            break;
    }
}
```

---

## 💡 Pourquoi c’est le meilleur design pour toi

### 1. Tu règles TON bug actuel immédiatement

* la modale ne casse plus l’état interne
* `stepPressedAction` redevient fiable
* `fnState` reste cohérent

---

### 2. Tu peux ajouter des modales sans casser le système

* confirm delete
* save
* load
* settings

---

### 3. Tu élimines le problème structurel actuel

Avant :

> "modal bloque tout → état cassé"

Après :

> "modal bloque les actions mais pas la physique"

---

### 4. Ton code devient scalable

Tu es en train de construire un système type :

* **input layer (hardware state)**
* **logic layer (musical behavior)**
* **UI layer (modal/feedback)**

C’est exactement la séparation qu’on voit dans des systèmes type DAW / trackers.

---

## ⚠️ Résumé brutal

* ❌ ne bloque jamais `updateStates()`
* ❌ ne mélange pas état + logique + UI
* ❌ ne `return` pas trop tôt dans `handleEvent()`

---

Si tu veux, prochaine étape logique pour ton architecture :
👉 transformer `FnMask + StepState` en **event stream pur (type mini engine DAW)**
là tu seras vraiment au niveau système pro.
































































## reqrite cmplet event

L’idée du “event stream pur” c’est de sortir complètement du modèle *“je stocke un état global (FnMask, StepState) puis je fais des if dessus”* pour passer à :

👉 **une suite d’événements immuables**
👉 + un **petit état séparé uniquement pour le contexte**
👉 + des “systèmes” qui consomment les events

C’est exactement ce que font les DAW / trackers modernes en backend.

---

# 🧠 1. Le concept

Au lieu de :

* `fnState`
* `stepState`
* `encoderAction()`
* callbacks mélangés

Tu produis un flux :

```cpp
InputEvent → InputMessage → System processing
```

---

# 📦 2. Nouveau modèle : InputMessage (event stream)

On remplace les états implicites par des messages explicites :

```cpp id="msg1"
enum class InputMessageType
{
    FnDown,
    FnUp,
    FnHold,

    StepDown,
    StepUp,
    StepHold,

    EncoderTurn
};
```

---

## Message struct

```cpp id="msg2"
struct InputMessage
{
    InputMessageType type;

    ControlId control;

    FnMask fnMask;     // snapshot au moment de l’event
    uint8_t value;     // encoder etc.

    bool isHoldEvent;
};
```

👉 IMPORTANT : plus de `FnState` global pour la logique métier.

---

# ⚙️ 3. Le changement clé : INPUT devient un “producer”

Dans `InputMode` :

```cpp id="msg3"
virtual void handleEvent(const InputEvent& event)
{
    updateRawState(event); // si nécessaire

    InputMessage msg = toMessage(event);

    if (modalState != ModalState::None)
    {
        modalSystem.process(msg);
        return;
    }

    dispatch(msg);
}
```

---

# 🔄 4. Transformation event → message

```cpp id="msg4"
InputMessage toMessage(const InputEvent& e)
{
    InputMessage msg;
    msg.control = e.control;
    msg.value = e.value;

    msg.fnMask = currentFnMask(); // snapshot simple

    switch(e.type)
    {
        case InputEventType::ButtonPressed:
            if (isFn(e.control))
                msg.type = InputMessageType::FnDown;
            else if (isStep(e.control))
                msg.type = InputMessageType::StepDown;
            break;

        case InputEventType::ButtonReleased:
            if (isFn(e.control))
                msg.type = InputMessageType::FnUp;
            else if (isStep(e.control))
                msg.type = InputMessageType::StepUp;
            break;

        case InputEventType::ButtonHold:
            if (isFn(e.control))
                msg.type = InputMessageType::FnHold;
            else if (isStep(e.control))
                msg.type = InputMessageType::StepHold;
            break;

        case InputEventType::EncoderTurned:
            msg.type = InputMessageType::EncoderTurn;
            break;
    }

    return msg;
}
```

---

# 🧩 5. Dispatch = mini engine (le cœur DAW)

```cpp id="msg5"
void dispatch(const InputMessage& msg)
{
    switch(msg.type)
    {
        case InputMessageType::FnDown:
            onFnDown(msg);
            break;

        case InputMessageType::FnHold:
            onFnHold(msg);
            break;

        case InputMessageType::FnUp:
            onFnUp(msg);
            break;

        case InputMessageType::StepDown:
            onStepDown(msg);
            break;

        case InputMessageType::StepUp:
            onStepUp(msg);
            break;

        case InputMessageType::EncoderTurn:
            onEncoder(msg);
            break;
    }
}
```

---

# 🎯 6. Ce que deviennent tes states actuels

## ❌ FnState / StepState disparaissent du métier

Ils deviennent optionnels ou purement internes :

* anti-rebond
* tracking hardware
* détection hold

👉 MAIS ils ne sont plus utilisés dans les règles musicales

---

# 🎹 7. Exemple concret (ton cas step)

Avant :

```cpp
stepPressedAction(event, fnMask);
stepState.fnCombo
stepState.pressed
```

Après :

```cpp
void onStepDown(const InputMessage& msg)
{
    if (msg.fnMask == 0)
    {
        sequencer.toggleStep(...);
    }

    if (msg.fnMask == FN4)
    {
        editStepParams(...);
    }
}
```

👉 plus d’état caché, juste des snapshots.

---

# 🪩 8. Exemple hold UX (ton confirm menu)

Avant :

* fnHoldAction
* fnHoldReleaseAction
* modalState hack

Après :

```cpp
void onFnHold(const InputMessage& msg)
{
    if (msg.fnMask == (FN4 | FN7))
    {
        modal.open(ConfirmDeleteQuarterNote);
    }
}
```

Et modal consomme ensuite :

```cpp
modal.process(msg)
```

```cpp
if (msg.type == FnDown && msg.control == Fn0)
    confirm();
```

---

# 🚀 9. Pourquoi c’est un VRAI upgrade

### ✔ plus de bug d’état

FnMask jamais “bloqué”

### ✔ modales propres

elles consomment des events

### ✔ testable

tu peux simuler :

```cpp
send(FnDown, FN4);
send(FnHold, FN4);
send(FnUp, FN4);
```

### ✔ DAW-style architecture

tu es dans du :

* Ableton-like input layer
* tracker-style event stream
* ECS-lite sans ECS

---

# 🧠 10. Vision finale

Tu passes de :

```
hardware state machine + callbacks + flags
```

à :

```
INPUT EVENTS → STREAM → SYSTEMS → ACTIONS
```

---

# ⚡ Conclusion

Ton système actuel est déjà bon, mais :

👉 il devient difficile à étendre (modales, combos, gestures)

Le passage event-stream te donne :

* zéro état caché
* zéro bug de synchronisation
* logique musicale plus propre
* extensibilité type DAW

---

Si tu veux, prochaine étape intéressante :

👉 je peux te montrer comment faire un **mini “sequencer engine” style Ableton note pipeline basé sur ce stream** (avec quantization + event scheduling propre).
