# 🎼 Exemples : wavetables et instruments façon "tracker"

Couvre : `WavetableSynth`, `InstrumentSequence`.

---

## 1. WavetableSynth — deux wavetables personnalisées

**Dossier d'origine :** `examples/Wavetables/WavetableSynth/`

### Ce que fait l'exemple
Définit deux wavetables 8 bits (256 échantillons chacune) : une onde « buzzy » (bourdonnante, faite de quelques harmoniques de sinus) et une onde « hollow » (creuse, façon carré doux). Elles sont enregistrées avec les identifiants 10 et 11, puis une mélodie alterne entre les deux timbres.

### Code traduit

```cpp
/**
 * @file WavetableSynth.ino
 * @author Danilo Gabriel
 * @brief Démontre l'utilisation de plusieurs wavetables personnalisées.
 *
 * Cet exemple définit deux wavetables 8 bits différentes :
 * 1. Une onde « buzzy » (bourdonnante), faite de seulement quelques harmoniques de sinus.
 * 2. Une onde « hollow » (creuse), façon carré adouci.
 *
 * Il les enregistre sous les ID 10 et 11, puis joue une mélodie simple,
 * en alternant entre les deux sons.
 *
 * Utilise la sortie I2S. Assure-toi d'avoir un DAC I2S connecté.
 * - BCK_PIN  : broche d'horloge de bits I2S
 * - WS_PIN   : broche de sélection de mot (LRC) I2S
 * - DATA_PIN : broche de données I2S
 */

#include <Arduino.h>
#include <ESP32Synth.h>
#include "WavetableData.h" // Nos wavetables personnalisées sont ici

// --- Configuration des broches pour DAC I2S ---
// --- Tu DOIS changer ces broches pour correspondre à ton câblage ---
const int BCK_PIN = 26;
const int WS_PIN = 25;
const int DATA_PIN = 22;

ESP32Synth synth;

// Une mélodie simple utilisant des notes de la gamme de Do mineur
uint32_t melody[] = { c4, ds4, g4, c5, g4, f4, ds4, c4 };
// Quelle wavetable utiliser pour chaque note (10 ou 11)
uint16_t wave_for_note[] = { 10, 10, 10, 11, 11, 10, 11, 10 };

// Définit les identifiants des wavetables
const uint16_t BUZZY_WAVE_ID = 10;
const uint16_t HOLLOW_WAVE_ID = 11;

void setup() {
  Serial.begin(115200);
  Serial.println("Exemple ESP32Synth - Wavetable");

  // --- Initialise le synthétiseur ---
  if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {
    Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
    while (1) delay(1000);
  }

  synth.setMasterVolume(200);

  // --- Enregistre les wavetables ---
  // Les données de ces tables sont dans WavetableData.h
  synth.registerWavetable(BUZZY_WAVE_ID, buzzy_wavetable, 256, BITS_8);
  synth.registerWavetable(HOLLOW_WAVE_ID, hollow_wavetable, 256, BITS_8);

  Serial.println("Synthétiseur initialisé et wavetables personnalisées enregistrées !");
}

void loop() {
  Serial.println("\nLecture de la mélodie avec les wavetables personnalisées...");

  uint8_t voice = 0;
  // Fixe une enveloppe générique pour la voix
  synth.setEnv(voice, 20, 150, 110, 250);

  // Joue la mélodie
  for (int i = 0; i < 8; i++) {
    uint16_t current_wave_id = wave_for_note[i];

    // Configure la voix pour utiliser la wavetable voulue.
    // On fait ça en fixant d'abord le type d'onde, puis les données spécifiques.
    // Le synth se souvient des dernières données fixées pour le type WAVE_WAVETABLE.
    // Une meilleure méthode consiste à utiliser les Instruments, mais ceci illustre le principe de base.
    synth.setWave(voice, WAVE_WAVETABLE);
    if(current_wave_id == BUZZY_WAVE_ID) {
        synth.setWavetable(voice, buzzy_wavetable, 256, BITS_8);
        Serial.printf("Note %d : Fréquence %.2f Hz, Onde : Buzzy\n", i, (float)melody[i] / 100.0f);
    } else {
        synth.setWavetable(voice, hollow_wavetable, 256, BITS_8);
        Serial.printf("Note %d : Fréquence %.2f Hz, Onde : Hollow\n", i, (float)melody[i] / 100.0f);
    }

    synth.noteOn(voice, melody[i], 127);
    delay(300);
    synth.noteOff(voice);
    delay(100);
  }

  delay(3000); // Attend avant de répéter
}
```

Extrait de `WavetableData.h` (les données brutes, 8 bits, valeurs 0-255 centrées sur 128) :
```cpp
// Une wavetable au son "buzzy" (bourdonnant), créée en sommant quelques sinusoïdes.
// Elle a un timbre plus brillant et plus complexe qu'un simple sinus.
const uint8_t buzzy_wavetable[256] = {
  128, 148, 168, 186, 202, 215, 225, 232,
  236, 238, 237, 234, 229, 222, 214, 205,
  // ... (256 valeurs au total, un cycle complet de l'onde)
};
```

### Explications
- Une **wavetable** est simplement un tableau représentant **un cycle complet** d'une forme d'onde, rejoué en boucle à la vitesse (fréquence) demandée. En 8 bits, les valeurs vont de 0 à 255 avec 128 comme point milieu (silence).
- `registerWavetable(id, data, size, depth)` permet d'enregistrer une wavetable sous un identifiant réutilisable ; `setWavetable(voice, data, size, depth)` l'attribue directement à une voix précise. Le code montre les deux usages combinés.
- Utilise l'outil `tools/Wavetables/WavetableMaker.py` pour générer facilement tes propres wavetables `.h` à partir d'équations ou de segments audio.

