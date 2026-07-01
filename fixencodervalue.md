Ton problème vient surtout d’un point architectural : tu essaies de **piloter l’état des encodeurs depuis les actions clavier**, mais tes callbacks (`stepPressedAction`, `encoderAction`) sont déclenchés **dans le même cycle d’input**, donc tu peux facilement avoir :

* un ancien “mode encodeur” encore actif
* un `fnMask` pas encore mis à jour comme tu crois
* ou un `encoderAction()` qui reçoit encore un contexte précédent (d’où ton “case 0” surprise)

---

## 🎯 Le vrai objectif

Tu veux faire ça proprement :

> “Quand j’appuie Step + Fn2, je veux que l’encodeur change de mode (range + valeur) AVANT que je le tourne.”

Donc il te faut un système **d’état d’encodeur (routing)**, pas des `setEncoder...()` dispersés dans les actions.

---

# ❌ Problème dans ton design actuel

Tu fais ça :

```cpp
stepPressedAction(...)
{
    input.setEncoderBoundaries(...)
    input.setEncoderValue(...)
}
```

Puis plus tard :

```cpp
encoderAction(...)
{
    switch(fnMask)
    {
        case 0: ...
        case FN2: ...
    }
}
```

👉 Le problème :

* `fnMask` utilisé dans `encoderAction` n’est pas forcément synchronisé avec le moment où tu as changé le mode
* et surtout : tu mixes **configuration** et **exécution**

---

# ✅ Solution propre (style QMK / FSM)

Tu dois séparer :

## 1. “mode encodeur courant” (STATE)

Ajoute un état dédié :

```cpp
enum class EncoderMode : uint8_t
{
    Global,
    StepNote,
    StepLength,
    StepInstrument,
    TrackVolume,
    TrackTranspose,
    Navigation
};

struct EncoderContext
{
    EncoderMode mode = EncoderMode::Global;
    FnMask fnMask = 0;
    bool stepHeld = false;
    ControlId stepId = ControlId::None;
};
```

---

## 2. Tu mets à jour le mode UNE SEULE FOIS

Dans `updateStates()` ou `dispatchActions()` :

```cpp
void updateEncoderContext()
{
    if (stepState.pressed)
    {
        if (fnState.pressedMask & FN2)
            encoderContext.mode = EncoderMode::StepLength;
        else if (fnState.pressedMask & FN3)
            encoderContext.mode = EncoderMode::StepInstrument;
        else
            encoderContext.mode = EncoderMode::StepNote;
    }
    else if (fnState.pressedMask & FN1)
    {
        encoderContext.mode = EncoderMode::Navigation;
    }
    else
    {
        encoderContext.mode = EncoderMode::Global;
    }

    encoderContext.fnMask = fnState.pressedMask;
}
```

👉 tu appelles ça **à chaque event** ou après updateStates.

---

## 3. Tu n’appelles PLUS `setEncoder...` dans les actions métier

❌ interdit :

```cpp
stepPressedAction -> setEncoderBoundaries()
```

---

## 4. Tout passe dans un seul point : `applyEncoderMapping()`

```cpp
void applyEncoderMapping()
{
    switch (encoderContext.mode)
    {
        case EncoderMode::Global:
            input.setEncoderBoundaries(Encoder0, 0, 255, false);
            input.setEncoderBoundaries(Encoder1, 1, 999, false);
            break;

        case EncoderMode::StepNote:
            input.setEncoderBoundaries(Encoder0, 0, 11, true);
            input.setEncoderBoundaries(Encoder1, -1, 9, false);
            break;

        case EncoderMode::StepLength:
            input.setEncoderBoundaries(Encoder0, 1, 32, false);
            break;

        case EncoderMode::StepInstrument:
            input.setEncoderBoundaries(Encoder1, 0, 11, false);
            break;

        case EncoderMode::Navigation:
            input.setEncoderBoundaries(Encoder0, 0, 16, true);
            input.setEncoderBoundaries(Encoder1, 0, 64, true);
            break;
    }

    syncEncoderValues();
}
```

---

## 5. Valeurs séparées (IMPORTANT)

```cpp
void syncEncoderValues()
{
    switch (encoderContext.mode)
    {
        case EncoderMode::StepNote:
        {
            Step& step = getCurrentStep();

            input.setEncoderValue(Encoder0, step.note % 12);
            input.setEncoderValue(Encoder1, step.note / 12);
            break;
        }

        case EncoderMode::StepLength:
        {
            Step& step = getCurrentStep();
            input.setEncoderValue(Encoder0, step.length);
            break;
        }
    }
}
```

---

## 🔥 Résultat

Tu obtiens :

### ✔ stable

* aucun “case 0 surprise”
* pas de conflit fnMask/states
* pas de double écriture encoder

### ✔ prévisible

* mode encodeur = 1 variable
* tout découle de ça

### ✔ scalable (style QMK layers)

* tu peux ajouter 10 modes sans toucher à l’input system

---

# 💡 Réponse directe à ton problème

> “comment setter proprement l’intervalle et la valeur avant de tourner”

👉 réponse :

**Tu ne dois jamais le faire dans `stepPressedAction`.**

Tu dois :

1. détecter état (Fn + Step)
2. mettre à jour `encoderContext.mode`
3. appeler `applyEncoderMapping()`
4. encoderAction ne fait QUE modifier le modèle

---

# Si tu veux aller encore plus loin

Je peux te transformer ça en :

### 👉 mini QMK-like layer engine

* layers empilables (Fn1 + Fn2)
* priorité
* override encoder routing
* context stack (modal inclus)

Ça te ferait gagner énormément de complexité dans ton code actuel.
