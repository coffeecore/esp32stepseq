Oui, le **handle est mieux** dans ton cas. J'avais commencé avec `stop(uint8_t instrument)` pour rester simple, mais pour un tracker/séquenceur polyphonique ce n'est pas suffisant.

Le problème avec :

```cpp
audio.stop(instrument);
```

est que si tu as :

```
Instrument 2 = Piano

Track 0 joue C4
Track 1 joue E4
Track 2 joue G4
```

Tu demandes :

```
stop(Piano)
```

Mais quelle note arrêter ?

Il faut identifier **la voix précise** créée lors du `play()`.

Donc on part plutôt sur :

```
Sequencer
      |
      | PlayNoteRequest
      v
IAudioEngine
      |
      | retourne un VoiceHandle
      v
TrackNoteState conserve ce handle
```

---

## 1) Commandes dans les Steps

Les commandes restent dans le `Step`, car elles appartiennent au morceau.

```cpp
enum class CommandType : uint8_t
{
    None,

    Arpeggio,
    Portamento,
    Vibrato
};


struct Command
{
    CommandType type;

    uint8_t data[4];
};
```

Exemple :

```cpp
Step step;

step.note = NOTE_C4;
step.length = 8;

step.commandCount = 2;

step.commands[0] =
{
    CommandType::Arpeggio,
    {0, 4, 7, 0}
};

step.commands[1] =
{
    CommandType::Portamento,
    {10, 0, 0, 0}
};
```

Le Step contient donc :

```
Note C4
durée 8 ticks
commandes :
    arp +0 +4 +7
    portamento vitesse 10
```

---

# 2) PlayNoteRequest

C'est l'objet qui sort du séquenceur.

Le moteur audio ne connaît pas `Step`.

Il reçoit une demande de lecture.

```cpp
constexpr uint8_t MAX_COMMANDS = 4;


struct PlayNoteRequest
{
    uint8_t note;

    uint8_t velocity;

    uint8_t instrument;

    uint16_t gate;


    const Command* commands;

    uint8_t commandCount;
};
```

---

# 3) VoiceHandle

On crée un identifiant opaque.

```cpp
struct VoiceHandle
{
    uint8_t id;

    bool valid() const
    {
        return id != 255;
    }
};
```

255 = aucun handle.

---

# 4) Interface audio

Le séquenceur dépend seulement de ça :

```cpp
class IAudioEngine
{
public:

    virtual ~IAudioEngine() = default;


    virtual void begin() = 0;


    virtual VoiceHandle play(
        const PlayNoteRequest& request
    ) = 0;


    virtual void stop(
        VoiceHandle voice
    ) = 0;
};
```

---

# 5) Ton TrackNoteState

Là ton équivalent de runtime devient :

```cpp
struct TrackNoteState
{
    bool playing = false;


    uint8_t note;

    uint16_t remainingTicks;


    VoiceHandle voice;
};
```

Tu n'as pas besoin de stocker l'instrument ici puisque le moteur possède le handle.

---

Quand un Step démarre :

```cpp
PlayNoteRequest request;


request.note =
    step.note + track.transpose;


request.velocity =
    track.volume;


request.instrument =
    step.instrument >= 0
        ? step.instrument
        : track.instrument;


request.gate =
    step.length;


request.commands =
    step.commands;


request.commandCount =
    step.commandCount;



runtime.voice =
    audio.play(request);


runtime.playing = true;

runtime.remainingTicks =
    step.length;
```

---

Quand la durée est terminée :

```cpp
runtime.remainingTicks--;


if(runtime.remainingTicks == 0)
{
    audio.stop(runtime.voice);

    runtime.playing = false;
}
```

---

# 6) Implémentation ESP32Synth

Maintenant ESP32Synth est caché derrière l'interface.

```cpp
class Esp32SynthEngine : public IAudioEngine
{
public:

    VoiceHandle play(
        const PlayNoteRequest& request
    ) override;


    void stop(
        VoiceHandle voice
    ) override;


private:

    Instrument instruments[12];


    uint8_t nextVoice = 0;
};
```

---

Le `play()` :

```cpp
VoiceHandle Esp32SynthEngine::play(
    const PlayNoteRequest& request)
{
    uint8_t voice =
        nextVoice++;


    Instrument& inst =
        instruments[request.instrument];


    //
    // Configuration instrument
    //

    applyInstrument(
        voice,
        inst);



    //
    // Commandes du Step
    //

    for(uint8_t i = 0;
        i < request.commandCount;
        i++)
    {
        const Command& cmd =
            request.commands[i];


        switch(cmd.type)
        {
            case CommandType::Arpeggio:

                synth.setArpeggio(
                    voice,
                    cmd.data[0],
                    cmd.data[1]);

                break;


            case CommandType::Portamento:

                synth.setPortamento(
                    voice,
                    cmd.data[0]);

                break;


            default:
                break;
        }
    }



    synth.noteOn(
        voice,
        request.note,
        request.velocity);



    return VoiceHandle{voice};
}
```

---

Le stop :

```cpp
void Esp32SynthEngine::stop(
    VoiceHandle voice)
{
    synth.noteOff(
        voice.id);
}
```

---

## Pourquoi je préfère cette version

Ton flux devient :

```
Step
 |
 | contient commandes
 |
Sequencer
 |
 | transforme en PlayNoteRequest
 |
IAudioEngine
 |
 | crée une voix
 |
VoiceHandle
 |
TrackNoteState
```

Le séquenceur ne sait pas :

* combien de voix existe le synthé ;
* comment fonctionne ESP32Synth ;
* si c'est un sample ou un oscillateur ;
* comment fonctionne l'ADSR.

Il sait juste :

> "J'ai demandé une note, voici la voix associée, je l'arrêterai plus tard."

Pour un tracker avec 12 instruments et potentiellement plusieurs notes simultanées, je partirais effectivement sur cette version avec handle. C'est plus robuste dès le départ.
