# 💾 Exemples : échantillons (samples) et streaming carte SD

Couvre : `Amen_Break_Loop`, `Low_BitDepth`, `ESP32Synth_WavPlayer`, `ESP32Synth_CYD_LVGL_WAV_PLAYER`.

---

## 1. Amen_Break_Loop — boucle de batterie en boucle infinie

**Dossier d'origine :** `examples/Samples/Amen_Break_Loop/`

### Ce que fait l'exemple
Charge un échantillon audio embarqué en mémoire flash (`Sample.h`, le fameux "Amen Break"), le configure comme instrument avec une zone unique couvrant toute la tessiture, et le joue en boucle avant (`LOOP_FORWARD`) indéfiniment.

### Code traduit

```cpp
#include "Sample.h"
#include "ESP32Synth.h"

ESP32Synth synth;

// --- Configuration pour setup() ---
const SampleZone zones_amen_break_44100hz[] = {
    { c0, g10, 0, c4 }
};

Instrument_Sample inst_amen_break_44100hz = {
    zones_amen_break_44100hz, // Le const SampleZone ci-dessus
    1, // Combien de zones
    LOOP_FORWARD, // Mode de boucle
    0, // début de la boucle
    0  // fin de la boucle (0 = dernier échantillon)
};

const int BCK_PIN = 19;
const int WS_PIN = 5;
const int DATA_PIN = 18;

void setup() {
    Serial.begin(115200);

    synth.begin(BCK_PIN, WS_PIN, DATA_PIN);
    synth.registerSample(0, amen_break_44100hz_data, amen_break_44100hz_len, amen_break_44100hz_rate, c4);
    synth.setInstrument(0, &inst_amen_break_44100hz);

    synth.setEnv(0,0,0,255,0); // enveloppe simple
    synth.noteOn(0,c4,255);
}

void loop() {
 // rien ici ! le Core 1 s'occupe déjà de l'audio et de la boucle !
}
```

### Explications
- **`SampleZone { lowFreq, highFreq, sampleId, rootOverride }`** : ici une seule zone couvre toute la plage `c0` à `g10`, ce qui signifie que quelle que soit la note jouée sur la voix, c'est ce même échantillon (`sampleId = 0`) qui sera utilisé, avec `c4` comme fréquence de référence (root pitch) — c'est-à-dire la hauteur à laquelle l'échantillon est joué sans transposition.
- `LOOP_FORWARD` avec `loopStart = 0` et `loopEnd = 0` signifie : boucle sur l'intégralité de l'échantillon, du début jusqu'à la fin (0 = "jusqu'au dernier échantillon").
- L'enveloppe `setEnv(0,0,0,255,0)` (Attaque=0, Decay=0, Sustain=255, Release=0) désactive quasiment toute enveloppe : le son est joué à plein volume immédiatement, sans fondu.
- Comme le rendu audio tourne dans une tâche FreeRTOS dédiée sur le Core 1, une fois `noteOn()` appelé dans `setup()`, la boucle audio continue indéfiniment **même si `loop()` est vide**.

---

## 2. Low_BitDepth — jouer un échantillon en 4 bits (lo-fi)

**Dossier d'origine :** `examples/Samples/Low_BitDepth/`

### Ce que fait l'exemple
Enregistre un échantillon en précisant explicitement une profondeur de **4 bits** (`BITS_4`), ce qui permet de réduire drastiquement l'empreinte mémoire d'un son sans avoir à trop sacrifier son taux d'échantillonnage.

### Code traduit

```cpp
#include <ESP32Synth.h>
#include "Sample.h"
ESP32Synth synth;

void setup(){
  synth.begin(26);
  synth.setWave(0,WAVE_SAMPLE);
  synth.registerSample(0, Barbeiro_cegolow_data, Barbeiro_cegolow_len, Barbeiro_cegolow_rate, c4, BITS_4);
  // On peut désormais réduire la profondeur de bits sans avoir à trop réduire le taux d'échantillonnage
  synth.setSample(0,0,LOOP_FORWARD, 0, 0);
  synth.noteOn(0,c4,255);
}
void loop(){

}
```

