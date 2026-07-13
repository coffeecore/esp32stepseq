# 🎮 Exemples : projets « Fun »

Couvre : `SpaceShooter`, `PolifonyTest`, `ESP32SynthMidi (BETA)`, `Poly12_MIDI_ADSR_LFO_Glide` (exemple communautaire).

---

## 1. SpaceShooter — jeu vidéo sur écran OLED avec musique et bruitages générés

**Dossier d'origine :** `examples/Fun/SpaceShooter/`

### Ce que fait l'exemple
Un vrai petit jeu de tir spatial (shoot 'em up) jouable sur écran OLED SSD1306 avec 4 boutons (gauche, droite, tir, bouclier), où **tous les bruitages et la musique de fond sont générés en direct par ESP32Synth** — y compris un delay/écho cinématique en arrière-plan pour donner de l'ampleur au son.

### Code traduit (moteur de delay + en-têtes)

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP32Synth.h"

// ====================================================================================
// DÉFINITIONS MATÉRIELLES ET DE CONTRÔLE
// ====================================================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP32Synth synth;

#define PIN_BTN_LEFT   13
#define PIN_BTN_RIGHT  12
#define PIN_BTN_FIRE   14
#define PIN_BTN_SHIELD 27

// ====================================================================================
// MOTEUR DSP "CINEMA DELAY" (entiers uniquement, optimisé cache IRAM)
// ====================================================================================
#define DELAY_SIZE 8192
#define DELAY_MASK (DELAY_SIZE - 1)

int32_t delayBuffer[DELAY_SIZE] __attribute__((aligned(16)));
uint32_t delayWriteIdx = 0;

void IRAM_ATTR cinemaDSP(int32_t* mixBuffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        int32_t dry = mixBuffer[i];

        // Délai d'environ 125ms (écho long et majestueux)
        uint32_t readIdx = (delayWriteIdx + (DELAY_SIZE - 6000)) & DELAY_MASK;
        int32_t wet = delayBuffer[readIdx];

        // Mélange dans le bus de sortie master
        mixBuffer[i] = dry + ((wet * 115) >> 8);

        // Feedback pour garder le son "flottant"
        delayBuffer[delayWriteIdx] = dry + ((wet * 140) >> 8);
        delayWriteIdx = (delayWriteIdx + 1) & DELAY_MASK;
    }
}

// ====================================================================================
// VARIABLES DU MOTEUR DE JEU ET PHYSIQUE
// ====================================================================================
// (suite : gestion du vaisseau, des ennemis, des tirs, des collisions...)
```

### Explications
- L'`__attribute__((aligned(16)))` sur le buffer de delay garantit un alignement mémoire optimal pour les accès du processeur Xtensa — un détail de performance qu'on ne voit généralement que dans du code très optimisé.
- Ce projet est la démonstration ultime que le CPU restant après le rendu audio (grâce à l'efficacité d'ESP32Synth, cf. philosophie §1 du guide API) suffit à faire tourner **simultanément** un moteur de jeu complet (physique, collisions, affichage OLED) et un moteur audio avec effet de delay temps réel — le tout sur un seul ESP32.
- Le delay "cinéma" (~125 ms, feedback à ~55%) sert probablement à donner du corps aux bruitages de tir/explosion et à la musique de fond, pour un rendu plus "grand" malgré le haut-parleur/DAC probablement modeste utilisé.

---

## 2. PolifonyTest — test de stress de polyphonie

**Dossier d'origine :** `examples/Fun/PolifonyTest/`

### Ce que fait l'exemple
Un banc d'essai permettant de choisir facilement entre les 3 modes de sortie (DAC interne, I2S, PDM) via une simple macro, pour **pousser le nombre de voix simultanées à son maximum** et vérifier la stabilité du système (voir les limites testées au §2 du guide API : jusqu'à 340-500 voix selon la puce).

### Code traduit (configuration de sortie)

```cpp
#include <Arduino.h>
#include "ESP32Synth.h"

// Définitions des broches I2S (modifie selon ta carte)
#define I2S_BCLK 4
#define I2S_LRCK 15
#define I2S_DOUT 2

#define DAC_PIN 25
#define PDM_PIN 25

ESP32Synth synth;

#define dac 0
#define i2s 1
#define pdm 2

