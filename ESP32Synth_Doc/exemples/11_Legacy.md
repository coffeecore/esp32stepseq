# 🕰️ Exemples : « Legacy » (démonstrations historiques du projet)

Couvre : `Square_pwm`, `JSBach_Toccata-d-AsmSynth-To-ESP32Synth`, `Aditive_square`, `Super_Saw`.

Ces exemples sont classés "Legacy" par l'auteur car ils illustrent des techniques plus anciennes ou plus simples du projet (souvent multi-voix intensif), mais restent d'excellentes références pédagogiques.

---

## 1. Square_pwm — balayage de largeur d'impulsion (PWM)

**Dossier d'origine :** `examples/Legacy/Square_pwm/`

### Ce que fait l'exemple
Joue une note grave tenue (Do2) en onde pulse, et fait varier en continu sa largeur d'impulsion (pulse width) d'étroite à large et inversement — ce balayage change le contenu harmonique du son, créant un classique effet de "balayage"/"phasing".

### Code traduit (intégral)

```cpp
/**
 * @file Square_pwm.ino
 * @author Danilo Gabriel
 * @brief Démontre la modulation de largeur d'impulsion (PWM) sur une onde carrée.
 *
 * Cet exemple joue une note grave constante (Do2) avec une forme d'onde pulse. Il
 * balaie ensuite en continu la largeur d'impulsion d'étroite à large et inversement.
 * Cette modulation de la largeur d'impulsion change le contenu harmonique du son,
 * créant un classique effet de "balayage" ou de "phasing".
 *
 * Utilise la sortie PDM par défaut sur la broche 5.
 */
#include "ESP32Synth.h"
#include "ESP32SynthNotes.h"

ESP32Synth synth;

// --- Variables de contrôle du PWM ---
int currentPW = 128; // Largeur d'impulsion actuelle (0-255). Démarre à 50% (128).
int direction = 1;   // Sens du balayage : 1 pour augmenter la largeur, -1 pour diminuer.

const int BCK_PIN = 19;
const int WS_PIN = 5;
const int DATA_PIN = 18;

/**
 * @brief Initialise le synthétiseur et configure le son.
 */
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialise le synth (attention : le fichier source original contient une petite
  // erreur de syntaxe ici — un "s" en trop après la parenthèse fermante :
  // "if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)s) {" — il faut lire simplement
  // "if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {" pour que ça compile).
  if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {
    Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
    while(1) delay(1000);
  }

  // --- Configure la voix 0 pour l'effet PWM ---
  synth.setWave(0, WAVE_PULSE);       // Forme d'onde pulse.
  synth.setEnv(0, 10, 0, 127, 100);   // Enveloppe façon orgue (attaque quasi instantanée, sustain plein).
  synth.setVolume(0, 200);            // Volume élevé.

  // Joue une note grave Do2 pour bien entendre l'effet PWM.
  synth.noteOn(0, c2, 200);
  Serial.println("Synth initialisé. Lecture de Do2 avec balayage PWM.");
}

/**
 * @brief Boucle principale qui module en continu la largeur d'impulsion.
 */
void loop() {
  // Met à jour la largeur d'impulsion.
  currentPW += direction;

  // Inverse le sens du balayage quand la largeur atteint ses limites.
  // On évite les extrêmes absolus (0 et 255) car le son disparaîtrait.
  if (currentPW >= 254) {
    direction = -1; // Commence à diminuer.
  }
  else if (currentPW <= 2) {
    direction = 1;  // Commence à augmenter.
  }

  // Applique la nouvelle largeur d'impulsion à la voix 0.
  synth.setPulseWidth(0, currentPW);

  // Un petit délai contrôle la vitesse de l'effet de balayage.
  // - Un délai plus petit (ex. 10) crée un balayage rapide et agressif.
  // - Un délai plus grand (ex. 30) crée un balayage lent et détaillé.
  delay(15);
}
```

### Explications
- Une onde `WAVE_PULSE` proche de 0% ou 100% de largeur d'impulsion tend vers le silence (impulsion infiniment étroite ou infiniment large) — c'est pourquoi le code borne le balayage entre 2 et 254 plutôt que 0 et 255.
- Ce balayage de PWM est l'une des techniques les plus simples et efficaces pour donner du mouvement à un son statique, sans toucher à la fréquence ni au volume.