### Explications
- `registerSample(sampleId, data, length, sampleRate, rootFreqCentiHz, depth)` accepte un paramètre optionnel `depth` (`BITS_4`, `BITS_8` ou `BITS_16` — par défaut `BITS_16`). En choisissant `BITS_4`, on divise par 4 la taille mémoire de l'échantillon comparé à du 16 bits, au prix d'une résolution d'amplitude réduite (16 niveaux seulement) — un compromis classique pour de la voix ou des percussions courtes sur des projets à mémoire flash limitée.
- `synth.begin(26)` utilise ici la surcharge la plus simple de `begin()` : un seul argument = **DAC interne** sur la broche indiquée (26). Cette surcharge ne fonctionne que sur ESP32 classique.
- `setWave(0, WAVE_SAMPLE)` puis `setSample(...)` est la méthode "manuelle" (sans passer par `Instrument_Sample`) pour attribuer un échantillon unique et fixe à une voix.

---

## 3. ESP32Synth_WavPlayer — lecteur WAV complet avec écran OLED et encodeur rotatif

**Dossier d'origine :** `examples/Sd Streams/ESP32Synth_WavPlayer/`

### Ce que fait l'exemple
Un vrai petit lecteur audio embarqué : scan des fichiers `.wav` sur la carte SD, affichage d'une playlist défilante sur écran OLED SSD1306, navigation et contrôle (play/pause, avance/retour 5s, boucle, volume) via un encodeur rotatif KY-040. C'est l'exemple le plus complet pour le **streaming SD**.

### Structure générale traduite (extraits clés)

```cpp
/**
 * @file ESP32Synth_WavPlayer.ino
 * @brief Lecteur de fichiers WAV utilisant ESP32Synth
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <vector>
#include "ESP32Synth.h"

// ==========================================================
// == DÉFINITION DES BROCHES
// ==========================================================

// Carte SD
#define SD_CS     5
#define SD_SCK    18
#define SD_MISO   19
#define SD_MOSI   23

// DAC I2S (PCM5102A)
#define I2S_BCK   4
#define I2S_WS    15
#define I2S_DIN   2

// Écran OLED I2C (broches par défaut)
#define OLED_SDA  21
#define OLED_SCL  22

// Encodeur rotatif (module KY-040)
#define ENC_CLK   13 // Broche A (CLK)
#define ENC_DT    14 // Broche B (DT)
#define ENC_SW    27 // Bouton de l'encodeur (SW)

// ... (déclarations des états UI, playlist, etc.)

void setup() {
    Serial.begin(115200);
    Wire.begin(OLED_SDA, OLED_SCL);
    // ... initialisation de l'écran, de l'encodeur ...

    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI, 16000000)) {
        // Erreur carte SD
        while (1);
    }

    File root = SD.open("/");
    scanDirectory(root); // Recherche récursive des fichiers .wav
    root.close();

    if (!synth.begin(I2S_BCK, I2S_WS, I2S_DIN, I2S_32BIT)) {
        // Erreur initialisation du synthétiseur
        while (1);
    }
    synth.setMasterVolume(currentVolume);
}

void loop() {
    handleInput();       // Lecture de l'encodeur et du bouton
    updatePlayerLogic();  // Gère la fin de piste / boucle / piste suivante
    drawUI();             // Rafraîchit l'écran OLED
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Recherche tous les fichiers .wav sur la carte SD
void scanDirectory(File dir) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
            String name = entry.name();
            String nameLower = name;
            nameLower.toLowerCase();
            if (nameLower.endsWith(".wav")) {
                playlist.push_back(name);
            }
        }
        entry.close();
    }
}

void playTrack(int index) {
    if (index < 0 || index >= playlist.size()) return;

    synth.stopStream(0);
    String path = "/" + playlist[index];

    synth.playStream(0, SD, path.c_str(), 255, 44100, false);

    currentTrack = index;
    isPlaying = true;

    delay(50);
    trackDurationMs = synth.getStreamDurationMs(0);
}

void updatePlayerLogic() {
    if (uiState == STATE_PLAYER && isPlaying) {
        if (!synth.isStreamPlaying(0) && trackDurationMs > 0) {
            if (isLooping) {
                playTrack(currentTrack);
            } else {
                int nextTrk = currentTrack + 1;
                if (nextTrk >= playlist.size()) nextTrk = 0;
                playTrack(nextTrk);
            }
        }
    }
}
```