// #define OUT_MODE dac
#define OUT_MODE i2s
// #define OUT_MODE pdm

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- ESP32Synth : démarrage du test de stress de polyphonie ---");
    Serial.printf("Puce détectée : %s\n", synth.getChipModel());
    Serial.printf("Limite de voix configurée : %d\n", MAX_VOICES);

    switch(OUT_MODE){
        case dac:
            if (!synth.begin(DAC_PIN)) {
                Serial.println("ERREUR : échec du démarrage du moteur audio !");
                while(1);
            }
        break;

        case i2s:
            if (!synth.begin(I2S_BCLK, I2S_LRCK, I2S_DOUT, I2S_32BIT)) {
                Serial.println("ERREUR : échec du démarrage du moteur audio !");
                while(1);
            }
        break;

        case pdm:
            if (!synth.begin(PDM_PIN)) {
                Serial.println("ERREUR : échec du démarrage du moteur audio !");
                while(1);
            }
        break;
    }
    // ... (la suite du programme déclenche vraisemblablement un grand nombre
    //      de voix, et affiche en série la charge CPU via synth.getCPULoad())
}
```

### Explications
- Ce test est le meilleur point de départ si tu veux **calibrer `MAX_VOICES`** pour ton propre projet : lance-le avec ta configuration matérielle réelle et surveille la sortie série (`getCPULoad()`, cf. §15 du guide API) jusqu'à observer des artefacts (clics, jitter) — la limite juste en dessous de ce seuil est ta valeur de `MAX_VOICES` sûre.
- Le fait de pouvoir basculer entre `dac`/`i2s`/`pdm` par une simple `#define OUT_MODE` en une ligne est une bonne pratique à reprendre dans tes propres sketchs pour tester rapidement plusieurs configurations matérielles sans dupliquer le code.

---

## 3. ESP32SynthMidi (BETA) — synthé MIDI polyphonique piloté par le port série

**Dossier d'origine :** `examples/Fun/ESP32SynthMidi(BETA)/`

### Ce que fait l'exemple
Transforme l'ESP32 en un synthétiseur polyphonique piloté par de simples **commandes texte envoyées sur le port série** — un gabarit pratique pour construire ensuite un vrai contrôleur MIDI complet.

### Format des commandes (traduit)
```
- Note On :   N<note> <vélocité>   (ex. "N60 127")
- Note Off:   F<note>              (ex. "F60")
- Enveloppe : E<a> <d> <s> <r>      (ex. "E10 100 200 300")
- Vibrato :   V<rate> <depth>      (ex. "V500 2500")
- Trémolo :   T<rate> <depth>      (ex. "T200 128")
- Glide :     P<temps_ms>          (ex. "P50")
- Onde :      W<id_onde>           (ex. "W-3" pour SAW)
- Volume :    M<volume_master>     (ex. "M200")
```

### Code traduit (extraits clés)

```cpp
// --- CONFIGURATION ---
// Définit le nombre de voix simultanées.
// IMPORTANT : ceci DOIT être inférieur ou égal à MAX_VOICES dans ESP32Synth.h (défaut 6)
#define POLYPHONY 6

#define BAUD_RATE 115200 // Utilise un débit standard et fiable

ESP32Synth synth;

int voiceNotes[POLYPHONY];     // Stocke le numéro de note MIDI pour chaque voix
uint32_t glideTimeMs = 0;      // Temps de glide/portamento en ms
uint32_t lastFreqCentiHz = 0;  // Dernière fréquence jouée (pour le glide)

char serialBuf[64];
uint8_t bufPos = 0;

/**
 * @brief Convertit un numéro de note MIDI en fréquence en centiHertz.
 * @param midiNote Le numéro de note MIDI (0-127).
 * @return La fréquence en centiHertz (Hz * 100).
 */
uint32_t midiToFreq(uint8_t midiNote) {
  // Formule utilisée : freq = 440 * 2^((note - 69) / 12)
  double freq = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);
  return (uint32_t)(freq * 100.0);
}

void setup() {
    Serial.begin(BAUD_RATE);

    // --- Initialise le synthétiseur ---
    bool success;
    #if USE_I2S
      success = synth.begin(BCK_PIN, WS_PIN, DATA_PIN, I2S_32BIT);
    #else
      #if !defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("!!! ERREUR : DAC interne disponible uniquement sur ESP32 d'origine.");
        while(1) delay(1000);
      #endif
      success = synth.begin(DAC_PIN);
    #endif

    if (!success) {
      Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
      while(1);
    }

    synth.setMasterVolume(200);
    synth.setControlRateHz(500); // Taux de contrôle plus élevé pour des glissandos/LFO plus réactifs

    // Initialise toutes les voix
    // ... (boucle d'initialisation sur POLYPHONY voix)
}
```

### Explications
- La conversion **note MIDI → fréquence** (`midiToFreq`) utilise la formule d'accordage tempéré standard : `f = 440 × 2^((note-69)/12)`, où la note MIDI 69 correspond au La4 (440 Hz) — une formule essentielle à connaître si tu veux brancher n'importe quel contrôleur MIDI sur ESP32Synth.
- `setControlRateHz(500)` augmente le taux de rafraîchissement de la boucle de contrôle (LFO, glissandos, enveloppes) de 100 Hz (défaut) à 500 Hz, ce qui rend les effets nettement plus réactifs et fluides — utile pour un usage "instrument joué en direct" où la latence de contrôle se remarque davantage qu'en musique séquencée.
- Le nom du fichier précise **"(BETA)"** : considère ce gabarit comme un point de départ à adapter, pas comme une implémentation MIDI définitive (par exemple, il ne gère probablement pas encore le vrai protocole MIDI binaire, seulement des commandes texte simplifiées).