---

## 2. JSBach_Toccata-d-AsmSynth-To-ESP32Synth — Toccata et Fugue en Ré mineur (BWV 565)

**Dossier d'origine :** `examples/Legacy/JSBach_Toccata-d-AsmSynth-To-ESP32Synth/`

### Ce que fait l'exemple
Un portage complet (1540 lignes) de la célèbre Toccata et Fugue en Ré mineur de J.S. Bach, à l'origine écrite pour le projet **AsmSynth**, adaptée ici pour ESP32Synth. Le fichier contient une séquence de notes codée en dur, multi-voix, simulant un playback façon tracker.

### En-tête traduit

```cpp
/**
 * @file JSBach_Toccata-d-AsmSynth-To-ESP32Synth.ino
 * @author Danilo Gabriel
 * @brief Joue la Toccata et Fugue en Ré mineur de J.S. Bach, BWV 565.
 *
 * Cet exemple est un portage d'un morceau créé à l'origine pour le projet AsmSynth.
 * Il démontre la capacité du synthétiseur à jouer de la musique complexe et
 * multi-voix, en utilisant une séquence de notes et de rythmes codée en dur.
 *
 * Il utilise un ensemble personnalisé de définitions de notes et de fonctions
 * utilitaires pour contrôler plusieurs voix, simulant une lecture façon tracker.
 *
 * Utilise la sortie I2S. Assure-toi d'avoir un DAC I2S connecté.
 * - BCK_PIN  : broche d'horloge de bits I2S
 * - WS_PIN   : broche de sélection de mot (LRC) I2S
 * - DATA_PIN : broche de données I2S
 */
#include "ESP32Synth.h"

// --- Configuration des broches pour DAC I2S ---
// --- Tu DOIS changer ces broches pour correspondre à ton câblage ---
const int BCK_PIN = 26;
const int WS_PIN = 25;
const int DATA_PIN = 22;

ESP32Synth synth;

// --- Définitions des notes (issues du portage AsmSynth d'origine) ---
// Ces définitions sont locales à ce sketch pour rester compatibles
// avec les données du morceau d'origine. Elles peuvent différer des
// définitions standards de ESP32SynthNotes.h (ex. 'h1' est utilisé pour 'b1').
#define c0 1635
#define cs0 1732
#define d0 1835
// ... (toutes les octaves suivent, identiques en valeur à ESP32SynthNotes.h,
//      mais parfois nommées différemment selon la convention allemande où
//      'h' désigne le Si et 'b' le Si bémol)
```

### Explications
- L'immense majorité du fichier (plus de 1500 lignes) est constituée de **données musicales pures** : des tableaux de fréquences et de durées représentant chaque voix de la partition, note après note — ce n'est pas du code "à comprendre" au sens algorithmique, mais une transcription directe de la partition.
- Le morceau ne comporte que **16 commentaires** dans tout le fichier : le reste est la partition elle-même. C'est un excellent exemple de la capacité d'ESP32Synth à gérer un morceau polyphonique complexe et long simplement via des appels `noteOn`/`noteOff` séquencés dans le temps.
- Remarque la petite note de compatibilité : le portage utilise `h1` au lieu de `b1` pour certaines notes — un vestige de la **convention allemande de notation** (où "H" désigne le Si naturel et "B" le Si bémol), différente de la convention anglo-saxonne utilisée par défaut dans `ESP32SynthNotes.h`.
- Si tu veux adapter une partition existante (MIDI, musicXML, etc.) vers ESP32Synth, cet exemple montre le principe général : convertir chaque note en une paire (fréquence CentiHz, durée), puis dérouler ces paires via `noteOn()`/`delay()`/`noteOff()` pour chaque voix.

---

## 3. Aditive_square — construire un carré par synthèse additive

**Dossier d'origine :** `examples/Legacy/Aditive_square/`

### Ce que fait l'exemple
Démontre les principes de la **synthèse additive** en construisant une onde carrée harmonique par harmonique, en utilisant plusieurs voix, chacune jouant un sinus à un harmonique impair de la fréquence fondamentale. Le volume de chaque harmonique est inversement proportionnel à son rang (1/n), conformément à la série de Fourier caractéristique d'une onde carrée. L'exemple ajoute et retire progressivement les harmoniques pour qu'on entende le timbre se "construire" et se "déconstruire" en direct.

