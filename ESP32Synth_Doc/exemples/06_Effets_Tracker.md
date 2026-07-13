# 🎹 Exemples : effets d'expression (vibrato, trémolo, glissandos, arpège)

Couvre : `EffectsDemo`, `Arpeggiator`.

---

## 1. EffectsDemo — vibrato, trémolo et glissandos

**Dossier d'origine :** `examples/Tracker Effects/EffectsDemo/`

### Ce que fait l'exemple
Une démonstration pédagogique et progressive de tous les effets d'expression natifs : trémolo (modulation de volume), vibrato (modulation de hauteur), glissando de fréquence (portamento), glissando de volume, puis une combinaison des quatre.

### Code traduit (intégral)

```cpp
/**
 * @file EffectsDemo.ino
 * @author Danilo Gabriel
 * @brief Démontre les fonctions de glissando, vibrato et trémolo d'ESP32Synth.
 *
 * Cet exemple présente différentes façons d'utiliser les effets intégrés du
 * synthétiseur pour ajouter de l'expression et du caractère aux sons.
 *
 * Fonctionne en sortie I2S ou avec le DAC interne des ESP32 classiques.
 *
 * --- Configuration ---
 * Pour utiliser l'I2S (recommandé pour une meilleure qualité), définis USE_I2S à 1
 * et configure tes broches I2S.
 *
 * Pour utiliser le DAC interne (ESP32 uniquement), définis USE_I2S à 0 et configure ta broche DAC.
 */

#include <ESP32Synth.h>

// --- CHOISIS TON MODE DE SORTIE ---
#define USE_I2S 1 // 1 pour DAC I2S, 0 pour DAC interne (ESP32 classique uniquement)

#if USE_I2S
  const int BCK_PIN  = 26; // Bit Clock
  const int WS_PIN   = 25; // Word Select (LRC)
  const int DATA_PIN = 22; // Data Out
#else
  const int DAC_PIN = 25;
#endif

ESP32Synth synth;

// =================================================================================
// SETUP
// =================================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("Démo d'effets ESP32Synth");
  Serial.printf("Cœur ESP32 utilisé : %s\n", synth.getChipModel());

  bool success;
  #if USE_I2S
    success = synth.begin(BCK_PIN, WS_PIN, DATA_PIN);
  #else
    #if !defined(CONFIG_IDF_TARGET_ESP32)
      Serial.println("!!! ERREUR : DAC interne disponible uniquement sur ESP32 d'origine.");
      while(1) delay(1000);
    #endif
    success = synth.begin(DAC_PIN);
  #endif

  if (!success) {
    Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
    while (1) delay(1000);
  }

  synth.setMasterVolume(200);

  // --- Configure une voix pour la démo ---
  uint8_t voice = 0;
  synth.setEnv(voice, 50, 200, 200, 300); // A, D, S, R
  synth.setWave(voice, WAVE_PULSE);
  synth.setPulseWidth(voice, 192); // Cycle de service 75%
}

// =================================================================================
// BOUCLE PRINCIPALE
// =================================================================================
void loop() {
  demonstrateTremolo();
  delay(1000);

  demonstrateVibrato();
  delay(1000);

  demonstrateFreqSlide();
  delay(1000);

  demonstrateVolumeSlide();
  delay(1000);

  demonstrateCombined();
  delay(3000);
}

// =================================================================================
// FONCTIONS DE DÉMONSTRATION
// =================================================================================

/** @brief Démontre l'effet de trémolo (modulation de volume). */
void demonstrateTremolo() {
  uint8_t voice = 0;

  Serial.println("Lecture d'une note tenue (La4)...");
  synth.noteOn(voice, a4, 255);
  delay(1000);

  Serial.println("-> Application d'un trémolo lent et peu profond (2 Hz)");
  // setTremolo(voice, rateCentiHz, depth)
  // rateCentiHz : vitesse en centièmes de Hz. 200 = 2 Hz.
  // depth : intensité de la modulation de volume (0-255 est une bonne plage).
  synth.setTremolo(voice, 200, 128);
  delay(2000);

  Serial.println("-> Augmentation de la vitesse du trémolo (8 Hz)");
  synth.setTremolo(voice, 800, 128);
  delay(2000);

  Serial.println("-> Augmentation de la profondeur du trémolo (220)");
  synth.setTremolo(voice, 800, 220);
  delay(2000);

  Serial.println("-> Désactivation du trémolo");
  synth.setTremolo(voice, 0, 0);
  delay(1000);

  synth.noteOff(voice);
}

/** @brief Démontre l'effet de vibrato (modulation de hauteur). */
void demonstrateVibrato() {
  uint8_t voice = 0;

  Serial.println("Lecture d'une note tenue (Do5)...");
  synth.noteOn(voice, c5, 255);
  delay(1000);

  Serial.println("-> Application d'un vibrato doux (5 Hz, déviation de 25Hz)");
  // setVibrato(voice, rateCentiHz, depthCentiHz)
  // rateCentiHz : vitesse de l'effet en centièmes de Hz. 500 = 5 Hz.
  // depthCentiHz : déviation de hauteur max par rapport à la fréquence de base, en centièmes de Hz.
  // 2500 = la hauteur varie de +/- 25 Hz.
  synth.setVibrato(voice, 500, 2500);
  delay(2000);

  Serial.println("-> Augmentation de la profondeur du vibrato (+/- 75 Hz)");
  synth.setVibrato(voice, 500, 7500);
  delay(2000);

  Serial.println("-> Augmentation de la vitesse du vibrato (10 Hz)");
  synth.setVibrato(voice, 1000, 7500);
  delay(2000);

  Serial.println("-> Désactivation du vibrato");
  synth.setVibrato(voice, 0, 0);
  delay(1000);

  synth.noteOff(voice);
}

/** @brief Démontre l'effet de glissando de fréquence (portamento). */
void demonstrateFreqSlide() {
  uint8_t voice = 0;

  Serial.println("-> Glissando ascendant d'une octave (Do4 à Do5) sur 800ms");
  // slideFreq() prépare le glissando. noteOn() le démarre.
  synth.slideFreq(voice, c4, c5, 800);
  synth.noteOn(voice, c4, 255); // La fréquence ici DOIT correspondre à la fréquence de départ du glissando.
  delay(1000);
  synth.noteOff(voice);
  delay(1000);

  Serial.println("-> Lecture d'une note aiguë (Sol5)...");
  synth.noteOn(voice, g5, 255);
  delay(1000);

  Serial.println("-> Glissando descendant vers Sol4 sur 1500ms");
  // slideFreqTo() glisse une note déjà en cours de lecture.
  synth.slideFreqTo(voice, g4, 1500);
  delay(2000);

  synth.noteOff(voice);
}

/** @brief Démontre l'effet de glissando de volume. */
void demonstrateVolumeSlide() {
  uint8_t voice = 0;

  Serial.println("-> Glissando de volume de 0 à 255 sur 1000ms (un 'swell')");
  // slideVol() te permet de définir les volumes de départ et d'arrivée.
  synth.slideVol(voice, 0, 255, 1000);
  synth.noteOn(voice, a4, 0); // La note démarre au volume initial du glissando.
  delay(1500);

  Serial.println("-> Glissando de volume vers 50 sur 800ms");
  // slideVolTo() glisse depuis le volume actuel vers une nouvelle cible.
  synth.slideVolTo(voice, 50, 800);
  delay(1200);

  synth.noteOff(voice);
}

/** @brief Démontre la combinaison de plusieurs effets. */
void demonstrateCombined() {
  uint8_t voice = 0;

  Serial.println("Lecture d'une note (Mi4) avec vibrato et trémolo...");
  synth.noteOn(voice, e4, 255);

  // Applique les deux effets en même temps
  synth.setVibrato(voice, 600, 4000);  // vitesse 6Hz, déviation 40Hz
  synth.setTremolo(voice, 400, 100);   // vitesse 4Hz, profondeur 100

  delay(2000);

  Serial.println("-> Glissando de fréquence vers Si4 pendant que les effets sont actifs");
  synth.slideFreqTo(voice, b4, 1200);
  delay(1500);

  Serial.println("-> Glissando de volume vers le bas pendant que la note joue toujours");
  synth.slideVolTo(voice, 0, 800);
  delay(1000);

  // Les effets sont toujours actifs, mais la note est silencieuse.
  // On les désactive et on arrête complètement la note.
  Serial.println("-> Désactivation des effets et de la note");
  synth.setVibrato(voice, 0, 0);
  synth.setTremolo(voice, 0, 0);
  synth.noteOff(voice);
}
```

