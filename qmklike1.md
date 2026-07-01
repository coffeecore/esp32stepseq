Bonne idée — et surtout, ça va te régler ton problème de “FnMask partout + encoders incohérents”.

L’objectif ici : **remplacer ton chaos de `if(fnMask)` + `stepState` + `modalState` par un moteur de couches (layers) déterministe**, comme QMK.

---

# 🧠 Concept (version ESP32-friendly)

On va construire 3 choses :

## 1. Layers (priorité = stack)

Chaque layer peut :

* redéfinir les encoders
* rediriger les actions
* activer des comportements (step edit, navigation, etc.)

---

## 2. Context global (état brut)

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

## 3. Layer system (le cœur)

```cpp
enum class LayerId : uint8_t
{
    Global,
    Navigation,
    StepEdit,
    StepLength,
    StepInstrument,
    Modal
};
```

---

# 🔥 Layer API (QMK-like)

Chaque layer peut override :

```cpp
class Layer
{
public:
    virtual ~Layer() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void applyEncoderMapping(Input& input) {}
    virtual void applyEncoderValues(Input& input) {}

    virtual void onStepPressed(InputContext&, InputEvent&) {}
    virtual void onStepReleased(InputContext&, InputEvent&) {}
    virtual void onEncoder(InputContext&, InputEvent&) {}
};
```

---

# 🧱 Layer Stack Manager

```cpp
class LayerStack
{
public:
    void push(LayerId id)
    {
        stack.push_back(id);
    }

    void pop()
    {
        if (!stack.empty())
            stack.pop_back();
    }

    LayerId top() const
    {
        return stack.empty() ? LayerId::Global : stack.back();
    }

    bool contains(LayerId id) const
    {
        return std::find(stack.begin(), stack.end(), id) != stack.end();
    }

private:
    std::vector<LayerId> stack;
};
```

---

# ⚙️ Input Engine (remplace ton dispatchActions)

```cpp
class InputEngine
{
public:
    LayerStack layers;
    InputContext ctx;

    void handleEvent(const InputEvent& event)
    {
        updateContext(event);

        Layer& layer = resolveLayer();

        dispatchToLayer(layer, event);

        applyEncoder(layer);
    }
```

---

## 1. Context update (ton ancien updateStates simplifié)

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

    updateLayers();
}
```

---

## 2. Layer resolution (IMPORTANT)

```cpp
Layer& resolveLayer()
{
    if (ctx.modal == ModalState::Confirm)
        return *modalLayer;

    if (ctx.stepHeld && (ctx.fnMask & FN2))
        return *stepLengthLayer;

    if (ctx.stepHeld && (ctx.fnMask & FN3))
        return *stepInstrumentLayer;

    if (ctx.stepHeld)
        return *stepEditLayer;

    if (ctx.fnMask & FN1)
        return *navigationLayer;

    return *globalLayer;
}
```

👉 Ça remplace 100% tes `switch(fnMask)`.

---

## 3. Dispatch propre

```cpp
void dispatchToLayer(Layer& layer, const InputEvent& event)
{
    switch (event.type)
    {
        case InputEventType::ButtonPressed:
            if (isStep(event.control))
                layer.onStepPressed(ctx, const_cast<InputEvent&>(event));
            break;

        case InputEventType::ButtonReleased:
            if (isStep(event.control))
                layer.onStepReleased(ctx, const_cast<InputEvent&>(event));
            break;

        case InputEventType::EncoderTurned:
            layer.onEncoder(ctx, const_cast<InputEvent&>(event));
            break;

        default:
            break;
    }
}
```

---

## 4. Encoders (LE POINT IMPORTANT)

```cpp
void applyEncoder(Layer& layer)
{
    layer.applyEncoderMapping(input);
    layer.applyEncoderValues(input);
}
```

---

# 🎹 Exemple concret (Step Length Layer)

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

    void onEncoder(InputContext&, InputEvent& e) override
    {
        if (e.control == Encoder0)
            step.length = e.value;
    }
};
```

---

# 💥 Résultat

## Avant

* 400 lignes de `switch(fnMask)`
* states partagés partout
* bugs de timing (ton problème actuel)
* encoders incohérents

## Après

* 1 seul endroit décide du mode
* 1 layer = 1 comportement
* encoders toujours cohérents
* plus de “je comprends pas pourquoi case 0”

---

# 🧠 Pourquoi ton bug disparaît avec ça

Ton problème actuel :

> Fn2 + Step → mauvais fnMask au moment encoderAction

👉 Ici impossible, car :

* le layer est recalculé AVANT encoder
* pas de dépendance à un snapshot instable

---

# 🚀 Si tu veux next step

Je peux te faire :

### 👉 version ultra propre (production-ready ESP32)

* zero malloc
* array fixed-size (pas vector)
* layer priority system
* modal stack (comme QMK "oneshot + layer lock")
* encoder routing table compile-time

Ou encore mieux :

### 👉 convertir TON code actuel automatiquement vers layers

(et te montrer exactement quoi supprimer)