### Code traduit (extraits clés)

```cpp
/**
 * @file Aditive_square.ino
 * @author Danilo Gabriel
 * @brief Synthétiseur additif qui construit une onde carrée à partir de sinusoïdes.
 *
 * Cet exemple démontre les principes de la synthèse additive en construisant
 * une onde carrée harmonique par harmonique. Il utilise plusieurs voix, chacune
 * jouant un sinus à un harmonique impair de la fréquence fondamentale.
 *
 * Le volume de chaque harmonique est inversement proportionnel à son rang (1/n),
 * ce qui est caractéristique de la série de Fourier d'une onde carrée.
 *
 * Cet exemple est "legacy" car il sollicite le synth avec de nombreuses voix.
 * Le nombre d'harmoniques est limité par MAX_VOICES dans la bibliothèque (défaut 6).
 */

#include "ESP32Synth.h"

ESP32Synth synth;

// --- Paramètres de synthèse ---
#define BASE_FREQ_CENTIHZ c4 // Note de base pour l'onde carrée (ex. Do4 depuis ESP32SynthNotes.h)

// IMPORTANT : le nombre de voix utilisées pour les harmoniques ne peut pas dépasser
// MAX_VOICES défini dans ESP32Synth.h (par défaut 6).
// On utilise toutes les voix disponibles pour construire l'onde.
#define MAX_VOICES_USE 6

#define STEP_DELAY 250      // Temps entre l'ajout/retrait de chaque harmonique (ms)
#define HOLD_TIME 3000      // Temps de maintien de l'onde complète avant déconstruction (ms)

// --- Contrôle d'état ---
unsigned long lastTime = 0;
int currentVoice = 0;
bool building = true;       // Est-on en train de construire (true) ou déconstruire (false) l'onde ?
bool holding = false;       // Est-on en pause au début ou à la fin d'un cycle ?

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Exemple ESP32Synth - Onde carrée additive");

    // --- Initialise le synthétiseur ---
    // ... (initialisation I2S ou DAC selon USE_I2S, identique aux autres exemples)

    synth.setMasterVolume(200);
    Serial.printf("Utilisation de %d voix (harmoniques) pour la synthèse.\n", MAX_VOICES_USE);

    // Configuration initiale pour toutes les voix qui seront utilisées
    for(int i = 0; i < MAX_VOICES_USE; i++) {
        synth.setWave(i, WAVE_SINE);
        // Enveloppe façon orgue (pas d'attaque, sustain plein, release rapide)
        synth.setEnv(i, 0, 0, 255, 50);
    }
    Serial.println("Synth initialisé. Démarrage du cycle de synthèse...");
}
```

### Explications
- La **série de Fourier d'une onde carrée** ne contient que des harmoniques **impairs** (1, 3, 5, 7...) de la fondamentale, chacun avec une amplitude en `1/n`. En sommant de plus en plus d'harmoniques sinusoïdaux avec ces proportions précises, on se rapproche progressivement d'une vraie onde carrée — cet exemple le rend audible en ajoutant les harmoniques un par un dans le temps.
- C'est un excellent complément pédagogique à `BasicWaveforms` : là où `WAVE_SAW`/`WAVE_PULSE` fournissent directement une forme d'onde toute faite, cet exemple montre **comment ces formes d'onde peuvent être reconstruites depuis zéro** par sommation de sinusoïdes — le principe fondamental derrière toute synthèse additive (orgues Hammond compris, cf. `HammondB3_Leslie122`).
- La limite `MAX_VOICES_USE = 6` illustre concrètement la contrainte de `MAX_VOICES` (§2 du guide API) : plus tu veux d'harmoniques précis, plus il te faut de voix disponibles.

---

## 4. Super_Saw — la "Super Saw" par empilement de dents de scie désaccordées

**Dossier d'origine :** `examples/Legacy/Super_Saw/`

### Ce que fait l'exemple
Recrée le son "Super Saw" emblématique de la musique électronique (popularisé par le Roland JP-8000) en empilant 7 voix en dent de scie légèrement désaccordées les unes des autres, puis joue une progression d'accords simple.

### Code traduit (intégral)