---

## 4. Poly12_MIDI_ADSR_LFO_Glide — contribution communautaire (par *fjtrsq*)

**Dossier d'origine :** `examples/Community Examples/Poly12_MIDI_ADSR_LFO_GlidePoly12_MIDI_ADSR_LFO_Glide/`

> Cet exemple provient du dossier communautaire du projet : ["Community Examples"](https://github.com/danilogcrf2-oss/ESP32Synth) accueille les contributions des utilisateurs. Contribué par **fjtrsq**.

### Ce que fait l'exemple
Un synthé MIDI polyphonique à 12 voix avec **vol de voix** (voice stealing), enveloppe ADSR réglable en direct par CC MIDI, LFO de vibrato subtil, et glide/portamento — une implémentation nettement plus aboutie que le gabarit BETA précédent.

### Code traduit (extraits clés)

```cpp
#include <ESP32Synth.h>

/*
  ESP32Synth - Exemple communautaire

  Créé/Contribué par : fjtrsq
  Fork source : https://github.com/fjtrsq
  Description : synthé MIDI polyphonique à 12 voix
*/

ESP32Synth synth;

static constexpr uint8_t VOICE_COUNT = 12;
static constexpr uint8_t MIDI_CHANNEL = 1; // 1..16

struct VoiceSlot {
  bool active = false;
  uint8_t note = 0;
  uint32_t freq = 0;
  uint32_t startedAt = 0;
};

VoiceSlot voiceSlots[VOICE_COUNT];

// Paramètres globaux du synthé, contrôlés par CC MIDI
WaveType selectedWave = WAVE_SAW; // CC 20 : 0-63 SAW, 64-127 PULSE
uint16_t attackMs = 12;
uint16_t decayMs = 120;
uint8_t sustain = 170;
uint16_t releaseMs = 220;
uint32_t glideMs = 0;
uint32_t lfoRateCentiHz = 550;  // 5.50 Hz
uint32_t lfoDepthCentiHz = 12;  // 0.12 Hz (très subtil)
uint16_t noteVolume = 255;

inline uint32_t midiNoteToCentiHz(uint8_t note) {
  const float hz = 440.0f * powf(2.0f, (int(note) - 69) / 12.0f);
  return uint32_t(hz * 100.0f + 0.5f);
}

int8_t findVoiceByNote(uint8_t note) {
  for (uint8_t i = 0; i < VOICE_COUNT; ++i) {
    if (voiceSlots[i].active && voiceSlots[i].note == note) return i;
  }
  return -1;
}

uint8_t allocateVoice() {
  for (uint8_t i = 0; i < VOICE_COUNT; ++i) {
    if (!voiceSlots[i].active) return i;
  }

  // Vol de voix (voice stealing) : on reprend la voix active la plus ancienne
  uint8_t oldest = 0;
  uint32_t oldestTime = voiceSlots[0].startedAt;
  for (uint8_t i = 1; i < VOICE_COUNT; ++i) {
    if (voiceSlots[i].startedAt < oldestTime) {
      oldestTime = voiceSlots[i].startedAt;
      oldest = i;
    }
  }
  return oldest;
}
```

### Explications
- **Le vol de voix (voice stealing)** est une technique indispensable dès qu'on gère un nombre de voix fixe et limité (ici 12) face à un flux MIDI qui peut demander plus de notes simultanées que de voix disponibles : `allocateVoice()` cherche d'abord une voix libre, et si aucune n'est libre, **réquisitionne la voix la plus ancienne** (`startedAt` le plus petit) pour jouer la nouvelle note — c'est exactement ce que fait un synthé matériel réel en cas de dépassement de polyphonie.
- La structure `VoiceSlot` (avec `active`, `note`, `freq`, `startedAt`) est un excellent patron de conception à réutiliser dès que tu veux gérer un pool de voix ESP32Synth depuis une source externe (MIDI, clavier, réseau...) : elle permet de retrouver facilement quelle voix logique correspond à quelle note MIDI (`findVoiceByNote`).
- Les paramètres globaux (`attackMs`, `decayMs`, `sustain`, `releaseMs`, `lfoRateCentiHz`...) modifiables par **Control Change MIDI (CC)** montrent comment brancher n'importe quel contrôleur MIDI (potards, faders) sur les paramètres d'ESP32Synth pour un vrai contrôle expressif en temps réel.
- N'hésite pas à consulter et étudier cet exemple si tu veux construire ton propre synthé MIDI robuste : c'est probablement la base la plus solide du projet pour cet usage.