### Explications
- **`slideFreq(voice, start, end, duration)` vs `slideFreqTo(voice, end, duration)`** : la première prépare un glissando complet (départ ET arrivée définis), à démarrer avec `noteOn()` sur la fréquence de départ ; la seconde glisse une note **déjà en train de jouer**, de sa valeur actuelle vers une nouvelle cible.
- Le même principe s'applique au volume avec `slideVol()`/`slideVolTo()`.
- Vibrato et trémolo peuvent rester actifs indépendamment des glissandos : la démo combinée montre bien que tous ces effets sont **cumulables** sur une même voix.
- Pour désactiver un vibrato ou un trémolo, il suffit de rappeler la fonction avec un `rate`/`depth` à `0`.

---

## 2. Arpeggiator — jouer des accords automatiquement en arpège

**Dossier d'origine :** `examples/Tracker Effects/Arpeggiator/`

### Ce que fait l'exemple
Joue une progression d'accords (La mineur - Sol - Do - Fa) en utilisant `setArpeggio()` pour faire jouer automatiquement, sur une seule voix, les notes de chaque accord en boucle ascendante — l'illustration parfaite de la puissance/simplicité de l'arpégiateur natif.

### Code traduit (intégral)

```cpp
/**
 * @file Arpeggiator.ino
 * @author Danilo Gabriel
 * @brief Démontre la fonctionnalité facile d'utilisation de l'arpégiateur.
 *
 * Cet exemple joue une progression d'accords simple (Am - G - C - F).
 * Pour chaque accord, il active une voix et utilise la fonction setArpeggio()
 * pour jouer automatiquement les notes de l'accord dans un motif ascendant.
 *
 * Ceci montre comment créer des motifs musicaux complexes avec très peu de code.
 *
 * Fonctionne en sortie I2S ou avec le DAC interne des ESP32 classiques.
 */

#include <Arduino.h>
#include <ESP32Synth.h>

// --- CHOISIS TON MODE DE SORTIE ---
#define USE_I2S 1 // 1 pour DAC I2S, 0 pour DAC interne (ESP32 classique uniquement)

#if USE_I2S
  const int BCK_PIN  = 26; // Bit Clock
  const int WS_PIN   = 25; // Word Select (LRC)
  const int DATA_PIN = 22; // Data Out
#else
  const int DAC_PIN = 25;
#endif

ESP32Synth synth;

// Définit la durée de chaque note de l'arpège, en millisecondes
const uint16_t ARP_SPEED_MS = 120;

void setup() {
  Serial.begin(115200);
  Serial.println("Exemple ESP32Synth - Arpégiateur");

  bool success;
  #if USE_I2S
    success = synth.begin(BCK_PIN, WS_PIN, DATA_PIN);
  #else
    #if !defined(CONFIG_IDF_TARGET_ESP32)
      Serial.println("!!! ERREUR : DAC interne disponible uniquement sur ESP32 d'origine.");
      while(1) delay(1000);
    #endif
    success = synth.begin(DAC_PIN);
  #endif

  if (!success) {
    Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
    while (1) delay(1000);
  }

  synth.setMasterVolume(200);

  // --- Configure une voix pour l'arpège ---
  uint8_t voice = 0;
  synth.setWave(voice, WAVE_PULSE);      // Une onde pulse sonne bien pour les arpèges
  synth.setPulseWidth(voice, 100);       // Largeur d'impulsion plus étroite
  synth.setEnv(voice, 5, 50, 255, 100);  // Attaque rapide, sustain maximal, release rapide

  Serial.println("Synthétiseur initialisé. Démarrage de la progression d'accords...");
}

/**
 * @brief Fonction utilitaire pour jouer un arpège sur un accord donné.
 * @param chordName Nom de l'accord (pour l'affichage).
 * @param n1 Première note de l'arpège.
 * @param n2 Deuxième note de l'arpège.
 * @param n3 Troisième note de l'arpège.
 * @param n4 Quatrième note de l'arpège.
 */
void playChordArpeggio(const char* chordName, uint32_t n1, uint32_t n2, uint32_t n3, uint32_t n4) {
    uint8_t voice = 0; // On utilise la même voix pour tous les accords

    Serial.printf("Lecture de l'arpège pour : %s\n", chordName);

    // Configure l'arpège avec les notes de l'accord.
    // Le synth jouera automatiquement n1, n2, n3, n4 puis recommencera.
    // Tu peux passer jusqu'à MAX_ARP_NOTES (16 par défaut) à cette fonction.
    synth.setArpeggio(voice, ARP_SPEED_MS, n1, n2, n3, n4);

    // noteOn démarre l'arpège. La fréquence donnée ici (n1) est la première
    // note qui sera jouée, mais l'arpégiateur prend immédiatement le contrôle.
    synth.noteOn(voice, n1, 255);

    // Laisse l'arpège jouer pendant 2 secondes
    delay(2000);

    // noteOff arrêtera le son en déclenchant l'enveloppe de relâchement.
    // On pourrait aussi utiliser detachArpeggio() si on voulait que la note
    // reste tenue sur sa dernière valeur sans continuer à arpéger.
    synth.noteOff(voice);

    // Petite pause entre les accords
    delay(200);
}

void loop() {
  // --- Joue une progression d'accords : Am - G - C - F ---

  // Arpège pour La mineur (La3, Do4, Mi4, La4)
  playChordArpeggio("Am", a3, c4, e4, a4);

  // Arpège pour Sol majeur (Sol3, Si3, Ré4, Sol4)
  playChordArpeggio("G", g3, b3, d4, g4);

  // Arpège pour Do majeur (Do4, Mi4, Sol4, Do5)
  playChordArpeggio("C", c4, e4, g4, c5);

  // Arpège pour Fa majeur (Fa3, La3, Do4, Fa4)
  playChordArpeggio("F", f3, a3, c4, f4);

  Serial.println("\nProgression terminée. Reprise dans 5 secondes...\n");
  delay(5000);
}
```

### Explications
- `setArpeggio(voice, durationMs, freq1, freq2, ..., freqN)` accepte un nombre **variable** d'arguments (grâce aux templates variadiques C++), jusqu'à `MAX_ARP_NOTES` (16 par défaut, configurable dans `ESP32Synth_Config.hpp`).
- Une fois l'arpège configuré, un simple `noteOn()` sur la voix suffit à démarrer automatiquement le cycle des notes — le moteur se charge en interne de changer la fréquence toutes les `durationMs` millisecondes, sans intervention supplémentaire du code utilisateur.
- `detachArpeggio(voice)` permettrait de "figer" la voix sur sa dernière note jouée, en désactivant l'arpège sans couper le son (contrairement à `noteOff()` qui déclenche le relâchement complet de l'enveloppe).
