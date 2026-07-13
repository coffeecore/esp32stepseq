# 🚀 ESP32Synth — Démarrage rapide

## Installation

1. Copie le dossier `ESP32Synth` (contenant `src/`, `examples/`, `library.properties`...) dans ton dossier `Arduino/libraries/`, **ou** utilise-le comme composant PlatformIO.
2. Dans ton `.ino` / `.cpp` :
   ```cpp
   #include "ESP32Synth.h" // Inclut automatiquement ESP32SynthNotes.h
   ESP32Synth synth;
   ```
3. Vérifie/adapte `src/ESP32Synth_Config.hpp` selon tes besoins (nombre de voix, RAM disponible) **avant** de compiler.

## Ton premier programme (I2S + DAC externe type PCM5102A)

```cpp
#include "ESP32Synth.h"

const int BCK_PIN  = 26; // Bit Clock
const int WS_PIN   = 25; // Word Select (LRC)
const int DATA_PIN = 22; // Data Out

ESP32Synth synth;

void setup() {
  Serial.begin(115200);

  if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {
    Serial.println("Erreur : impossible d'initialiser le synthétiseur !");
    while (1) delay(1000);
  }

  synth.setMasterVolume(200);       // Volume général (0-255)
  synth.setWave(0, WAVE_SAW);       // Voix 0 en dent de scie
  synth.setEnv(0, 10, 200, 150, 400); // Attaque 10ms, Decay 200ms, Sustain 150, Release 400ms
}

void loop() {
  synth.noteOn(0, c4, 255);  // Joue Do4 à pleine puissance
  delay(500);
  synth.noteOff(0);
  delay(500);
}
```

C'est tout : `synth.begin()` démarre en interne une tâche audio dédiée sur le **Core 1** — ta `loop()` reste libre pour gérer boutons, écran, MIDI, etc. sans jamais bloquer le son.

## Pourquoi 500 voix ? (philosophie du projet)

La polyphonie extrême d'ESP32Synth (300+ voix sur ESP32 classique, jusqu'à 500 sur S3) n'est pas qu'une statistique marketing : c'est une **démonstration d'efficacité**. En éliminant les `float`, les divisions matérielles et les branchements du chemin de rendu audio critique, la bibliothèque dégage une immense marge processeur — que tu peux ensuite réinvestir dans des blocs de synthèse avancés :

- synthèse FM 6 opérateurs (façon Yamaha DX7) ;
- modélisation physique acoustique (cordes, guides d'ondes, peaux de tambour) ;
- filtres résonants multi-pôles adaptatifs ;
- moteurs de waveshaping et de distorsion de phase.

Pour construire ces blocs toi-même, garde en tête la règle : **arithmétique à virgule fixe 16.16 ou 32.32, tables de correspondance (LUT), décalages de bits (`>>`)** — jamais de `float` ni de division dans la boucle de rendu.

## Empreinte mémoire et isolation matérielle

Pour maximiser la RAM disponible, chaque voix (`struct Voice`) utilise un `union` C++ qui superpose en mémoire les champs mutuellement exclusifs selon le type d'onde utilisé :

```cpp
struct Voice {
    int64_t slideVolCurr; // Alignement 8 octets pour le pipeline Xtensa
    int64_t slideVolInc;

    union {
        // Mode WAVE_SAMPLE & WAVE_STREAM
        struct {
            uint64_t samplePos1616;
            uint32_t sampleInc1616;
            uint32_t sampleLoopStart;
            uint32_t sampleLoopEnd;
            uint32_t streamFracAccum;
        };
        // Mode WAVE_WAVETABLE
        struct {
            const void* wtData;
            uint32_t    wtSize;
        };
        // Mode WAVE_CUSTOM
        uint32_t cw[6];
    };
    // ...
};
```

Ce `union` garantit que quelle que soit la configuration de la voix, son empreinte mémoire de base reste minimale — ce qui limite les défauts de cache (cache misses).

## Où aller ensuite ?

- **`01_Guide_API.md`** pour la référence complète de toutes les fonctions.
- **`exemples/`** pour des cas d'usage concrets, traduits et expliqués, allant des formes d'onde de base jusqu'à des synthés physiques complets avec écran tactile.
