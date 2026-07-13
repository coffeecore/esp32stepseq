# 🎛️ ESP32Synth — Référence complète de l'API (français)

Traduit et expliqué à partir du code source (`src/ESP32Synth.h`, `.cpp`, `.hpp`) — version **2.4.3**.

## Sommaire

1. [Vue d'ensemble et philosophie](#1-vue-densemble-et-philosophie)
2. [Installation et configuration (`ESP32Synth_Config.hpp`)](#2-installation-et-configuration)
3. [Initialisation du moteur audio](#3-initialisation-du-moteur-audio)
4. [Contrôle de base des voix](#4-contrôle-de-base-des-voix)
5. [Enveloppe ADSR](#5-enveloppe-adsr)
6. [Contrôle de phase](#6-contrôle-de-phase)
7. [Modulations : vibrato, trémolo, glissandos](#7-modulations--vibrato-trémolo-glissandos)
8. [Arpégiateur](#8-arpégiateur)
9. [Wavetables](#9-wavetables)
10. [Instruments façon "tracker"](#10-instruments-façon-tracker)
11. [Échantillons (samples)](#11-échantillons-samples)
12. [Streaming depuis carte SD](#12-streaming-depuis-carte-sd)
13. [Formes d'onde personnalisées (`WAVE_CUSTOM`)](#13-formes-donde-personnalisées-wave_custom)
14. [Hooks DSP globaux et sortie personnalisée](#14-hooks-dsp-globaux-et-sortie-personnalisée)
15. [Getters et diagnostics](#15-getters-et-diagnostics)
16. [Table des types et énumérations](#16-table-des-types-et-énumérations)
17. [Table des notes (CentiHz)](#17-table-des-notes-centihz)
18. [Visualiser tes ondes en direct sur un écran OLED (mini-oscilloscope)](#18-visualiser-tes-ondes-en-direct-sur-un-écran-oled-mini-oscilloscope)

---

## 1. Vue d'ensemble et philosophie

ESP32Synth n'utilise **aucun `float`** dans son chemin de rendu audio critique. Tout est fait en **arithmétique à virgule fixe** (16.16 ou 32.32), avec des tables de correspondance (LUT) et des décalages de bits (`>>`, `<<`) à la place des divisions matérielles. Ce choix libère énormément de temps processeur, ce qui permet :

- de faire tourner **80 à 500 voix simultanées** selon la puce (ESP32 classique ou S3) ;
- de garder assez de marge CPU pour injecter des blocs DSP complexes (FM 6 opérateurs, modélisation physique de cordes, filtres résonants multi-pôles, etc.) sans faire saturer la tâche audio (FreeRTOS) ni provoquer de redémarrages Watchdog.

**Règle d'or si tu écris du DSP personnalisé** : reste en virgule fixe (`int32_t`/`int64_t`), utilise des LUT et des décalages de bits — évite `float`, les divisions et les branchements coûteux dans la boucle de rendu.

## 2. Installation et configuration

Le fichier `src/ESP32Synth_Config.hpp` centralise toutes les limites de la bibliothèque. Il faut le modifier **avant compilation** (ce ne sont pas des paramètres runtime).

```cpp
#define MAX_VOICES      80   // Nombre max de voix simultanées (limité CPU/RAM)
#define MAX_WAVETABLES  20   // Nombre max de wavetables enregistrées
#define MAX_SAMPLES     20   // Nombre max d'échantillons enregistrés
#define MAX_ARP_NOTES   16   // Nombre max de notes dans un arpège
#define MAX_STREAMS      4   // Nombre max de flux SD simultanés
#define STREAM_BUF_SAMPLES 2048 // Taille du buffer circulaire de streaming (doit être une puissance de 2)
```

### Limites de polyphonie testées

| Puce | RAM-safe | Défaut | Maximum absolu | Point de rupture (jitter FreeRTOS) |
|---|---|---|---|---|
| ESP32 classique | 140 voix | 80 | 340 | 346 |
| ESP32-S3 | 150 voix | 80 | 500 | 515 |

### Taille des buffers DMA (latence vs polyphonie)

```cpp
#define SYNTH_DMA_BUF_LEN   512  // Défaut : haute polyphonie, latence légère
#define SYNTH_DMA_BUF_COUNT 6
```

Formule : `latence (ms) = (buf_len * buf_count) / sample_rate * 1000`

| Profil | LEN | COUNT | Usage |
|---|---|---|---|
| Défaut | 512 | 6 | Haute polyphonie, léger délai |
| Équilibré | 256 | 4 | Bon pour la plupart des claviers MIDI |
| Live | 128 | 2 | Latence quasi imperceptible, un peu moins de voix |

L'auteur ajoute (avec humour) des réglages extrêmes de 64/32/16 échantillons "pour torturer le CPU" — à ne pas prendre au sérieux, ils fonctionnent mais réduisent fortement la polyphonie.

### Mode "RAM minimale" (utile avec LVGL ou d'autres libs gourmandes)

```cpp
#define MAX_VOICES         1
#define MAX_WAVETABLES     1
#define MAX_SAMPLES        1
#define MAX_ARP_NOTES      7
#define MAX_STREAMS        1
#define STREAM_BUF_SAMPLES 2048
```

### PlatformIO (`platformio.ini`)

**Framework Arduino :**
```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
build_flags =
    -O3
    -funroll-loops
```

**Framework ESP-IDF natif :**
```ini
[env:esp32s3-idf]
platform = espressif32
board = esp32-s3-devkitc-1
framework = espidf
monitor_speed = 115200
build_flags =
    -O3
    -funroll-loops
```
La bibliothèque détecte automatiquement le framework actif (`#if defined(ARDUINO)`) et adapte ses appels système (temps, fichiers) en conséquence — le même code `.ino`/`.cpp` compile sous les deux environnements.

---

## 3. Initialisation du moteur audio

```cpp
#include "ESP32Synth.h"
ESP32Synth synth;
```

### Modes de sortie disponibles

| Mode | Description | Contraintes |
|---|---|---|
| `SMODE_I2S` | Sortie I2S standard vers un DAC externe (ex. PCM5102A). Meilleure qualité, CPU le plus faible. | Nécessite plusieurs broches (BCK, WS, DATA), 16 ou 32 bits |
| `SMODE_DAC` | DAC interne (broches GPIO25/26 uniquement) | ESP32 classique seulement, 8 bits, pas disponible sur S3 |
| `SMODE_PDM` | Modulation de densité d'impulsions, flux 16 bits haute fréquence sur une seule broche | Recommandé sur S3 ; déconseillé sur ESP32 classique (bruit) |
| `SMODE_PWM` | PWM matériel (LEDC) piloté par interruption bas niveau, jitter-free, 10 bits @ 48 kHz | Fonctionne sur ESP32 et S3 ; ajouter un filtre RC passif recommandé |
| `SMODE_CUSTOM` | Mode "pull" : aucun timer interne, tu appelles toi-même le rendu (Bluetooth A2DP, WiFi, etc.) | Voir §14 |

### Méthodes `begin()` disponibles (surcharges)

```cpp
void end();                                                                       // Arrête le moteur audio
bool begin(int dacPin);                                                          // DAC interne (ESP32 classique)
bool begin(int bckPin, int wsPin, int dataPin);                                  // I2S 16 bits (raccourci)
bool begin(int bckPin, int wsPin, int dataPin, I2S_Depth i2sDepth);              // I2S avec profondeur choisie
bool begin(int bckPin, int wsPin, int dataPin, int mclkPin, I2S_Depth i2sDepth); // I2S + horloge maître
bool begin(int dataPin, SynthOutputMode mode, int clkPin, int wsPin, I2S_Depth i2sDepth);              // Forme générique
bool begin(int dataPin, SynthOutputMode mode, int clkPin, int wsPin, int mclkPin, I2S_Depth i2sDepth); // Forme générique + mclk
bool beginCustom(uint32_t sampleRate = 48000, SynthCustomOutputCallback customOutput = nullptr); // Mode SMODE_CUSTOM
```

### Exemple d'initialisation

```cpp
#include "ESP32Synth.h"
ESP32Synth synth;

void setup_audio() {
    // Mode I2S standard (DAC externe type PCM5102A - BCK, WS, DATA)
    // Paramètres : dataPin, mode, clkPin, wsPin, Profondeur
    synth.begin(4, 15, 2, I2S_32BIT);

    // Ou : mode PWM matériel sur une seule broche (audio 10 bits sur GPIO 25)
    // synth.begin(25, SMODE_PWM, -1, -1, I2S_16BIT);

    // Ou : mode PDM (audio 1-bit suréchantillonné, haute fréquence, GPIO 2)
    // synth.begin(2, SMODE_PDM, 4, -1, I2S_16BIT);

    // Volume général du moteur (échelle 0-255)
    synth.setMasterVolume(255);
}
```

### Réglages globaux du moteur

```cpp
void setSampleRate(uint32_t rate);      // EXPÉRIMENTAL — à utiliser à tes risques et périls
void setControlRateHz(uint16_t hz);     // Fréquence de la boucle de contrôle (LFO, enveloppes...)
void setMasterVolume(uint16_t volume);  // Volume général (0-255... en interne mis à l'échelle sur 16 bits)
void setVolDepthBase(uint8_t bits);
void setMasterBitcrush(uint8_t bits);   // Résolution artificielle réduite (0-32 bits, 0 = désactivé)
uint16_t getMasterVolume();
const char* getChipModel();             // "ESP32-S3", "ESP32-Standard", ou "Generic ESP32"
int32_t getSampleRate();
```

Exemple bitcrush lo-fi :
```cpp
synth.setMasterBitcrush(8); // Réduction lo-fi en sortie 8 bits
```

---

## 4. Contrôle de base des voix

Chaque « voix » (`voice`, un `uint16_t` de 0 à `MAX_VOICES - 1`) est un canal de synthèse indépendant. La hauteur (pitch) est exprimée en **CentiHz** (Hz × 100), ce qui permet un réglage fin sans passer par des `float`.

```cpp
void noteOn(uint16_t voice, uint32_t freqCentiHz, uint16_t volume);  // Déclenche la voix
void noteOff(uint16_t voice);                                        // Déclenche la phase de relâchement (release)
void setFrequency(uint16_t voice, uint32_t freqCentiHz);             // Change la fréquence à la volée
void setVolume(uint16_t voice, uint16_t volume);
void setWave(uint16_t voice, WaveType type);                         // Sinus / triangle / dent de scie / pulse / bruit / etc.
void setPulseWidthBitDepth(uint8_t bits);
void setPulseWidth(uint16_t voice, uint32_t width);                  // Largeur d'impulsion (0-255)
void setCustomWave(uint16_t voice, SynthCustomWaveCallback cb);       // Voir §13
```

Exemple :
```cpp
// Déclenche la voix 0 sur Do4 (Do médian), volume maximal
synth.noteOn(0, c4, 255);

// Change la hauteur et la largeur d'impulsion en direct
synth.setFrequency(0, cs4);        // Monte vers Do#4
synth.setWave(0, WAVE_PULSE);
synth.setPulseWidth(0, 128);       // Cycle de service ~50%

// Déclenche le relâchement (release) de l'enveloppe
synth.noteOff(0);
```

---

## 5. Enveloppe ADSR

```cpp
void setEnv(uint16_t voice, uint16_t a, uint16_t d, uint8_t s, uint16_t r);
void setSmoothEnv(uint16_t voice, bool enable);
```

- `a` : Attaque (ms)
- `d` : Decay/déclin (ms)
- `s` : Niveau de sustien/maintien (0-255)
- `r` : Release/relâchement (ms)

```cpp
// Attaque 10ms, Decay 150ms, Sustain 120/255, Release 1200ms
synth.setEnv(0, 10, 150, 120, 1200);
```

L'état interne de l'enveloppe est exposé via l'énumération `EnvState` : `ENV_IDLE`, `ENV_ATTACK`, `ENV_DECAY`, `ENV_SUSTAIN`, `ENV_RELEASE` (voir §15, `getEnvState`).

---

## 6. Contrôle de phase

```cpp
void setStartPhase(uint16_t voice, uint16_t phaseDegrees);   // Phase de départ au prochain noteOn (0-359°)
void setCurrentPhase(uint16_t voice, uint16_t phaseDegrees); // Force la phase courante immédiatement
```

Utile pour synchroniser plusieurs voix en phase (ex. supersaw sans battements parasites au déclenchement) ou pour des effets de synchronisation matérielle (hard sync).

---

## 7. Modulations : vibrato, trémolo, glissandos

```cpp
// Vibrato = modulation de fréquence (LFO appliqué au pitch)
void setVibrato(uint16_t voice, uint32_t rateCentiHz, uint32_t depthCentiHz);
void setVibratoPhase(uint16_t voice, uint16_t phaseDegrees);

// Trémolo = modulation d'amplitude (LFO appliqué au volume)
void setTremolo(uint16_t voice, uint32_t rateCentiHz, uint16_t depth);
void setTremoloPhase(uint16_t voice, uint16_t phaseDegrees);

// Glissando de fréquence (portamento) — algorithme de Bresenham (pas de division au taux de contrôle)
void slideFreq(uint16_t voice, uint32_t startFreqCentiHz, uint32_t endFreqCentiHz, uint32_t durationMs);
void slideFreqTo(uint16_t voice, uint32_t endFreqCentiHz, uint32_t durationMs);

// Glissando de volume
void slideVol(uint16_t voice, uint16_t startVol, uint16_t endVol, uint32_t durationMs);
void slideVolTo(uint16_t voice, uint16_t endVol, uint32_t durationMs);
```

Exemples :
```cpp
// Vibrato : LFO à 6.5 Hz (650 cHz), profondeur 30 Hz (3000 cHz)
synth.setVibrato(0, 650, 3000);

// Trémolo : LFO à 4 Hz (400 cHz), profondeur 80 (sur 255)
synth.setTremolo(0, 400, 80);

// Glisse vers Do5 en exactement 500 ms
synth.slideFreqTo(0, c5, 500);
```

> `slideFreq()` prépare le glissando ; c'est `noteOn()` qui le démarre réellement. Attention à ce que la fréquence donnée à `noteOn()` corresponde à la fréquence de départ du glissando.

---

## 8. Arpégiateur

```cpp
template <typename... Args>
void setArpeggio(uint16_t voice, uint16_t durationMs, Args... freqs); // Notes variadiques (jusqu'à MAX_ARP_NOTES)
void detachArpeggio(uint16_t voice);
```

```cpp
// Voix 0, durée de chaque pas : 120ms, notes : Do4, Mi4, Sol4, Do5
synth.setArpeggio(0, 120, c4, e4, g4, c5);
```

---

## 9. Wavetables

Une wavetable est un tableau de valeurs (4, 8 ou 16 bits) décrivant **un seul cycle** d'une forme d'onde, rejoué en boucle. Contrairement aux échantillons (§11), **il n'y a pas de fréquence de référence (root pitch)** : une wavetable est un oscillateur comme les autres, elle utilise le même `phaseInc` calculé directement à partir de la fréquence demandée à `noteOn()`. Concrètement (extrait réel du rendu, `ESP32Synth_Renders.hpp`) :

```cpp
// L'index dans la table est dérivé de la phase courante (32 bits), pas d'un ratio de fréquence
uint32_t idx = ((ph >> 16) * size) >> 16;   // ph avance de "inc" (= phaseInc) à chaque échantillon
mixBuffer[i] += (data[idx] * finalVol) >> 16;
```

Autrement dit, **un cycle complet de la wavetable = une période exacte de la note jouée**, quelle que soit la taille de la table (256, 512, 2048 points...). C'est la même logique de phase que pour `WAVE_SINE`/`WAVE_SAW`/etc. — la seule différence est que la forme lue vient de ton tableau plutôt que d'une table interne figée.

### Format des données selon `BitDepth`

| Profondeur | Stockage | Décodage interne |
|---|---|---|
| `BITS_16` | `int16_t`, signé, plein 16 bits | Utilisé tel quel |
| `BITS_8` | `uint8_t`, **non signé**, silence = 128 | `(valeur - 128) << 8` avant mixage |
| `BITS_4` | `uint8_t`, **2 échantillons par octet** (nibbles), silence = 8 | `((nibble - 8) * 4096)` avant mixage |

Ce dernier point est important si tu génères tes propres wavetables à la main : en 8 bits, `128` est le zéro (silence), pas `0` ; en 4 bits, deux valeurs sont compressées par octet (`data[idx >> 1] >> ((idx & 1) << 2) & 0x0F`), avec `8` comme zéro.

```cpp
void setWavetable(uint16_t voice, const void* data, uint32_t size, BitDepth depth);      // Attribue directement à une voix
void registerWavetable(uint16_t id, const void* data, uint32_t size, BitDepth depth);    // Enregistre sous un ID réutilisable
```

`registerWavetable()` remplit juste une table interne (`wavetables[id]`) consultable ensuite par ID — utilisée notamment par les `Instrument` du §10 (qui référencent des wavetables par ID plutôt que par pointeur). `setWavetable()`, elle, colle directement un pointeur de données sur une voix, sans passer par un ID.

```cpp
synth.registerWavetable(BUZZY_WAVE_ID, buzzy_wavetable, 256, BITS_8);
synth.setWave(voice, WAVE_WAVETABLE);
synth.setWavetable(voice, buzzy_wavetable, 256, BITS_8);
```

L'outil Python `tools/Wavetables/WavetableMaker.py` permet de générer des wavetables (`.h`) à partir d'équations mathématiques ou de segments d'onde.

> Sur ESP32-S3, remarque dans le code source que le rendu de wavetable est vectorisé en SIMD (traitement de 4 échantillons à la fois via des types `v4i32`/`v4u32`) — un gain de performance qui explique en partie pourquoi le S3 supporte davantage de voix simultanées que l'ESP32 classique (cf. tableau de polyphonie au §2).

---

## 10. Instruments façon "tracker"

Une `Instrument` décrit une **séquence de pas** (volume + forme d'onde) rejouée automatiquement pendant que la note est tenue, façon instruments de trackers (ProTracker, FastTracker...). C'est un mécanisme entièrement séparé de l'enveloppe ADSR classique — les deux ne se combinent pas comme on pourrait le croire (détail important expliqué plus bas).

```cpp
struct Instrument {
    const uint8_t* seqVolumes;   // Séquence de volumes pour la phase "attaque"
    const int16_t* seqWaves;     // Séquence de formes d'onde pour la phase "attaque"
    const uint8_t* relVolumes;   // Séquence de volumes pour la phase "relâchement"
    const int16_t* relWaves;     // Séquence de formes d'onde pour la phase "relâchement"
    uint16_t       seqSpeedMs;   // Durée de chaque pas de la séquence d'attaque (ms)
    int16_t        susWave;      // Forme d'onde tenue pendant le sustain
    uint16_t       relSpeedMs;   // Durée de chaque pas de la séquence de relâchement (ms)
    uint8_t        seqLen;       // Nombre de pas dans la séquence d'attaque
    uint8_t        susVol;       // Volume tenu pendant le sustain (0-255)
    uint8_t        relLen;       // Nombre de pas dans la séquence de relâchement
    bool           smoothMorph;  // Présent dans la structure, mais non exploité par le moteur actuel (voir note ci-dessous)
    bool           smoothVolume; // Interpole le volume en douceur entre deux pas (voir détail ci-dessous)
};
```

### 10.1 `seqWaves` : valeurs négatives = onde de base, valeurs positives = ID de wavetable

C'est une astuce d'encodage à connaître : le type de `seqWaves`/`relWaves`/`susWave` est `int16_t`, et le moteur interprète le **signe** de chaque valeur pour savoir quoi faire (logique réelle extraite de `processControl()`) :
```cpp
if (wVal < 0) {
    // Valeur négative = une onde de base, casté directement en WaveType
    // (rappel : WAVE_SINE=-1, WAVE_TRIANGLE=-2, WAVE_SAW=-3, WAVE_PULSE=-4, WAVE_NOISE=-5)
    vo->currWaveType = (WaveType)wVal;
} else {
    // Valeur positive = un ID de wavetable enregistrée via registerWavetable()
    vo->currWaveId = (uint16_t)wVal;
    vo->wtData = wavetables[vo->currWaveId].data;
    vo->wtSize = wavetables[vo->currWaveId].size;
    vo->depth  = wavetables[vo->currWaveId].depth;
}
```
Ça veut dire qu'un même `Instrument` peut mélanger, pas par pas, des ondes de base (`WAVE_SINE`, etc.) et des wavetables personnalisées, simplement en jouant sur le signe des valeurs de la séquence — pas besoin que tout soit des wavetables enregistrées.

### 10.2 Le déroulement réel des 3 phases

Contrairement à une enveloppe ADSR classique (Attaque/Decay/Sustain/Release), un `Instrument` ne connaît que **3 phases**, pilotées par la machine à états `envState` — mais attention, ici cette variable ne représente **pas** un niveau d'enveloppe qui monte/descend, elle indique juste **quelle séquence est en train de jouer** :

- **`ENV_ATTACK`** : déroule `seqVolumes[]`/`seqWaves[]`, un pas toutes les `seqSpeedMs` millisecondes, jusqu'à épuisement de `seqLen` pas. Passe ensuite automatiquement en sustain.
- **`ENV_SUSTAIN`** : reste bloqué sur `susVol`/`susWave` tant que la note est tenue (jusqu'au prochain `noteOff()`).
- **`ENV_RELEASE`** (déclenché par `noteOff()`) : déroule `relVolumes[]`/`relWaves[]`, un pas toutes les `relSpeedMs` ms, jusqu'à épuisement de `relLen` pas — puis la voix se désactive (`vo->active = false`). Si `relLen == 0`, la voix se coupe **immédiatement** au `noteOff()`, sans fondu.

### 10.3 ⚠️ Piège important : `setEnv()` est ignoré sur une voix pilotée par un `Instrument`

En creusant `updateAdsrBlock()` (la fonction qui calcule l'enveloppe ADSR à chaque bloc audio), on trouve ceci en tout premier :
```cpp
static FORCE_INLINE IRAM_ATTR void updateAdsrBlock(Voice* vo, int samples, int32_t& startEnv, int32_t& envStep) {
    if (vo->inst) {
        startEnv       = ENV_MAX;  // Force le volume d'enveloppe au maximum...
        vo->currEnvVal = ENV_MAX;
        envStep        = 0;        // ...et ne bouge plus jamais tout seul.
        return;
    }
    // ... (sinon, calcul ADSR classique basé sur setEnv())
```
**Dès qu'une voix a un `Instrument` attaché (`setInstrument(voice, &monInstrument)`), l'enveloppe ADSR classique (`setEnv()`) est totalement court-circuitée** : le "volume d'enveloppe" reste bloqué à sa valeur maximale en permanence. Tout le contrôle du volume dans le temps passe alors **exclusivement** par les séquences `seqVolumes`/`susVol`/`relVolumes` de l'instrument lui-même. Si tu appelles `setEnv()` sur une voix qui a (ou aura) un `Instrument`, cet appel n'aura donc **aucun effet audible** tant que l'instrument reste attaché — un piège assez facile à rencontrer en pratique.

### 10.4 `smoothVolume` vs `smoothMorph`

- **`smoothVolume = true`** : quand un pas de séquence change de volume, le moteur appelle en interne `slideVolAbsolute()` pour glisser en douceur de l'ancien volume vers le nouveau, sur la durée du pas (`seqSpeedMs`/`relSpeedMs`), plutôt que de sauter brutalement — ce champ **est** activement utilisé (voir `if (inst->smoothVolume && ms > 0) slideVolAbsolute(...)` dans `processControl()`).
- **`smoothMorph`** : ce champ existe dans la structure `Instrument`, mais **n'est référencé nulle part ailleurs dans le code source actuel** (`grep` ne le retrouve que dans la déclaration du `struct`, jamais lu ni utilisé dans le moteur de rendu). En clair : **actuellement, changer de wavetable d'un pas à l'autre se fait par un saut net, jamais par un morphing/crossfade**, quelle que soit la valeur de `smoothMorph` que tu donnes. Si tu veux un effet de morphing progressif entre timbres, la technique qui fonctionne réellement est celle vue dans `RT_FM_on_Wavetables` (§8 des exemples) : précalculer toi-même une série de wavetables intermédiaires et les enchaîner comme des pas de séquence rapprochés.

### 10.5 Attacher un instrument

```cpp
void setInstrument(uint16_t voice, Instrument* inst);
void setInstrument(uint16_t voice, Instrument_Sample* inst); // Voir §11 pour la variante samples multizones
void detachInstrument(uint16_t voice, WaveType newWaveType);
```

`detachInstrument(voice, newWaveType)` fait exactement `setInstrument(voice, (Instrument*)nullptr)` puis `setWave(voice, newWaveType)` — ça détache l'instrument (et donc réactive l'ADSR classique via `setEnv()`) et repasse la voix sur une onde de base standard en une seule ligne.

---

## 11. Échantillons (samples)

Cette section a été creusée directement dans l'implémentation (`ESP32Synth_Core.hpp`, `noteOn()`, et le moteur de rendu `ESP32Synth_Renders.hpp`), pour expliquer précisément ce qui se passe en interne — pas seulement recopier les signatures.

### 11.1 Le principe général : un « sample » = un enregistrement PCM brut + une hauteur de référence

Un échantillon (sample) est un enregistrement audio (typiquement un `.wav` converti en tableau C) joué à une vitesse variable pour changer sa hauteur perçue — exactement comme une bande magnétique ou un sampler matériel classique (E-mu, Akai...). Il n'y a **pas de pitch-shifting "intelligent"** (pas de vocoder ni d'étirement temporel) : jouer plus aigu = lire le buffer plus vite (et donc plus court), jouer plus grave = le lire plus lentement (et donc plus long). C'est pour ça que chaque sample a besoin d'une **fréquence de référence** (`rootFreqCentiHz`) : c'est la hauteur à laquelle il doit être lu à vitesse normale (×1.0).

```cpp
bool registerSample(uint16_t sampleId, const void* data, uint32_t length,
                     uint32_t sampleRate, uint32_t rootFreqCentiHz, BitDepth depth = BITS_16);
```

`registerSample()` ne fait qu'**enregistrer les métadonnées** d'un buffer déjà existant en mémoire (Flash ou RAM) sous un identifiant (`sampleId`, jusqu'à `MAX_SAMPLES`) :
- `data` : pointeur vers les données PCM brutes (4, 8 ou 16 bits selon `depth`) ;
- `length` : nombre d'échantillons (pas d'octets) ;
- `sampleRate` : fréquence d'échantillonnage native de l'enregistrement (ex. 44100) ;
- `rootFreqCentiHz` : la note à laquelle ce sample doit être joué sans transposition (ex. `c4` si tu as enregistré un Do4) ;
- `depth` : `BITS_4`, `BITS_8` ou `BITS_16` (impacte la taille mémoire, voir `Low_BitDepth` dans les exemples).

Cette fonction ne touche à aucune voix : elle remplit juste une table interne `registeredSamples[sampleId]` consultable ensuite par n'importe quelle voix.

### 11.2 Deux façons d'attacher un sample à une voix — et pourquoi elles existent toutes les deux

C'est le point que tu soulevais : `setSample()` et `setInstrument(Instrument_Sample*)` font des choses **différentes**, même si elles se ressemblent en surface.

#### A) `setSample()` — un sample fixe, assigné manuellement

```cpp
void setSample(uint16_t voice, uint16_t sampleId, LoopMode loopMode = LOOP_OFF,
               uint32_t loopStart = 0, uint32_t loopEnd = 0);
```

Regarde ce que fait réellement cette fonction en interne :
```cpp
void ESP32Synth::setSample(uint16_t voice, uint16_t sampleId, LoopMode loopMode, uint32_t loopStart, uint32_t loopEnd) {
    if (voice >= MAX_VOICES || sampleId >= MAX_SAMPLES) return;
    Voice* v = &voices[voice];
    v->type            = WAVE_SAMPLE;
    v->curSampleId     = sampleId;      // <-- UN SEUL sample fixé pour cette voix
    v->sampleLoopMode  = loopMode;
    v->sampleLoopStart = loopStart;
    v->sampleLoopEnd   = loopEnd;
    v->instSample      = nullptr;       // <-- Détache tout Instrument_Sample existant
}
```
`setSample(voice, sampleId, ...)` colle **un seul et même sample** (`sampleId`) sur cette voix, une fois pour toutes. Peu importe la note que tu joueras ensuite avec `noteOn(voice, freq, vol)` : c'est **toujours ce même enregistrement** qui sera lu, simplement transposé selon le rapport entre `freq` et le `rootFreqCentiHz` enregistré pour ce sample :
```cpp
// Dans noteOn(), cas WAVE_SAMPLE simple :
const SampleData* sData = &registeredSamples[vo->curSampleId];
uint64_t ratio1616 = ((uint64_t)freqCentiHz << 16) / sData->rootFreqCentiHz;
vo->sampleInc1616  = (uint32_t)((ratio1616 * sData->sampleRate) / _sampleRate);
```
Concrètement : si tu enregistres `c4` comme racine et que tu appelles `noteOn(voice, c5, 255)` (une octave au-dessus), le sample sera lu **deux fois plus vite** (donc deux fois plus court et une octave plus aigu) — comme un sampler à une seule cellule.

**Usage typique** : une boucle de batterie, un effet sonore, un sample unique qu'on veut simplement pouvoir retransposer légèrement (`Amen_Break_Loop`, `Low_BitDepth`).

#### B) `setInstrument(voice, Instrument_Sample*)` — plusieurs samples répartis sur le clavier (multisample / key split)

```cpp
void setInstrument(uint16_t voice, Instrument_Sample* inst);
```

Ici, on n'attache pas un sample mais une **carte de répartition** (comme un patch de sampler/soundfont) qui dit : « pour telle plage de fréquences (telle zone du clavier), utilise tel sample enregistré, avec éventuellement telle racine de substitution ». C'est ce que fait la structure `SampleZone` :

```cpp
struct SampleZone {
    uint32_t lowFreq;       // Borne basse de la zone, en CentiHz
    uint32_t highFreq;      // Borne haute de la zone, en CentiHz
    uint16_t sampleId;      // Quel sample enregistré (via registerSample) utiliser dans cette zone
    uint32_t rootOverride;  // Root pitch de remplacement pour cette zone (0 = utiliser celui du sample lui-même)
};

struct Instrument_Sample {
    const SampleZone* zones;
    uint8_t           numZones;
    LoopMode          loopMode;   // Appliqué à toutes les zones de cet instrument
    uint32_t          loopStart;
    uint32_t          loopEnd;    // 0 = jusqu'à la fin du sample sélectionné
};
```

Et voici ce qui se passe **à chaque `noteOn()`** quand une voix a un `Instrument_Sample` attaché (extrait réel de `noteOn()`) :
```cpp
// Cherche la zone couvrant la fréquence demandée
for (int i = 0; i < vo->instSample->numZones; i++) {
    const SampleZone* z = &vo->instSample->zones[i];
    if (freqCentiHz >= z->lowFreq && freqCentiHz <= z->highFreq) {
        sData           = &registeredSamples[z->sampleId];
        vo->curSampleId = z->sampleId;
        root = (z->rootOverride > 0) ? z->rootOverride : sData->rootFreqCentiHz;
        break; // première zone correspondante = celle utilisée
    }
}
```
Autrement dit : **le sample réellement joué est choisi dynamiquement à chaque `noteOn()`, en fonction de la note demandée**, en parcourant les zones dans l'ordre jusqu'à trouver celle qui couvre `freqCentiHz`. C'est exactement le principe d'un instrument multi-échantillonné (« multisample ») d'un vrai sampler/soundfont : par exemple, tu peux enregistrer 3 samples de piano à différentes octaves et définir 3 zones, pour que chaque octave utilise l'enregistrement le plus proche (meilleur réalisme qu'une seule cellule retransposée sur toute la tessiture).

Dans l'exemple `Amen_Break_Loop` du projet, l'instrument ne définit **qu'une seule zone** couvrant tout le clavier (`{ c0, g10, 0, c4 }` = de `c0` à `g10`, sample `0`, racine forcée à `c4`) — c'est donc un cas particulier (une seule cellule) de ce mécanisme plus général, ce qui explique la ressemblance apparente avec `setSample()`.

**Usage typique** : instruments réalistes multi-échantillonnés (piano, orgue à tuyaux samplé par note, batterie multi-samples par vélocité...), là où `setSample()` suffit pour un sample unique/effet ponctuel.

> **Pourquoi découper en zones plutôt qu'utiliser un seul sample transposé partout ?** Ce n'est **pas** une histoire de RAM (chaque `SampleZone`/`SampleData` supplémentaire ne coûte que quelques octets de métadonnées — le vrai coût mémoire supplémentaire, le cas échéant, serait plutôt en **Flash**, pour stocker plusieurs enregistrements). La vraie raison est que le moteur ne fait **aucune correction de formants** lors de la transposition : jouer plus aigu/plus grave qu'un sample n'est qu'une simple variation de vitesse de lecture (`sampleInc1616`), sans intelligence (pas de pitch-shift façon vocodeur). Plus tu t'éloignes de `rootFreqCentiHz`, plus l'effet devient audible et artificiel (façon "chipmunk" en trop aigu, façon "monstre ralenti" en trop grave). Le key split sert justement à **minimiser cette distance de transposition** : chaque zone fait jouer le sample enregistré le plus proche possible de la note demandée, pour un rendu beaucoup plus fidèle au timbre réel de l'instrument à chaque registre.

#### En résumé

| | `setSample()` | `setInstrument(Instrument_Sample*)` |
|---|---|---|
| Nombre de samples utilisables | 1 seul, fixe | Plusieurs, choisis dynamiquement selon la note |
| Sélection du sample | Manuelle, une fois pour toutes | Automatique à chaque `noteOn()`, via les `SampleZone` |
| Racine de hauteur | Celle enregistrée dans `registerSample()` | Celle du sample, ou `rootOverride` par zone |
| Cas d'usage | Effet, boucle, sample unique retransposable | Instrument réaliste multi-échantillonné |

Les deux mécanismes utilisent le **même moteur de lecture/boucle** ensuite (mêmes champs internes `samplePos1616`, `sampleInc1616`, mêmes modes de boucle) — seule la façon dont `curSampleId` est déterminé diffère.

> Remarque : appeler `setSample()` sur une voix efface automatiquement son éventuel `Instrument_Sample` (`v->instSample = nullptr`), et inversement `setInstrument(voice, Instrument_Sample*)` prend le pas dessus. Les deux ne peuvent pas être actifs en même temps sur une même voix.

### 11.3 Modes de boucle (`LoopMode`) — comportement exact en lecture

```cpp
enum LoopMode : uint8_t { LOOP_OFF, LOOP_FORWARD, LOOP_PINGPONG, LOOP_REVERSE };
```

Le rendu maintient une position de lecture en 16.16 (`samplePos1616`) qui avance ou recule de `sampleInc1616` à chaque échantillon, entre les bornes `sampleLoopStart` (`lStart`) et `sampleLoopEnd` (`lEnd`, ou la fin du sample si `loopEnd = 0`). Voici la logique exacte à chaque franchissement de borne (extrait de `ESP32Synth_Renders.hpp`) :

- **`LOOP_OFF`** : dès que la lecture atteint `lEnd` (en lecture avant) ou `lStart` (en lecture arrière), la voix marque `sampleFinished = true` et s'arrête net — pas de bouclage, le sample est joué une seule fois.
- **`LOOP_FORWARD`** : en atteignant `lEnd`, la position revient instantanément à `lStart` (`pos -= (lEnd - lStart)`) et continue d'avancer — un bouclage classique en boucle avant, sans changement de sens.
- **`LOOP_PINGPONG`** : en atteignant `lEnd`, la lecture **inverse son sens** et repart en arrière (`dir = false`) ; en atteignant ensuite `lStart`, elle repart en avant (`dir = true`) — un aller-retour continu, ce qui évite le "clic" qu'on entend parfois avec `LOOP_FORWARD` si le point de fin ne se raccorde pas parfaitement au point de début.
- **`LOOP_REVERSE`** : la voix démarre en lecture **arrière** dès le début (`sampleDirection = (loopMode != LOOP_REVERSE)`, donc `false` ici) ; en atteignant `lStart`, elle boucle en repartant de `lEnd`, mais toujours en lisant à l'envers — un bouclage arrière perpétuel.

`setSampleLoop(voice, loopMode, loopStart, loopEnd)` permet de changer ces réglages **sans redéclencher la note** (contrairement à `setSample()` qui redéfinit tout et coupe l'`Instrument_Sample` éventuellement attaché) — utile par exemple pour modifier dynamiquement les points de boucle d'un sample en cours de lecture.

### 11.4 Position de départ dans le sample (`setStartPhase`)

Un détail utile : `setStartPhase(voice, phaseDegrees)` (§6) ne sert pas qu'aux oscillateurs — pour une voix de type sample, elle définit **un décalage de départ en pourcentage du sample** (`phaseDegrees` sur 360° = 0 à 100% de la longueur), via `startOffset = (sData->length * vo->startPhase) / 360`. Ça permet par exemple de démarrer la lecture d'un sample au milieu plutôt qu'au tout début.

### 11.5 Exemple complet commenté

```cpp
// 1. Enregistre le sample brut sous l'ID 0, avec Do4 comme hauteur de référence
synth.registerSample(0, amen_break_data, amen_break_len, amen_break_rate, c4);

// 2a. Option simple : coller ce sample fixe sur la voix 0, en boucle avant complète
synth.setSample(0, 0, LOOP_FORWARD, 0, 0);

// 2b. Option "instrument" : la même chose mais via une zone (permet d'en ajouter d'autres plus tard)
const SampleZone zones[] = { { c0, g10, /*sampleId=*/0, /*rootOverride=*/c4 } };
Instrument_Sample instAmen = { zones, 1, LOOP_FORWARD, 0, 0 };
synth.setInstrument(0, &instAmen);

synth.setEnv(0, 0, 0, 255, 0); // Pas d'enveloppe : volume plein instantané
synth.noteOn(0, c4, 255);      // Joue à la hauteur de référence -> vitesse native (x1.0)
synth.noteOn(0, c5, 255);      // Jouerait la même donnée 2x plus vite (une octave au-dessus)
```

L'outil Python `tools/Samples/WavToEsp32SynthConverter.py` convertit des fichiers audio courts en tableaux C statiques 4/8/16 bits (exploitables directement par `registerSample`), pour éviter d'avoir besoin d'une carte SD pour des instruments transitoires (percussions, etc.).

---

## 12. Streaming depuis carte SD

Le streaming permet de lire de longs fichiers `.wav` directement depuis la carte SD, sans les charger entièrement en RAM.

### 12.1 Architecture réelle : tâche productrice + buffer circulaire + voix consommatrice

En creusant `ESP32Synth_SDStream.hpp` et `sdLoaderTask()`, voici précisément ce qui se passe :

1. **`setupStream()`** (appelé automatiquement par `playStream()`, ou directement si tu veux préparer un flux sans le démarrer) :
   - Démarre, **à la demande et une seule fois**, une tâche FreeRTOS dédiée (`sdLoaderTask`, épinglée sur `SYNTH_SD_TASK_CORE` = Core 0 par défaut) — elle ne redémarre pas à chaque appel.
   - Cherche un slot libre parmi `MAX_STREAMS` (§2), ouvre le fichier, **parse l'en-tête WAV** (`parseWavHeader()`) pour en extraire la fréquence d'échantillonnage native, la position/taille des données audio, le nombre de canaux et la profondeur en bits — puis positionne le curseur de lecture au début des données audio (après l'en-tête).
   - Attache ce `StreamTrack` à la voix (`voice.streamTrackId`) et met la voix en `WAVE_STREAM`, mais **inactive** (`vo->active = false`) : la lecture n'a pas encore démarré.

2. **`playStream()`** appelle `setupStream()` en interne, puis :
   - Attend jusqu'à 100 ms (par pas de 1 ms) que le **buffer circulaire commence à se remplir** (`while (trk->head == trk->tail && timeout > 0) vTaskDelay(...)`) — un petit pré-chargement pour éviter un silence ou un artefact au tout premier instant de lecture.
   - Calcule le ratio de rééchantillonnage exactement **comme pour un sample classique** (§11) : `ratio1616 = (freqDemandée << 16) / rootFreqCentiHz`, puis `sampleInc1616 = ratio1616 * sampleRateDuFichier / sampleRateDuSynth`. **Le fichier WAV a donc lui aussi une "hauteur de référence" (`rootFreqCentiHz`, paramètre par défaut `26163` = `c4`)** : jouer une fréquence différente à `noteOn()`/en argument accélère ou ralentit la lecture du fichier, exactement comme un sample — ce n'est pas fait pour être transposé (c'est de la musique de fond), mais le mécanisme est disponible si tu veux un effet "33/45 tours".
   - Démarre l'enveloppe ADSR normalement (contrairement aux voix pilotées par un `Instrument`, une voix `WAVE_STREAM` respecte bien `setEnv()`).

3. **En tâche de fond (`sdLoaderTask`, Core 0)** : un buffer circulaire de taille `STREAM_BUF_SAMPLES` (2048 par défaut, doit être une puissance de 2 — voir `STREAM_BUF_MASK` utilisé pour un wraparound par simple masque binaire plutôt qu'un modulo) est continuellement rempli à partir du fichier, tant qu'il reste de la place libre (`freeSpace = (STREAM_BUF_SAMPLES + tail - head - 1) & STREAM_BUF_MASK`). C'est un classique **producteur/consommateur** : la tâche SD écrit à l'index `head`, le moteur de rendu audio (Core 1) lit et avance l'index `tail` — aucun verrou (mutex) n'est nécessaire tant que chaque cœur ne touche qu'à "son" index.

Cette architecture garantit que les accès SD (lents, potentiellement plusieurs millisecondes de latence) **ne bloquent jamais le rendu audio** : le Core 1 lit toujours depuis la RAM (le buffer circulaire), jamais directement depuis la carte SD.

### 12.2 Signatures

```cpp
// Sous Arduino (fs::FS)
int8_t setupStream(uint16_t voice, fs::FS &fs, const char* path, uint32_t rootFreqCentiHz = 26163, bool loop = false);
int8_t playStream(uint16_t voice, fs::FS &fs, const char* path, uint16_t volume = 255, uint32_t rootFreqCentiHz = 26163, bool loop = false);

// Sous ESP-IDF natif (chemins POSIX / VFS)
int8_t setupStream(uint16_t voice, const char* path, uint32_t rootFreqCentiHz = 26163, bool loop = false);
int8_t playStream(uint16_t voice, const char* path, uint16_t volume = 255, uint32_t rootFreqCentiHz = 26163, bool loop = false);
```

Les deux retournent l'**ID interne du flux** (0 à `MAX_STREAMS-1`), ou `-1` en cas d'échec (fichier introuvable, en-tête WAV invalide, ou plus aucun slot de flux disponible).

### 12.3 Contrôle de la lecture — comportement exact

```cpp
void     pauseStream(uint16_t voice);   // trk->playing = false : la tâche SD arrête juste de remplir le buffer, le fichier reste ouvert
void     resumeStream(uint16_t voice);  // trk->playing = true  : reprise immédiate, sans recharger le fichier
void     stopStream(uint16_t voice);    // Ferme le fichier et libère le slot de flux — il faudra rappeler playStream() pour rejouer
void     seekStreamMs(uint16_t voice, uint32_t ms);
void     setStreamLoopPointsMs(uint16_t voice, uint32_t startMs, uint32_t endMs);
uint32_t getStreamPositionMs(uint16_t voice);
uint32_t getStreamDurationMs(uint16_t voice);
bool     isStreamPlaying(uint16_t voice);
```

- **`seekStreamMs(voice, ms)`** ne déplace pas directement le curseur : il pose une "cible" (`trk->seekTarget`) que la tâche SD ira traiter à son prochain passage — la recherche (seek) reste donc asynchrone et ne bloque jamais l'appelant.
- **`setStreamLoopPointsMs(voice, startMs, endMs)`** convertit les millisecondes en position **exacte en octets** dans le fichier, en tenant compte du nombre de canaux et de la profondeur en bits du WAV (`bytesPerSample = (bitsPerSample/8) * numChannels`), puis **aligne ces positions sur une frontière de trame audio complète** (`startBytes -= (startBytes - dataStartPos) % bytesPerSample`) — ce qui évite de couper un échantillon stéréo en plein milieu (ce qui produirait un "clic" à chaque bouclage). Si `endMs = 0`, la boucle va jusqu'à la toute fin du fichier.
- **`getStreamPositionMs()`/`getStreamDurationMs()`** sont calculés respectivement à partir du nombre d'échantillons déjà joués (`trk->samplesPlayed`) et de la taille totale des données audio (`trk->dataSize`), tous deux divisés par la fréquence d'échantillonnage native du fichier — indépendamment de la fréquence de lecture demandée (donc même si tu "transposes" la lecture, ces valeurs restent calées sur le temps réel du fichier d'origine, pas sur le temps perçu à l'oreille).

### 12.4 Exemple annoté

```cpp
#ifdef ARDUINO
#include <SD.h>
#include <SPI.h>

void jouer_musique_de_fond() {
    // Voix, périphérique FS, chemin, volume, fréquence de base, boucle
    // c4 (26163) = valeur par défaut : le fichier sera lu à sa vitesse native.
    synth.playStream(1, SD, "/musique_ambiance.wav", 255, c4, true);

    // Boucle uniquement le segment entre 2s et 24s (repositionnement automatique
    // et sans clic, aligné sur une frontière de trame audio)
    synth.setStreamLoopPointsMs(1, 2000, 24000);
}
#endif
```

```cpp
#ifndef ARDUINO
void jouer_musique_de_fond_idf() {
    // ESP-IDF abstrait le système de fichiers en POSIX : on passe le chemin direct
    synth.playStream(1, "/sdcard/musique_ambiance.wav", 255, c4, true);
}
#endif
```

---

## 13. Formes d'onde personnalisées (`WAVE_CUSTOM`)

C'est le mécanisme le plus puissant de la bibliothèque : tu écris **ta propre fonction de synthèse**, appelée directement dans la boucle de rendu audio (donc en `IRAM_ATTR`, très optimisée).

### Signature du callback

```cpp
typedef void (*SynthCustomWaveCallback)(Voice* vo, int32_t* mixBuffer, int samples,
                                         int32_t startEnv, int32_t envStep);
void setCustomWave(uint16_t voice, SynthCustomWaveCallback cb);
```

- `vo` : pointeur vers la structure `Voice` (accès à la phase, l'incrément de phase, le volume, et surtout **6 registres génériques `cw[6]`** que tu peux utiliser librement comme mémoire d'état pour ton algorithme — oscillateur modulateur, ligne à retard, etc.) ;
- `mixBuffer` : buffer de mixage 32 bits sur lequel tu dois **ajouter** (`+=`) ton signal ;
- `samples` : nombre d'échantillons à générer sur cet appel ;
- `startEnv` / `envStep` : valeur de départ et incrément de l'enveloppe ADSR pour ce bloc (à appliquer toi-même à ton signal).

### Exemple : oscillateur FM 2 opérateurs

```cpp
void IRAM_ATTR fmTwoOpOscillator(Voice* vo, int32_t* mixBuffer, int samples, int32_t startEnv, int32_t envStep) {
    int32_t currentEnv = startEnv;
    int32_t volBase = ((uint32_t)vo->vol * vo->trmModGain) >> 8;

    uint32_t carrierPhase = vo->phase;
    uint32_t carrierInc = vo->phaseInc + vo->vibOffset;

    // Le modulateur tourne à 2x la fréquence de la porteuse (relation harmonique simple)
    uint32_t modulatorPhase = vo->cw[0];
    uint32_t modulatorInc = carrierInc * 2;
    int16_t prevSample = (int16_t)vo->cw[1]; // Stockage du feedback

    for (int i = 0; i < samples; i++) {
        // Le modulateur produit un sinus avec feedback de phase (~12.5%)
        uint32_t feedbackPhase = modulatorPhase + (prevSample << 12);
        int32_t modSample = sineLUT[feedbackPhase >> SINE_SHIFT];
        prevSample = (int16_t)modSample;

        // Module la phase de la porteuse avec la sortie du modulateur
        uint32_t finalCarrierPhase = carrierPhase + (modSample * 16); // indice de modulation
        int32_t signal = sineLUT[(finalCarrierPhase >> SINE_SHIFT) & SINE_LUT_MASK];

        // Applique l'enveloppe 32 bits et le volume
        int32_t envSafe = currentEnv >> 14;
        envSafe &= ~(envSafe >> 31); // protection contre les valeurs négatives
        int32_t finalVol = (int32_t)((envSafe * volBase) >> 14);

        mixBuffer[i] += (signal * finalVol) >> 16;

        carrierPhase += carrierInc;
        modulatorPhase += modulatorInc;
        currentEnv += envStep;
    }

    // Sauvegarde les états dans les registres personnalisés de la voix
    vo->phase = carrierPhase;
    vo->cw[0] = modulatorPhase;
    vo->cw[1] = (uint32_t)prevSample;
}

void jouer_lead_fm() {
    synth.setCustomWave(0, fmTwoOpOscillator);
    synth.noteOn(0, c4, 255);
}
```

---

## 14. Hooks DSP globaux et sortie personnalisée

### DSP global (post-mixage)

Appliqué sur le bus mixé 32 bits **avant** conversion au format du DAC. Idéal pour une réverbération, un delay, un compresseur global, etc.

```cpp
typedef void (*SynthDSPCallback)(int32_t* mixBuffer, int numSamples);
void setCustomDSP(SynthDSPCallback dspFunc);
```

```cpp
#define DECAY_LINE_SIZE 4096
#define DECAY_MASK (DECAY_LINE_SIZE - 1)

int32_t delayLine[DECAY_LINE_SIZE];
int32_t delayWriteIndex = 0;

// Filtre en peigne (comb filter) haute vitesse en virgule fixe
void IRAM_ATTR globalDelayDSP(int32_t* mixBuffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        int32_t inputSample = mixBuffer[i];

        // Récupère l'échantillon retardé en mémoire
        int32_t delayedSample = delayLine[(delayWriteIndex - 3000) & DECAY_MASK];

        // Filtrage en peigne (feedback ~62.5%, soit 5/8)
        int32_t newSample = inputSample + ((delayedSample * 5) >> 3);

        // Écrit dans le buffer circulaire
        delayLine[delayWriteIndex] = newSample;
        delayWriteIndex = (delayWriteIndex + 1) & DECAY_MASK;

        // Réinjecte dans le canal master actif
        mixBuffer[i] = newSample;
    }
}

void setup() {
    synth.begin(2, SMODE_I2S, 4, 15, I2S_16BIT);
    synth.setCustomDSP(globalDelayDSP);
}
```

### Callback de contrôle personnalisé

```cpp
typedef void (*SynthControlCallback)();
void setCustomControl(SynthControlCallback ctrlFunc);
```
Exécuté au taux de contrôle (`controlRateHz`, 100 Hz par défaut) — utile pour piloter tes propres LFO/logique globale en synchronisation avec le moteur.

### Mode "Pull" : sortie personnalisée (Bluetooth A2DP, WiFi, WebSocket...)

En `SMODE_CUSTOM`, aucun timer DMA interne n'est utilisé : c'est **toi** qui appelles la génération d'échantillons, au rythme demandé par ta pile réseau/Bluetooth.

```cpp
typedef void (*SynthCustomOutputCallback)(int16_t* samples, int numSamples);
void setCustomOutput(SynthCustomOutputCallback outFunc);

// Génération manuelle de sample
void generateSamples(int16_t* outBuffer, int numSamples);
void generateSamplesStereo(int16_t* outBufferLR, int numSamplePairs);
```

```cpp
ESP32Synth synth;

void setup() {
    // Initialisation à 44.1kHz ou 48kHz, sans timer automatique (customOutput = nullptr)
    synth.beginCustom(44100, nullptr);
    synth.noteOn(0, c4, 255);
}

// Callback audio de ta pile Bluetooth/WiFi
void write_bluetooth_packet(uint8_t *stream_buffer, int buffer_length) {
    int samplePairs = buffer_length / 4; // Chaque trame stéréo 16 bits = 4 octets (G + D)

    // Convertit, met à l'échelle et copie les trames directement
    synth.generateSamplesStereo((int16_t*)stream_buffer, samplePairs);
}
```

---

## 15. Getters et diagnostics

```cpp
uint32_t getFrequencyCentiHz(uint16_t voice);
uint16_t getVolume(uint16_t voice);
uint8_t  getVolume8Bit(uint16_t voice);
uint8_t  getEnv8Bit(uint16_t voice);
uint8_t  getOutput8Bit(uint16_t voice);
uint32_t getVolumeRaw(uint16_t voice);
uint32_t getEnvRaw(uint16_t voice);
uint32_t getOutputRaw(uint16_t voice);
bool     isVoiceActive(uint16_t voice);
EnvState getEnvState(uint16_t voice);
WaveType getWaveType(uint16_t voice);
uint32_t getPhase(uint16_t voice);
uint32_t getPulseWidth(uint16_t voice);

float getCPULoad(); // Charge CPU du moteur audio, de 0.0 à 100.0 %
```

### Débogage bas niveau

- **Clics numériques / redémarrages Watchdog** : vérifie que le processeur Xtensa tourne bien à **240 MHz** (certaines cartes ESP32 démarrent par défaut à 160 MHz, ce qui réduit fortement la marge de calcul disponible).
- **Contention FPU sur S3** : l'ESP32-S3 utilise des registres SIMD vectoriels avancés sur le Core 1. Si d'autres tâches gourmandes (caméra, calculs complexes...) tournent en parallèle sur le Core 1, il y aura des conflits. Place ces tâches sur le Core 0 et réserve le Core 1 exclusivement au moteur audio.
- **Audio PWM qui scintille** : en `SMODE_PWM`, assure-toi qu'aucune autre tâche n'accède au canal LEDC 0 ni n'écrit dans les registres du Timer 0 (ça casse l'alignement de l'ISR de débordement). En cas de sifflement haute fréquence sur la broche, ajoute un simple filtre RC passe-bas de reconstruction (résistance 150 Ω + condensateur 100 nF).

---

## 16. Table des types et énumérations

```cpp
enum SynthOutputMode : uint8_t { SMODE_PDM, SMODE_I2S, SMODE_DAC, SMODE_PWM, SMODE_CUSTOM };

enum WaveType : int8_t {
    WAVE_SINE      = -1,
    WAVE_TRIANGLE  = -2,
    WAVE_SAW       = -3,
    WAVE_PULSE     = -4,
    WAVE_NOISE     = -5,
    WAVE_WAVETABLE = 1,
    WAVE_SAMPLE    = 2,
    WAVE_STREAM    = 3,
    WAVE_CUSTOM    = 4,
};

enum BitDepth : uint8_t { BITS_4, BITS_8, BITS_16 };

enum EnvState : uint8_t { ENV_IDLE, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE };

enum I2S_Depth : uint8_t { I2S_16BIT, I2S_32BIT };

enum LoopMode : uint8_t { LOOP_OFF, LOOP_FORWARD, LOOP_PINGPONG, LOOP_REVERSE };
```

## 17. Table des notes (CentiHz)

Le fichier `ESP32SynthNotes.h` définit toutes les notes de `c0` à environ `b9`/`g10` (do à si, dièses en suffixe `s`), en **CentiHz** (Hz × 100). Exemples :

```cpp
#define c4  26163   // Do médian ≈ 261.63 Hz
#define cs4 27718   // Do#4
#define a4  44000   // La4 = 440 Hz (référence standard)
```

Ces constantes s'utilisent directement dans `noteOn()`, `setFrequency()`, `setArpeggio()`, etc.

---

## 18. Visualiser tes ondes en direct sur un écran OLED (mini-oscilloscope)

Ce n'est **pas une fonctionnalité native d'ESP32Synth** — la bibliothèque n'a pas d'oscilloscope intégré — mais une technique à construire toi-même par-dessus, très utile en développement : voir *réellement* la forme d'onde que tu génères (de base, wavetable, sample ou `WAVE_CUSTOM`) plutôt que de l'imaginer à partir du code.

### 18.1 Pourquoi `getOutputRaw()` ne suffit pas

En regardant son implémentation réelle (§15) :
```cpp
uint32_t ESP32Synth::getOutputRaw(uint16_t voice) {
    return (uint32_t)(((uint64_t)voices[voice].currEnvVal * voices[voice].vol) >> 16);
}
```
`getOutputRaw()` (comme `getOutput8Bit()`) ne renvoie que **enveloppe × volume** — un simple niveau global (utile pour un VU-mètre / une barre de niveau), **pas la valeur instantanée de l'onde**. Il ne te permettra donc jamais de voir la *forme* réelle d'un sinus, d'une dent de scie ou de ton oscillateur `WAVE_CUSTOM` — pour ça, il faut intercepter les échantillons audio bruts eux-mêmes, avant ou après mixage.

### 18.2 La bonne approche : capturer le bus audio via `setCustomDSP()`

Le hook `setCustomDSP()` (§14) te donne accès à `mixBuffer`, le buffer 32 bits contenant le signal **déjà sommé de toutes les voix actives**. C'est le point d'accroche idéal : à chaque bloc audio, on recopie quelques échantillons dans un buffer "photo" (snapshot) que la boucle principale (`loop()`) ira lire pour dessiner sur l'OLED — exactement la même technique de **double buffering** que celle vue dans `SVF_filters_on_wavetable` (§8 des exemples), mais appliquée ici à de la capture au lieu du calcul de filtre.

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP32Synth.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);
ESP32Synth synth;

// --- Double buffer de capture (1 point par pixel de large d'écran) ---
#define SCOPE_WIDTH 128
int16_t scopeBuf[2][SCOPE_WIDTH];
volatile uint8_t  captureBufIdx  = 0;   // Buffer dans lequel le DSP écrit en ce moment
volatile uint16_t captureWriteIdx = 0;  // Position d'écriture dans ce buffer
volatile bool     scopeReady     = false; // Un buffer complet est prêt à être dessiné

// Facteur de décimation : 1 = zoom max (regarde 128 échantillons bruts, ~2.7ms à 48kHz,
// pratique pour une seule note grave). Augmente-le pour "dézoomer" et voir plusieurs
// cycles d'une note aiguë (ex. 8 pour voir ~21ms de signal).
#define SCOPE_DECIMATION 4
uint16_t decimCounter = 0;

// --- Capture (tourne dans la tâche audio, Core 1 — reste rapide et sans allocation !) ---
void IRAM_ATTR scopeCapture(int32_t* mixBuffer, int numSamples) {
    if (scopeReady) return; // La boucle principale n'a pas encore consommé le buffer précédent

    for (int i = 0; i < numSamples; i++) {
        if (++decimCounter < SCOPE_DECIMATION) continue;
        decimCounter = 0;

        // Le bus master est en 32 bits ; on ramène sur 16 bits pour l'affichage.
        scopeBuf[captureBufIdx][captureWriteIdx] = (int16_t)(mixBuffer[i] >> 16);
        captureWriteIdx++;

        if (captureWriteIdx >= SCOPE_WIDTH) {
            captureWriteIdx = 0;
            scopeReady = true;      // Signale à loop() qu'un buffer est prêt
            captureBufIdx ^= 1;     // Bascule sur l'autre buffer pour la capture suivante
            break;                  // On finira de traiter le reste du bloc au prochain appel
        }
    }
}

void setup() {
    Wire.begin(21, 22); // Adapte selon ton câblage SDA/SCL
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    synth.begin(4, 15, 2, I2S_32BIT); // Adapte selon ton câblage I2S
    synth.setMasterVolume(200);
    synth.setCustomDSP(scopeCapture); // Attache la capture au bus master

    // --- Exemple : une onde à observer ---
    synth.setWave(0, WAVE_SAW);
    synth.setEnv(0, 10, 200, 150, 400);
    synth.noteOn(0, c3, 200); // Une note grave se voit bien avec peu de décimation
}

void loop() {
    if (scopeReady) {
        // Copie locale rapide pour ne pas bloquer trop longtemps le buffer partagé
        uint8_t readBuf = captureBufIdx ^ 1;
        static int16_t localBuf[SCOPE_WIDTH];
        memcpy(localBuf, scopeBuf[readBuf], sizeof(localBuf));
        scopeReady = false; // Libère le buffer pour la prochaine capture

        // --- Trigger simple : cherche un front montant proche de zéro ---
        // (évite que le dessin ne "saute" horizontalement à chaque rafraîchissement)
        int triggerIdx = 0;
        for (int i = 1; i < SCOPE_WIDTH - 1; i++) {
            if (localBuf[i - 1] <= 0 && localBuf[i] > 0) { triggerIdx = i; break; }
        }

        // --- Dessin ---
        display.clearDisplay();
        int prevX = 0, prevY = 32;
        for (int x = 0; x < SCOPE_WIDTH; x++) {
            int srcIdx = (triggerIdx + x) % SCOPE_WIDTH;
            // Mise à l'échelle : 16 bits (-32768..32767) -> hauteur d'écran (0..63), centré sur 32
            int y = 32 - (localBuf[srcIdx] >> 10);
            if (y < 0) y = 0; if (y > 63) y = 63;

            if (x > 0) display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
            prevX = x; prevY = y;
        }
        display.display(); // ~30-40ms sur I2C standard -> vise ~20-25 rafraîchissements/seconde
    }
}
```

### 18.3 Points importants

- **`scopeReady` fait office de verrou simple** entre la tâche audio (qui écrit) et `loop()` (qui lit) : tant que `loop()` n'a pas consommé le buffer (`scopeReady = false`), la capture suivante est ignorée (`if (scopeReady) return;`) plutôt que d'écraser des données en cours de lecture — ça évite un déchirement d'image (tearing) sans avoir besoin d'un vrai mutex FreeRTOS, au prix de sauter occasionnellement une capture si `loop()` est lente (acceptable pour un usage visuel).
- **Le trigger par front montant** (`localBuf[i-1] <= 0 && localBuf[i] > 0`) est ce qui stabilise l'affichage à l'écran — sans lui, la forme d'onde "glisserait" horizontalement à chaque rafraîchissement car le point de départ de la capture n'a aucune raison de tomber au même endroit du cycle à chaque fois. C'est exactement le principe du trigger d'un vrai oscilloscope.
- **`SCOPE_DECIMATION`** contrôle le "zoom temporel" : à 48 kHz, capturer 128 échantillons bruts (décimation 1) ne couvre que ~2.7 ms — suffisant pour voir un seul cycle d'une note grave (`c2` ≈ 65 Hz → période ~15 ms, donc il en faudrait plutôt ~5-6 pour voir un cycle complet), mais beaucoup trop court pour une note aiguë. Augmente la décimation (ne garder qu'1 échantillon sur N) pour observer une durée plus longue, au prix d'une résolution temporelle plus grossière.
- **`display.display()` coûte cher** (transfert I2C de tout le framebuffer, ~30-40 ms typiquement) : ça plafonne le taux de rafraîchissement visuel autour de 20-30 Hz, largement suffisant pour un usage de mise au point visuelle, mais n'espère pas un vrai oscilloscope temps réel fluide.

### 18.4 Pour isoler une seule voix (utile en développement de `WAVE_CUSTOM`)

La méthode ci-dessus capture le **bus master** (toutes les voix actives sommées). Si tu es en train de mettre au point un oscillateur personnalisé et que tu veux voir *uniquement* sa sortie, isolée des autres voix, deux options :

1. **Le plus simple** : coupe le volume de toutes les autres voix (`synth.setVolume(i, 0)`) pendant que tu regardes celle qui t'intéresse — le bus master devient alors équivalent à cette voix seule.
2. **Le plus précis** : si tu écris toi-même le callback `WAVE_CUSTOM` (§13), tu as un accès direct à chaque échantillon avant qu'il ne soit ajouté au mixage — tu peux donc écrire une copie de débogage toi-même, directement dans ta fonction :
   ```cpp
   void IRAM_ATTR monOscillateur(Voice* vo, int32_t* mixBuffer, int samples, int32_t startEnv, int32_t envStep) {
       for (int i = 0; i < samples; i++) {
           int32_t s = /* ... ton calcul de synthèse ... */;
           mixBuffer[i] += s;

           // Optionnel : recopie isolée pour debug/scope, indépendante du mixage global
           if (!scopeReady) {
               scopeBuf[captureBufIdx][captureWriteIdx++] = (int16_t)(s >> 16);
               if (captureWriteIdx >= SCOPE_WIDTH) { captureWriteIdx = 0; scopeReady = true; captureBufIdx ^= 1; }
           }
       }
   }
   ```
   Cette variante te donne la forme d'onde exacte de **ton** oscillateur, avant tout mixage ou enveloppe globale — l'idéal pour mettre au point un nouvel algorithme `WAVE_CUSTOM` (repliement, FM, waveshaping...) et voir immédiatement l'effet de chaque changement de formule sur la forme réelle du signal.

### 18.5 Alternative plus simple : un simple VU-mètre (barre de niveau)

Si un vrai oscilloscope est plus que ce dont tu as besoin, un simple indicateur de niveau est beaucoup plus léger à coder — et là, `getOutputRaw()`/`getOutput8Bit()` (§15) suffisent parfaitement, puisqu'ils donnent justement le niveau global (enveloppe × volume) sans qu'il soit nécessaire de capturer le signal audio brut :
```cpp
void loop() {
    uint8_t level = synth.getOutput8Bit(0); // 0-255
    int barHeight = map(level, 0, 255, 0, 63);
    display.clearDisplay();
    display.fillRect(0, 63 - barHeight, 20, barHeight, SSD1306_WHITE);
    display.display();
    delay(30);
}
```
Utile par exemple pour visualiser en direct une enveloppe ADSR ou une séquence d'instrument (§10) qui joue, sans avoir besoin de voir la forme d'onde elle-même.

---
