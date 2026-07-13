# 🎹 Exemples : synthés physiques (boutons, encodeur, écran tactile)

Couvre : `ESP32Synth_Chill_Synth`, `CYD_Expressive_Ribbon_Synth`.

Ces deux exemples montrent comment transformer l'ESP32 en un véritable **instrument physique autonome** (sans PC ni MIDI), avec de vrais boutons/encodeur ou un écran tactile façon "ruban expressif" (ribbon controller).

---

## 1. ESP32Synth_Chill_Synth — synthé d'ambiance à 6 boutons d'accords

**Dossier d'origine :** `examples/Physical synths/ESP32Synth_Chill_Synth/`

### Matériel nécessaire (traduit du `Readme.txt`)
Il te faut 6 boutons, quelques fils de câblage, deux plaques d'essai (si tu utilises l'ESP32 classique, pour avoir plus de place et un montage plus propre), un DAC I2S, un écran OLED I2C SSD1306, un encodeur rotatif (testé avec le KY-040), et bien sûr un ESP32 double cœur.

| Périphérique | Broches ESP32 |
|---|---|
| DAC I2S | LCK(15), DIN(2), BCK(4) |
| Écran | SCK(22), SDA(21) |
| Encodeur | CLK(25), DT(33), SW(32) |
| Boutons | 5, 27, 14, 13, 18, 19 (câblage libre) |

### Ce que fait l'exemple
Chacun des 6 boutons déclenche un **accord de 6 notes** différent, piochés dans une banque de 12 accords/gammes (stockée en mémoire flash, donc "zéro RAM consommée"), parmi lesquels : gamme pentatonique apaisante, accords majeurs/mineurs enrichis (9e, 11e, 6/9), accord diminué 7, mode lydien, mode phrygien, accord "quartal" (empilement de quartes). Un encodeur rotatif permet probablement de faire défiler la banque d'accords (vu la présence de `ESP32Encoder`), affichée sur l'écran OLED.

### Code traduit (extraits clés)

```cpp
/*
   ESP32Synth - Chill Synth
   Auteur : Danilo Gabriel
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <ESP32Synth.h>

// Instances
ESP32Synth synth;
ESP32Encoder encoder;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Broches des boutons
const int btnPins[6] = {5, 27, 14, 13, 18, 19};
bool btnStates[6] = {true, true, true, true, true, true};

// ====================================================================================
// BANQUE D'ACCORDS ET DE GAMMES (6 notes par bouton) - En mémoire Flash (zéro RAM utilisée)
// ====================================================================================
#define NUM_CHORDS 12

const char* chordNames[NUM_CHORDS] = {
    "Paix Pentatonique",
    "Do Majeur 11",
    "Do Mineur 11",
    "Do Majeur 9",
    "Do Mineur 9",
    "Do Sus4 Ouvert",
    "Do Diminué 7",
    "Do Lydien Rêveur",
    "Do Phrygien Sombre",
    "Do Majeur 6/9",
    "Do Mineur 6/9",
    "Quartal Spatial"
};

const uint32_t chordBank[NUM_CHORDS][6] = {
    {c4, d4, e4, g4, a4, c5},         // 0 : Paix Pentatonique (Do, Ré, Mi, Sol, La, Do)
    {c4, e4, g4, b4, d5, f5},         // 1 : Do Maj 11
    {c4, ds4, g4, as4, d5, f5},       // 2 : Do Min 11 (Do, Mib, Sol, Sib, Ré, Fa)
    {c4, e4, g4, b4, d5, g5},         // 3 : Do Maj 9
    {c4, ds4, g4, as4, d5, g5},       // 4 : Do Min 9
    {c4, f4, g4, c5, f5, g5},         // 5 : Do Sus4 Ouvert
    {c4, ds4, fs4, a4, c5, ds5},      // 6 : Do Dim 7 (Do, Mib, Solb, La, Do, Mib)
    {c4, e4, g4, b4, d5, fs5},        // 7 : Do Lydien Rêveur (Do Maj avec 4e augmentée)
    {c4, cs4, f4, g4, as4, c5},       // 8 : Do Phrygien Dominant (sombre / égyptien)
    {c4, e4, g4, a4, d5, g5},         // 9 : Do Maj 6/9
    {c4, ds4, g4, a4, d5, g5},        // 10 : Do Min 6/9
    {c4, f4, as4, ds5, gs5, cs6}      // 11 : Quartal Spatial (tout en quartes)
};

// ====================================================================================
// BUFFER DE DELAY DSP (~300ms à 48kHz = ~57 Ko). En RAM interne (zéro défaut de cache)
// ====================================================================================
```