```cpp
/**
 * @file Super_Saw.ino
 * @author Danilo Gabriel
 * @brief Crée un son "Super Saw" en superposant et désaccordant des dents de scie.
 *
 * Cet exemple démontre comment créer un patch "Super Saw" riche et épais,
 * un son célèbre en musique électronique. Il fonctionne en jouant la même note
 * sur plusieurs voix, toutes réglées en forme d'onde dent de scie.
 *
 * Le cœur de l'effet vient du léger désaccordage de chaque voix par rapport à
 * la fréquence fondamentale. Cela crée un effet façon chorus qui rend le son
 * large et puissant.
 *
 * Utilise la sortie PDM par défaut sur la broche 5.
 */
#include "ESP32Synth.h"
#include "ESP32SynthNotes.h"

ESP32Synth synth;

const int BCK_PIN = 19;
const int WS_PIN = 5;
const int DATA_PIN = 18;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialise le synth sur la sortie PDM par défaut (broche 5), tourne sur le Core 1.
    if (!synth.begin(BCK_PIN, WS_PIN, DATA_PIN)) {
        Serial.println("!!! ERREUR : échec de l'initialisation du synthétiseur.");
        while(1) delay(1000);
    }

    // Configure 7 voix pour faire partie du patch Super Saw.
    for(int i = 0; i < 7; i++) {
        synth.setWave(i, WAVE_SAW);       // Toutes les voix en dent de scie.
        synth.setEnv(i, 50, 0, 127, 500); // Attaque légèrement douce et release longue.
    }
    Serial.println("Synth initialisé. Lecture de la progression d'accords Super Saw.");
}

/**
 * @brief Boucle principale qui joue une progression d'accords simple.
 */
void loop() {
  playSaw(c2);
  delay(2000);
  playSaw(e2);
  delay(2000);
  playSaw(g2);
  delay(2000);
  playSaw(a2);
  delay(2000);
}

/**
 * @brief Joue une note avec l'effet Super Saw.
 * @param baseNote La note fondamentale à jouer (ex. c2, a3).
 */
void playSaw(uint32_t baseNote){
    // Arrête toute note précédente pour garantir un départ propre.
    for(int i = 0; i < 7; i++) {
        synth.noteOff(i);
    }

    // --- Empile les dents de scie désaccordées ---
    // Les valeurs de fréquence sont en CentiHertz (Hz * 100).
    // Les petits décalages (+150, -200, etc.) créent le désaccordage.

    // Voix 0 : la note centrale, fondamentale.
    synth.noteOn(0, baseNote, 100);

    // Voix 1 & 2 : légèrement désaccordées, "panoramiquées" via le volume.
    synth.noteOn(1, baseNote + 150, 80); // Désaccordée vers le haut de 1.5 Hz
    synth.noteOn(2, baseNote - 150, 80); // Désaccordée vers le bas de 1.5 Hz

    // Voix 3 & 4 : plus désaccordées, volume plus faible.
    synth.noteOn(3, baseNote + 200, 70); // Désaccordée vers le haut de 2.0 Hz
    synth.noteOn(4, baseNote - 200, 70); // Désaccordée vers le bas de 2.0 Hz

    // Voix 5 & 6 : les plus désaccordées, volume le plus faible.
    synth.noteOn(5, baseNote + 300, 60); // Désaccordée vers le haut de 3.0 Hz
    synth.noteOn(6, baseNote - 300, 60); // Désaccordée vers le bas de 3.0 Hz
}
```

### Explications
- Le principe de la Super Saw est simple mais puissant : **plusieurs oscillateurs identiques légèrement désaccordés** créent des battements lents entre eux (interférences constructives/destructives progressives), donnant une sensation de largeur et de mouvement qu'un seul oscillateur ne peut pas produire.
- Le volume décroissant à mesure que le désaccordage augmente (100 → 80 → 70 → 60) évite que les voix les plus désaccordées ne dominent le mélange et cassent la perception d'une seule note "épaisse" plutôt que plusieurs notes dissonantes.
- Comparé au moteur "String Ensemble" vu dans `JunoAndDX7` (§3 des exemples de formes d'onde) qui fait la même chose **en interne dans un seul callback `WAVE_CUSTOM`** avec 3 oscillateurs, cette version "Legacy" utilise l'approche la plus simple et directe : **une voix ESP32Synth = un oscillateur**, ce qui est plus lisible pour débuter mais consomme davantage de voix du budget `MAX_VOICES`.