---

## 2. InstrumentSequence — un instrument façon tracker (pluck qui s'éteint)

**Dossier d'origine :** `examples/Tracker_FX/InstrumentSequence/`

### Ce que fait l'exemple
Montre comment utiliser la structure `Instrument` pour créer un son qui **évolue automatiquement dans le temps** tant que la note est tenue — comme les instruments des vieux trackers (ProTracker, etc.). Ici : un son « pincé » (pluck) qui s'éteint progressivement.

### Code traduit

```cpp
/**
 * @file InstrumentSequence.ino
 * @author Danilo Gabriel
 * @brief Démontre la fonctionnalité 'Instrument' pour des effets façon tracker.
 *
 * Cet exemple montre comment utiliser la structure 'Instrument' pour créer un son
 * qui évolue dans le temps, comme les instruments des trackers old-school.
 *
 * L'instrument définit une séquence de volumes et d'ID de wavetables qui sont
 * jouées automatiquement quand une note est tenue. Celui-ci crée un simple
 * son « pincé » (plucked) qui s'estompe.
 *
 * Utilise la sortie I2S. Assure-toi d'avoir un DAC I2S connecté.
 * - BCK_PIN  : broche d'horloge de bits I2S
 * - WS_PIN   : broche de sélection de mot (LRC) I2S
 * - DATA_PIN : broche de données I2S
 */

#include <Arduino.h>
#include <ESP32Synth.h>
#include "Wavetables.h" // Inclut les données de wavetable personnalisées

// --- Configuration des broches pour DAC I2S ---
// --- Tu DOIS changer ces broches pour correspondre à ton câblage ---
const int BCK_PIN = 26;
const int WS_PIN = 25;
const int DATA_PIN = 22;

ESP32Synth synth;

// --- Définit la séquence de l'instrument ---

// Enveloppe de volume pour la partie attaque/sustain du son.
// Ceci crée un effet de fondu rapide.
const uint8_t attackVolumes[] = { 127, 100, 80, 60, 40, 20, 10, 0 };

// On utilise la même wavetable simple type sinus pour tout le son.
// Utiliser W_SINE fonctionnerait aussi. Ici on en utilise une personnalisée pour la démonstration.
const int16_t attackWaves[] = { 1, 1, 1, 1, 1, 1, 1, 1 };

// Définit la structure principale de l'instrument
Instrument pluckyInstrument = {
  .seqVolumes    = attackVolumes,     // Pointeur vers la séquence de volumes
  .seqWaves      = attackWaves,       // Pointeur vers la séquence d'ID de wavetables
  .seqLen        = 8,                 // Nombre de pas dans la séquence
  .seqSpeedMs    = 30,                // Durée en ms de chaque pas

  .susVol        = 0,                 // Volume de sustain (0, car le son s'éteint)
  .susWave       = W_SINE,            // Wavetable tenue si sustain > 0

  .relVolumes    = nullptr,           // Pas de séquence de relâchement séparée
  .relWaves      = nullptr,
  .relLen        = 0,
  .relSpeedMs    = 0,

  .smoothMorph   = false              // Ne pas faire d'interpolation entre les pas de wavetable
};


void setup() {
  Serial.begin(115200);
  Serial.println("Exemple ESP32Synth - Séquence d'instrument");

  // --- Initialise le synthétiseur ---
  if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {
    Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
    while (1) delay(1000);
  }

  synth.setMasterVolume(200);

  // --- Enregistre les wavetables ---
  // On doit enregistrer la wavetable utilisée par notre instrument (ID '1').
  // Les données sont dans Wavetables.h
  synth.registerWavetable(1, sine_wavetable, 256, BITS_8);

  Serial.println("Synthétiseur initialisé et wavetable enregistrée !");
}

void loop() {
  uint8_t voice = 0;

  // Attache l'instrument à la voix 0
  synth.setInstrument(voice, &pluckyInstrument);

  Serial.println("\nLecture de Do4 avec l'instrument personnalisé.");
  synth.noteOn(voice, c4, 127); // Le volume ici est maximal, l'instrument gère le reste

  // L'instrument a une durée totale de 8 pas * 30ms/pas = 240ms.
  // On tient la note plus longtemps pour montrer qu'elle s'est éteinte et arrêtée.
  delay(500);

  // La note se termine techniquement quand la séquence est finie, mais on appelle noteOff
  // pour s'assurer que la voix est correctement libérée.
  synth.noteOff(voice);

  delay(1000); // Attend avant de répéter

  // Joue maintenant une note plus aiguë
  Serial.println("Lecture de Sol4 avec l'instrument personnalisé.");
  synth.noteOn(voice, g4, 127);
  delay(500);
  synth.noteOff(voice);

  delay(2000); // Pause plus longue
}
```

### Explications
- La structure `Instrument` décrit une **séquence de pas** (`seqVolumes`/`seqWaves`) rejouée automatiquement dès que la note est déclenchée, à raison d'un pas toutes les `seqSpeedMs` millisecondes. Ici, 8 pas de 30 ms = 240 ms de fondu — le volume décroît de 127 à 0, ce qui donne un son "pincé" naturel.
- `susVol`/`susWave` définiraient ce qui se joue en boucle si le volume de sustain n'était pas à 0 (ici la séquence s'arrête simplement une fois arrivée à zéro).
- Cette approche imite directement le fonctionnement des instruments de trackers historiques (Amiga ProTracker, FastTracker II...) où chaque "instrument" est en réalité une petite séquence de commandes plutôt qu'un simple échantillon statique.
- Note : le volume passé à `noteOn()` (ici 127) sert de **volume de référence** ; c'est ensuite la séquence de l'instrument qui module ce volume dans le temps.