### Explications
- Stocker la banque d'accords en `const` place les données en **mémoire Flash** plutôt qu'en RAM (le compilateur peut le faire automatiquement pour des tableaux `const` non modifiés), un choix judicieux quand on a beaucoup de données musicales fixes et peu de RAM disponible.
- Chaque bouton déclenche typiquement 6 voix simultanément (une par note de l'accord) — un excellent exercice pratique pour la gestion multi-voix synchronisée (`noteOn` sur plusieurs voix d'un coup, `noteOff` groupé au relâchement du bouton).
- Le buffer de delay DSP (~57 Ko en RAM interne) illustre encore une fois l'importance de placer les gros buffers audio en **RAM interne** plutôt qu'en PSRAM externe, pour éviter les défauts de cache qui dégraderaient les performances temps réel.
- Cet exemple est une excellente base de départ si tu veux construire ton propre petit instrument d'ambiance/méditation avec seulement quelques boutons.

---

## 2. CYD_Expressive_Ribbon_Synth — synthé expressif à écran tactile façon thérémine/ruban

**Dossier d'origine :** `examples/Physical synths/CYD_Expressive_Ribbon_Synth/`

### Matériel nécessaire (traduit du `Readme.txt`)
Il te faut simplement une carte **CYD** ("Cheap Yellow Display", un ESP32 avec écran TFT tactile intégré, très populaire et bon marché). Pour une meilleure qualité sonore, tu peux ajouter un DAC I2S externe (PCM5102A) sur les broches prévues dans le code.

### Ce que fait l'exemple
Transforme l'écran tactile de la CYD en un **contrôleur "ruban" expressif** (façon thérémine ou "ribbon controller" de synthé analogique vintage) : la position du doigt sur l'écran pilote en continu la hauteur (pitch) et probablement un filtre résonant, avec une gestion soignée du tactile (anti-fantôme/anti-ghosting) pour éviter les faux déclenchements.

### En-tête et configuration traduits

```cpp
/*
   ESP32Synth - CYD Expressive Ribbon Synth
   Auteur : Danilo Gabriel

   - Onde Morph Tri-Saw 64 bits (immunisée contre le débordement / le biais DC)
   - Filtre SVF de Chamberlin avec verrou anti-blocage et compensation de gain
   - Anti-fantôme tactile et interface blindée sans traces visuelles résiduelles
*/

#pragma GCC optimize ("O3,unroll-loops")

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ESP32Synth.h>

// --- BROCHES CYD ---
#define XPT_IRQ 36
#define XPT_MOSI 32
#define XPT_MISO 39
#define XPT_CLK 25
#define XPT_CS 33
#define BACKLIGHT_PIN 21

// --- BROCHES AUDIO I2S ---
#define I2S_BCK 22
#define I2S_WS 27
#define I2S_DATA 3

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT_CS, XPT_IRQ);
ESP32Synth synth;

// =========================================================================
//    VARIABLES DE CONTRÔLE PARTAGÉES (interface -> DSP)
// =========================================================================
volatile uint32_t dsp_peak_phase = 2147483648;
volatile uint32_t dsp_span_phase = 2147483647;

volatile int32_t filter_f = 0;
volatile int32_t filter_q = 0;
volatile int32_t filter_gain = 32768;

static int32_t f_low = 0;
static int32_t f_high = 0;
static int32_t f_band = 0;
```

### Explications
- **"Onde Morph Tri-Saw 64 bits"** : l'oscillateur personnalisé de cet exemple mélange en continu (morph) entre une onde triangle et une dent de scie, avec des calculs en 64 bits pour éviter tout débordement (overflow) ou dérive de composante continue (DC bias) lors de glissandos rapides et continus du doigt sur l'écran — un souci qu'on rencontre vite avec un contrôleur "ruban" où la fréquence change en permanence, contrairement à des notes discrètes classiques.
- Le **filtre SVF de Chamberlin** (déjà vu dans `Acid_TB-303_SVF_Filter`) est repris ici avec un "verrou anti-blocage" supplémentaire : à haute résonance et modulation rapide et continue, ce type de filtre peut numériquement diverger (se bloquer sur une valeur infinie/instable) — le verrou anti-blocage détecte et corrige cette situation.
- Les **variables `volatile`** (`dsp_peak_phase`, `filter_f`, `filter_q`...) servent de pont de communication entre la tâche qui lit le tactile/dessine l'interface et le hook DSP audio (`setCustomDSP`) qui tourne dans une tâche séparée — c'est le schéma classique pour communiquer en toute sécurité entre deux tâches FreeRTOS sans mutex lourd, à condition que les variables restent de simples types atomiques (ici des entiers 32 bits).
- **"Interface blindée sans traces visuelles résiduelles"** fait référence à une technique de rafraîchissement d'écran optimisée (ne redessiner que les zones modifiées) pour éviter le scintillement ou les résidus graphiques ("ghosting") typiques d'un rafraîchissement TFT trop naïf combiné à un tactile résistif bruité.
