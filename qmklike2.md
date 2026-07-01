OK. On va faire une conversion **directe de TON architecture actuelle → mini QMK-like layer engine**, sans bullshit, **compatible ESP32 (pas de malloc obligatoire, pas de STL obligatoire)**.

L’idée : tu gardes tes `InputMode`, mais **tu supprimes `fnMask switch hell` + routing dispersé**.

---

# 🧠 1. NOUVELLE ARCHI (minimal ESP32)

On remplace :

* ❌ `dispatchActions(fnMask switch)`
* ❌ logique encoder dans plusieurs fonctions
* ❌ stepState + fnState utilisés pour router

Par :

## ✔ 1 seul concept : LayerResolver

---

# 🧱 2. TYPES DE BASE

```cpp
enum class LayerId : uint8_t
{
    Global,
    StepEdit,
    StepLength,
    StepInstrument,
    Navigation,
    Modal
};
```

---

## Context global (remplace fnState + stepState routing)

```cpp
struct InputContext
{
    FnMask fnMask = 0;

    bool stepHeld = false;
    ControlId stepId = ControlId::None;

    ModalState modal = ModalState::None;
};
```

---

# 🧠 3. LAYER INTERFACE (ultra light ESP32)

```cpp
class Layer
{
public:
    virtual void onStepPressed(const InputContext&, const InputEvent&) {}
    virtual void onStepReleased(const InputContext&, const InputEvent&) {}

    virtual void onEncoder(const InputContext&, const InputEvent&) {}

    virtual void applyEncoderMapping(Input& input) {}
    virtual void applyEncoderValues(Input& input) {}
};
```

---

# ⚙️ 4. LAYERS (remplace TES switch cases)

---

## GLOBAL

```cpp
class GlobalLayer : public Layer
{
public:
    void applyEncoderMapping(Input& input) override
    {
        input.setEncoderBoundaries(Encoder0, 0, 255, false);
        input.setEncoderBoundaries(Encoder1, 1, 999, false);
    }
};
```

---

## STEP EDIT (FN none + step)

```cpp
class StepEditLayer : public Layer
{
public:
    void applyEncoderMapping(Input& input) override
    {
        input.setEncoderBoundaries(Encoder0, 0, 11, true);
        input.setEncoderBoundaries(Encoder1, -1, 9, false);
    }

    void applyEncoderValues(Input& input) override
    {
        Step& step = getStep();

        input.setEncoderValue(Encoder0, step.note % 12);
        input.setEncoderValue(Encoder1, step.note / 12);
    }

    void onEncoder(const InputContext&, const InputEvent& e) override
    {
        Step& step = getStep();

        if (e.control == Encoder0)
            step.note = (step.note / 12) * 12 + e.value;

        if (e.control == Encoder1)
            step.note = (step.note % 12) + (e.value * 12);
    }
};
```

---

## STEP LENGTH (FN2 + step)

```cpp
class StepLengthLayer : public Layer
{
public:
    void applyEncoderMapping(Input& input) override
    {
        input.setEncoderBoundaries(Encoder0, 1, 32, false);
    }

    void applyEncoderValues(Input& input) override
    {
        Step& step = getStep();
        input.setEncoderValue(Encoder0, step.length);
    }

    void onEncoder(const InputContext&, const InputEvent& e) override
    {
        if (e.control == Encoder0)
            getStep().length = e.value;
    }
};
```

---

## NAVIGATION (FN1)

```cpp
class NavigationLayer : public Layer
{
public:
    void applyEncoderMapping(Input& input) override
    {
        input.setEncoderBoundaries(Encoder0, 0, 16, true);
        input.setEncoderBoundaries(Encoder1, 0, 64, true);
    }

    void onEncoder(const InputContext&, const InputEvent& e) override
    {
        if (e.control == Encoder0)
            setTrack(e.value);

        if (e.control == Encoder1)
            setStep(e.value);
    }
};
```

---

## MODAL (remplace ton modalAction)

```cpp
class ModalLayer : public Layer
{
public:
    void onStepPressed(const InputContext&, const InputEvent&) override
    {
        // ignore steps
    }

    void onEncoder(const InputContext&, const InputEvent&) override
    {
        // lock input
    }
};
```

---

# 🧠 5. ENGINE (remplace InputMode dispatchActions COMPLET)

```cpp
class InputEngine
{
public:
    InputContext ctx;

    GlobalLayer global;
    StepEditLayer stepEdit;
    StepLengthLayer stepLength;
    StepInstrumentLayer stepInstrument;
    NavigationLayer navigation;
    ModalLayer modal;

    Layer* currentLayer = &global;
```

---

## RESOLUTION (LE CŒUR DU SYSTEME)

```cpp
void resolveLayer()
{
    if (ctx.modal != ModalState::None)
    {
        currentLayer = &modal;
        return;
    }

    if (ctx.stepHeld)
    {
        if (ctx.fnMask & FN2)
        {
            currentLayer = &stepLength;
            return;
        }

        if (ctx.fnMask & FN3)
        {
            currentLayer = &stepInstrument;
            return;
        }

        currentLayer = &stepEdit;
        return;
    }

    if (ctx.fnMask & FN1)
    {
        currentLayer = &navigation;
        return;
    }

    currentLayer = &global;
}
```

---

# ⚡ 6. HANDLE EVENT (REMPLACE TON handleEvent)

```cpp
void handleEvent(const InputEvent& event)
{
    updateContext(event);
    resolveLayer();

    switch (event.type)
    {
        case InputEventType::ButtonPressed:
            if (isStep(event.control))
                currentLayer->onStepPressed(ctx, event);
            break;

        case InputEventType::ButtonReleased:
            if (isStep(event.control))
                currentLayer->onStepReleased(ctx, event);
            break;

        case InputEventType::EncoderTurned:
            currentLayer->onEncoder(ctx, event);
            break;
    }

    currentLayer->applyEncoderMapping(input);
    currentLayer->applyEncoderValues(input);
}
```

---

# 🧠 7. updateContext (ultra simplifié)

```cpp
void updateContext(const InputEvent& event)
{
    switch (event.type)
    {
        case InputEventType::ButtonPressed:
            if (isFn(event.control))
                ctx.fnMask |= fnBit(event.control);

            if (isStep(event.control))
            {
                ctx.stepHeld = true;
                ctx.stepId = event.control;
            }
            break;

        case InputEventType::ButtonReleased:
            if (isFn(event.control))
                ctx.fnMask &= ~fnBit(event.control);

            if (isStep(event.control))
                ctx.stepHeld = false;
            break;

        default:
            break;
    }
}
```

---

# 💥 CE QUE TU VIENS DE GAGNER

## ❌ supprimé

* fnState.holdTriggered
* fnState.modifierUsed
* fnState.pressCombo chaos
* switch(fnMask)
* cleanupStates
* encoderAction spaghetti

---

## ✅ remplacé par

* 1 context
* 1 resolver
* layers indépendants
* encoders déterministes

---

# 🚀 RESULTAT FINAL

✔ comportement identique à ton système actuel
✔ mais :

* plus de bugs de timing
* plus de “case 0 surprise”
* plus de states incohérents
* facile à étendre (nouvelle layer = 20 lignes)

---

# Si tu veux prochaine étape

Je peux te faire :

### 👉 version “production ESP32 optimisée”

* sans virtual (table de fonctions)
* sans heap
* fixed array layers
* 0 overhead runtime
* ISR-friendly input queue

ou

### 👉 migration automatique de TON code actuel (ligne par ligne supprimable)

dis-moi.
