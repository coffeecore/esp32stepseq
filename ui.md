Je pense que tu es arrivé au point où il faut **séparer les responsabilités**, mais **pas les états**.

Et la réponse à ta dernière question est justement ce qui dicte l'architecture.

> *Comment mon input dépend de mon screen ?*

À mon avis : **il ne doit pas en dépendre directement.**

Il doit dépendre d'un **mode d'interface** (ou "workspace"), pas d'une classe `Screen`.

## Je partirais sur ça

```text
                 UIState
        +----------------------+
        | workspace            |
        | modal                |
        | selectedInstrument   |
        +----------------------+
             ▲             ▲
             │             │
     InputEngine      DisplayEngine
             │             │
      resolveLayer()  resolveScreen()
```

Ainsi :

* `InputEngine` lit `UIState.workspace` pour savoir quels layers sont possibles.
* `DisplayEngine` lit le même `UIState.workspace` pour savoir quel écran afficher.

Ils restent synchronisés **sans se connaître**.

---

## Un `UIState` plutôt qu'un `DisplayContext`

Je ne l'appellerais pas `DisplayContext`, parce qu'il ne sert pas qu'à l'affichage.

Par exemple :

```cpp
enum class Workspace
{
    Sequencer,
    Instrument,
    Mixer,
};
```

Puis :

```cpp
struct UIState
{
    Workspace workspace = Workspace::Sequencer;

    ModalState modal = ModalState::None;

    uint8_t selectedInstrument = 0;
};
```

Ce n'est pas un contexte graphique, c'est l'état de l'interface.

---

## DisplayEngine

Très simple.

```cpp
class DisplayEngine
{
public:
    Display& display;
    UIState& ui;

    SequencerScreen sequencer;
    InstrumentScreen instrument;

    Screen* currentScreen = &sequencer;

    DisplayEngine(Display& d, UIState& u)
        : display(d),
          ui(u),
          sequencer(u),
          instrument(u)
    {}

    void resolveScreen()
    {
        switch (ui.workspace)
        {
            case Workspace::Sequencer:
                currentScreen = &sequencer;
                break;

            case Workspace::Instrument:
                currentScreen = &instrument;
                break;
        }
    }

    void draw()
    {
        resolveScreen();

        display.clear();

        currentScreen->draw(display);

        display.present();
    }
};
```

---

## Une Screen

```cpp
class Screen
{
public:
    virtual void draw(Display&) = 0;
};
```

Puis :

```cpp
class SequencerScreen : public Screen
{
public:

    UIState& ui;
    SequencerTimer& sequencer;

    SequencerScreen(UIState& u,
                    SequencerTimer& s)
        : ui(u),
          sequencer(s)
    {
    }

    void draw(Display& d) override
    {
        d.drawHeader(sequencer);

        d.drawTracks(sequencer);

        d.drawStatus();
    }
};
```

Tu remarques que cette classe ne connaît pas les boutons.

---

## Et l'InputEngine ?

Même idée.

```cpp
resolveLayer()
{
    if(ui.workspace == Workspace::Instrument)
    {
        ...
    }
    else
    {
        ...
    }
}
```

Par exemple :

```text
Workspace::Sequencer

    GlobalLayer
    StepLayer
    NavigationLayer

Workspace::Instrument

    InstrumentLayer
    InstrumentEnvelopeLayer
    InstrumentFXLayer
```

---

## Qui change le workspace ?

Par exemple :

```text
Fn3
↓

GlobalLayer

↓

ui.workspace = Workspace::Instrument;
```

Le coup suivant :

```text
DisplayEngine

↓

InstrumentScreen
```

et

```text
InputEngine

↓

InstrumentLayer
```

Les deux changent automatiquement.

---

# Initialisation du `UIState`

C'est probablement la partie la plus importante.

Le `UIState` ne doit appartenir **ni** à `InputEngine`, **ni** à `DisplayEngine`.

Il appartient à l'application, exactement comme ton `Sequencer`.

Par exemple :

```cpp
UIState ui;

SequencerTimer sequencer;
Display display;

InputEngine inputEngine(
    ui,
    sequencer,
    display,
    rotaryEncoders);

DisplayEngine displayEngine(
    ui,
    display,
    sequencer);
```

Les deux moteurs reçoivent une **référence** vers le même objet :

```text
            UIState
               ▲
        ┌──────┴──────┐
        │             │
        ▼             ▼
InputEngine     DisplayEngine
```

Ainsi :

* `InputEngine` peut faire :

```cpp
ui.workspace = Workspace::Instrument;
```

* et lors du prochain rafraîchissement, `DisplayEngine` voit immédiatement :

```cpp
ui.workspace == Workspace::Instrument
```

sans qu'il soit nécessaire d'envoyer un message ou d'appeler une fonction.

C'est exactement le même principe que tu utilises déjà avec `InputContext` : plusieurs objets partagent une même source de vérité.

---

## Je ne modifierais presque pas `InputContext`

Je garderais :

```cpp
InputContext
```

pour ce qui concerne les entrées :

* `stepHeld`
* `fnMask`
* `stepId`
* `stepUsedAsModifier`
* l'état des Fn (`fnState`)
* etc.

Et je créerais un petit :

```cpp
UIState
```

pour :

* `workspace`
* `modal`
* `selectedInstrument`
* éventuellement l'onglet actif dans l'éditeur d'instrument.

---

## Au final

Je séparerais les données comme ceci :

```text
InputContext
--------------
stepHeld
fnMask
stepId
stepUsedAsModifier
...

UIState
--------------
workspace
modal
selectedInstrument
...

Sequencer
--------------
patterns
tracks
steps
tempo
...
```

Je trouve que cette séparation est assez naturelle :

* **`InputContext`** = ce que fait actuellement l'utilisateur avec les contrôles.
* **`UIState`** = où il se trouve dans l'interface.
* **`Sequencer`** = les données du projet.

Et `InputEngine` comme `DisplayEngine` lisent le même `UIState` pour rester synchronisés, sans dépendre l'un de l'autre ni des classes `Screen`. C'est un couplage faible, simple à comprendre et qui évoluera facilement quand tu ajouteras d'autres écrans.