Contrôles gérés via `handleInput()` (extraits) :
```cpp
case CTRL_PLAY:
    if (isPlaying) { synth.pauseStream(0); isPlaying = false; }
    else { synth.resumeStream(0); isPlaying = true; }
    break;
case CTRL_REV5: {
    uint32_t p = synth.getStreamPositionMs(0);
    synth.seekStreamMs(0, p > 5000 ? p - 5000 : 0);   // recule de 5 secondes
    break;
}
case CTRL_FWD5: {
    uint32_t p = synth.getStreamPositionMs(0);
    if (p + 5000 < trackDurationMs) synth.seekStreamMs(0, p + 5000); // avance de 5 secondes
    break;
}
case CTRL_LOOP:
    isLooping = !isLooping;
    break;
```

### Explications
- Toute la logique de streaming SD utilise l'API de haut niveau vue au §12 du guide API : `playStream()`, `pauseStream()`, `resumeStream()`, `seekStreamMs()`, `getStreamPositionMs()`, `getStreamDurationMs()`, `isStreamPlaying()`, `stopStream()`.
- Le fichier est décodé en tâche de fond (Core 0) : c'est ce qui permet à `loop()` de gérer en même temps l'affichage OLED et la navigation, sans jamais provoquer de coupure audio (contrairement à une lecture SD "bloquante" classique).
- L'essentiel du reste du fichier (non reproduit intégralement ici) est de la logique d'interface graphique OLED "maison" (dessin d'icônes, texte défilant type marquee, sliders de volume/position) : ce n'est pas spécifique à ESP32Synth, mais illustre bien comment construire une UI complète autour du moteur audio.

---

## 4. ESP32Synth_CYD_LVGL_WAV_PLAYER — même principe, sur écran tactile CYD avec LVGL

**Dossier d'origine :** `examples/Sd Streams/ESP32Synth_CYD_LVGL_WAV_PLAYER/`

### Ce que fait l'exemple
Adapte le lecteur WAV pour une carte **"Cheap Yellow Display" (CYD)** — un ESP32 avec écran tactile TFT intégré — en utilisant la bibliothèque graphique **LVGL** plutôt que Adafruit_GFX/SSD1306.

### En-tête traduit et broches

```cpp
// Active le mode « RAM ultra basse » dans ESP32Synth.h pour fonctionner sur le CYD.
// Vérifie que ton LVGL est bien configuré.

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "ESP32Synth.h"

// Broches CYD
#define XPT_IRQ 36
#define XPT_MOSI 32
#define XPT_MISO 39
#define XPT_CLK 25
#define XPT_CS 33
#define BACKLIGHT_PIN 21
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5

// Broches DAC I2S
#define I2S_BCK 22
#define I2S_WS 27
#define I2S_DATA 3

// Limites (réduites pour tenir dans la RAM du ESP32)
```

### Explications
- Ce commentaire d'en-tête est important : sur une carte CYD (peu de RAM libre à cause de l'écran tactile + LVGL), il est fortement recommandé de basculer `src/ESP32Synth_Config.hpp` sur le profil **"Low RAM usage"** présenté au §2 du guide API (`MAX_VOICES = 1`, etc.), sans quoi la compilation ou l'exécution peut échouer par manque de mémoire.
- Le principe de streaming reste identique à l'exemple précédent (`playStream`, `pauseStream`...) : seule la couche d'affichage change (widgets LVGL tactiles au lieu de dessin bas niveau sur OLED).
- `TFT_eSPI` et `XPT2046_Touchscreen` gèrent respectivement l'écran TFT et la dalle tactile résistive du CYD — ce sont des bibliothèques externes, pas fournies par ESP32Synth.
