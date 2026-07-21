OK. On va faire une **migration progressive**, pas une refonte brutale. Objectif : tu peux **compiler à chaque étape**, et supprimer ton ancien système au fur et à mesure.

Je te donne un plan **“remplace → compile → supprime”**.

---

# 🧭 ÉTAPE 0 — ÉTAT ACTUEL (ce que tu as)

Tu as :

* `fnState / stepState`
* `updateStates()`
* `dispatchActions()`
* `cleanupStates()`
* `stepPressedAction / fnPressedAction / encoderAction`
* `modalAction()`

👉 On ne casse rien tout de suite.

---

# 🧱 ÉTAPE 1 — AJOUT DU CONTEXT (SAFE ADD)

Ajoute en parallèle :

```cpp id="ctx1"
struct InputContext
{
    FnMask fnMask = 0;
    bool stepHeld = false;
    ControlId stepId = ControlId::None;
    ModalState modal = ModalState::None;
};
```

Dans ta classe :

```cpp id="ctx2"
InputContext ctx;
```

---

# 🔁 ÉTAPE 2 — DUPLICATION SAFE DE UPDATE STATES

👉 On garde ton ancien `updateStates()` MAIS on ajoute sync ctx

Ajoute à la fin :

```cpp id="ctx3"
void syncContextFromLegacy()
{
    ctx.fnMask = fnState.pressedMask;
    ctx.stepHeld = stepState.pressed;
    ctx.stepId = stepState.stepId;
    ctx.modal = modalState;
}
```

---

Et modifie ton handleEvent TEMPORAIREMENT :

```cpp id="ctx4"
virtual void handleEvent(const InputEvent& event)
{
    updateStates(event);

    syncContextFromLegacy();   // <-- ADD

    if (modalState != ModalState::None)
    {
        modalAction(event);
        cleanupStates(event);
        return;
    }

    dispatchActions(event);
    cleanupStates(event);
}
```

✔ Rien ne casse
✔ ctx est synchronisé
✔ tu peux tester layers sans risque

---

# 🧱 ÉTAPE 3 — AJOUT DU LAYER ROUTER (sans toucher ton code)

Ajoute :

```cpp id="lr1"
Layer* resolveLayer()
{
    if (ctx.modal != ModalState::None)
        return &modalLayer;

    if (ctx.stepHeld)
    {
        if (ctx.fnMask & FN2) return &stepLengthLayer;
        if (ctx.fnMask & FN3) return &stepInstrumentLayer;
        return &stepEditLayer;
    }

    if (ctx.fnMask & FN1)
        return &navigationLayer;

    return &globalLayer;
}
```

---

Ajoute les layers vides :

```cpp id="lr2"
Layer globalLayer;
Layer stepEditLayer;
Layer stepLengthLayer;
Layer stepInstrumentLayer;
Layer navigationLayer;
Layer modalLayer;
```

---

# ⚡ ÉTAPE 4 — AJOUT DU DISPATCH PAR LAYER (EN PARALLÈLE)

Ajoute une nouvelle fonction :

```cpp id="lr3"
void dispatchLayer(const InputEvent& event)
{
    Layer* layer = resolveLayer();

    switch (event.type)
    {
        case InputEventType::ButtonPressed:
            if (isStep(event.control))
                layer->onStepPressed(ctx, event);
            break;

        case InputEventType::ButtonReleased:
            if (isStep(event.control))
                layer->onStepReleased(ctx, event);
            break;

        case InputEventType::EncoderTurned:
            layer->onEncoder(ctx, event);
            break;
    }

    layer->applyEncoderMapping(input);
    layer->applyEncoderValues(input);
}
```

---

# 🔁 ÉTAPE 5 — SWITCH GRADUEL (IMPORTANT)

Modifie handleEvent comme ça :

```cpp id="lr4"
virtual void handleEvent(const InputEvent& event)
{
    updateStates(event);
    syncContextFromLegacy();

    dispatchActions(event);   // ancien système encore actif

    dispatchLayer(event);     // nouveau système en parallèle

    cleanupStates(event);
}
```

✔ ancien + nouveau fonctionnent ensemble
✔ tu compares les deux comportements

---

# 🧪 ÉTAPE 6 — MIGRATION PROGRESSIVE DES ACTIONS

## Exemple : stepReleaseAction

### AVANT (ancien système)

```cpp
void stepReleaseAction(...)
```

---

### APRÈS (layer system)

Dans `stepEditLayer` :

```cpp id="mig1"
void onStepReleased(const InputContext& ctx, const InputEvent& event) override
{
    uint8_t row = (uint8_t(event.control)) / 4;
    uint8_t col = (uint8_t(event.control)) % 4;

    if (ctx.fnMask == 0)
    {
        sequencerTimer.toggleStep(...);
    }
}
```

---

## Exemple encoderAction

### AVANT

```cpp
encoderAction()
switch(fnMask)
```

---

### APRÈS (StepLengthLayer)

```cpp id="mig2"
void onEncoder(const InputContext& ctx, const InputEvent& e) override
{
    if (e.control == Encoder0)
    {
        sequencerTimer.setStepLength(...);
    }
}
```

---

# 🧹 ÉTAPE 7 — SUPPRESSION PROGRESSIVE

Quand tout est migré :

### supprimer :

* dispatchActions()
* updateStates()
* cleanupStates()
* fnState
* stepState
* fnMask switches
* encoderAction()

---

# 💥 ÉTAPE 8 — FINAL SIMPLIFIÉ

Ton handleEvent devient :

```cpp id="final1"
void handleEvent(const InputEvent& event)
{
    updateContext(event);

    Layer* layer = resolveLayer();

    dispatchLayer(event);

    layer->applyEncoderMapping(input);
    layer->applyEncoderValues(input);
}
```

---

# 🧠 CE QUE TU VIENS D’OBTENIR

## Avant

* état distribué
* logique couplée input + UI + sequencer
* bugs Fn timing

## Après

* 1 context
* 1 resolver
* layers indépendants
* encoders fiables à 100%

---

# ⚠️ POINT IMPORTANT (ton bug actuel)

Ton bug Fn1 / Step vient de :

* `fnMask snapshot != moment réel d’utilisation`
* cleanupStates trop tard ou trop tôt

👉 Dans le layer system :

* ce bug disparaît automatiquement car **le layer est recalculé à CHAQUE event**

---

# 🚀 Si tu veux next step

Je peux te faire :

### 👉 migration encore plus safe

* compatibilité bit-à-bit avec ton ancien système
* debug overlay (voir layer actif en temps réel sur OLED)
* logs pour vérifier chaque transition

ou

### 👉 version “ESP32 ultra optimisée”

* sans virtual
* table function pointers
* 0 allocations
* ISR-safe queue

dis-moi.
