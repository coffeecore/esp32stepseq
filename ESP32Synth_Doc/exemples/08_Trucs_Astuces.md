# 🧠 Exemples : trucs et astuces avancés

Couvre : `RT_FM_on_Wavetables`, `SVF_filters_on_wavetable`.

Ces deux exemples partagent une même idée astucieuse : **précalculer** des variantes de timbre (FM ou filtrage) sous forme de **plusieurs wavetables statiques**, puis les faire défiler dans le temps via un `Instrument` (§10 du guide API) — plutôt que de calculer la FM ou le filtre en temps réel dans la boucle audio. C'est un excellent compromis performance/qualité pour obtenir des timbres évolutifs riches sans surcharger le CPU.

---

## 1. RT_FM_on_Wavetables — banques FM précalculées jouées façon tracker

**Dossier d'origine :** `examples/Tricks/FM/RT_FM_on_Wavetables/`

### Ce que fait l'exemple
Génère au démarrage 3 **banques de 16 wavetables FM** (nappe/pad, pluck/arpège, basse), chaque banque représentant la même onde FM avec une profondeur de modulation décroissante (de très métallique à presque pur). Ces 16 étapes sont ensuite enchaînées automatiquement par un `Instrument` avec interpolation douce (`smoothMorph = true`), simulant un filtre qui "s'ouvre" ou un timbre qui évolue — sans jamais calculer de FM en temps réel dans la voix.

### Code traduit (extraits clés)

```cpp
#include <Arduino.h>
#include <ESP32Synth.h>
#include <math.h>

ESP32Synth synth;

int16_t* wt_pad[16];
int16_t* wt_pluck[16];
int16_t* wt_bass[16];

// Accords du morceau relaxant
const uint32_t chords[4][4] = {
    {c3, ds3, g3, c4},    // Do min
    {gs2, c3, ds3, g3},   // La♭ maj
    {ds3, g3, as3, ds4},  // Mi♭ maj
    {as2, d3, f3, as3}    // Si♭ maj
};

// ====================================================================================
// == INSTRUMENTS ET ENVELOPPES
// ====================================================================================

// --- NAPPE / PAD (ID 0 à 15) ---
const int16_t padWaves[16]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const uint8_t padVols[16]    = {60, 90, 120, 150, 130, 110, 90, 80, 70, 60, 55, 50, 45, 40, 35, 30};
const int16_t padRelWaves[4] = {15, 15, 15, 15};
const uint8_t relVols[4]     = {20, 10, 5, 0};

Instrument padInst = {
    .seqVolumes = padVols, .seqWaves = padWaves, .relVolumes = relVols, .relWaves = padRelWaves,
    .seqSpeedMs = 150, .susWave = 15, .relSpeedMs = 300, .seqLen = 16, .susVol = 25, .relLen = 4,
    .smoothMorph = true, .smoothVolume = true
};

// --- PLUCK / ARPÈGE (ID 16 à 31) ---
const int16_t pluckWaves[8]   = {16, 18, 20, 22, 24, 26, 28, 31};
const uint8_t pluckVols[8]    = {150, 90, 50, 25, 10, 5, 0, 0};
const int16_t pluckRelWaves[1]= {31};

Instrument pluckInst = {
    .seqVolumes = pluckVols, .seqWaves = pluckWaves, .relVolumes = relVols, .relWaves = pluckRelWaves,
    .seqSpeedMs = 60, .susWave = 31, .relSpeedMs = 50, .seqLen = 8, .susVol = 0, .relLen = 1,
    .smoothMorph = true, .smoothVolume = true
};

// --- BASSE (ID 32 à 47) ---
const int16_t bassWaves[16]   = {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
const uint8_t bassVols[16]    = {200, 190, 180, 160, 140, 120, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10};
const int16_t bassRelWaves[4] = {47, 47, 47, 47};

Instrument bassInst = {
    .seqVolumes = bassVols, .seqWaves = bassWaves, .relVolumes = relVols, .relWaves = bassRelWaves,
    .seqSpeedMs = 120, .susWave = 47, .relSpeedMs = 200, .seqLen = 16, .susVol = 10, .relLen = 4,
    .smoothMorph = true, .smoothVolume = true
};

// ====================================================================================
// == MOTEUR FM OPTIMISÉ (génération hors ligne, une seule fois au démarrage)
// ====================================================================================
void generateFMBank(int16_t* dest[16], float ratioC, float ratioM, float maxDepth, float amplitude) {
    for (int w = 0; w < 16; w++) {
        float depth = maxDepth * (1.0f - ((float)w / 15.0f));
        for (int i = 0; i < 512; i++) {
            float phaseC = ((float)i / 512.0f) * 2.0f * PI * ratioC;
            float phaseM = ((float)i / 512.0f) * 2.0f * PI * ratioM;
            // sinf() force l'utilisation du FPU de l'ESP32 pour des calculs extrêmement rapides
            dest[w][i] = (int16_t)(sinf(phaseC + depth * sinf(phaseM)) * amplitude);
        }
    }
}

// ====================================================================================
// == TÂCHE EN ARRIÈRE-PLAN (CORE 0) - MUSIQUE ET MATHÉMATIQUES DE LFO
// ====================================================================================
void fmMusicTask(void* param) {
    unsigned long lastSequencerTick = 0;
    unsigned long lastTimbreTick = 0;
    int tickCount = 0;
    int chordIdx = 0;
    // ... (boucle qui avance dans la progression d'accords et pilote les instruments)
}
```

### Explications
- **Pourquoi précalculer la FM plutôt que la faire en temps réel ?** Calculer du `sinf()` (FPU flottant) pour chaque échantillon de chaque voix serait coûteux à haute polyphonie. En précalculant 16 étapes de FM avec une profondeur de modulation décroissante (`depth = maxDepth * (1 - w/15)`), on obtient un "morphing" de timbre extrêmement réaliste (le son s'assagit progressivement, comme une enveloppe de filtre qui se referme) **sans aucun calcul flottant pendant la lecture** — seule une lecture de wavetable simple (entière) est nécessaire à l'exécution.
- `smoothMorph = true` dans la structure `Instrument` active l'**interpolation** entre deux wavetables consécutives de la séquence, ce qui lisse la transition entre les 16 étapes et évite tout artefact de "saut" audible.
- Les trois instruments (`padInst`, `pluckInst`, `bassInst`) illustrent bien comment on peut concevoir des timbres très différents (nappe longue et douce, pluck court et brillant, basse profonde) simplement en jouant sur `seqSpeedMs`, `seqLen`, `susVol` et la forme des courbes de volume.
- La génération (`generateFMBank`) tourne une seule fois dans `setup()`, ce qui justifie l'usage de `float`/`sinf()` ici — cette section n'est **pas** dans le chemin audio critique, contrairement à la règle générale du guide API sur l'arithmétique à virgule fixe.

---

## 2. SVF_filters_on_wavetable — démonstration pédagogique passe-bas / passe-bande / passe-haut

**Dossier d'origine :** `examples/Tricks/Filters/SVF_filters_on_wavetable/`

### Ce que fait l'exemple
Génère une onde dent de scie et une onde pulse de base, puis leur applique **hors ligne** (dans `setup()`/en tâche de fond) un filtre SVF (State-Variable Filter) réglable, produisant des wavetables filtrées en passe-bas, passe-bande et passe-haut. Une petite machine à états fait défiler une démonstration pédagogique (passe-bas, puis passe-bande, puis passe-haut) avant de jouer un morceau utilisant ces timbres filtrés. Un système de **double buffering** (`wtFilteredLP[2][...]`) permet de recalculer une nouvelle version du filtre en arrière-plan sans jamais interrompre le son en cours de lecture.

### Code traduit (extraits clés)

```cpp
#include <Arduino.h>
#include <ESP32Synth.h>

ESP32Synth synth;

#define WT_SIZE 512

// ==============================================================================
// ONDES DE BASE EN FLOTTANT
// ==============================================================================
float baseSaw[WT_SIZE];
float basePulse[WT_SIZE];

// ==============================================================================
// DOUBLE BUFFERING POUR LES WAVETABLES
// ==============================================================================
int16_t wtFilteredLP[2][WT_SIZE];
int16_t wtFilteredBP[2][WT_SIZE];
int16_t wtFilteredHP[2][WT_SIZE];
volatile uint8_t activeBuf[3] = {0, 0, 0};

volatile uint32_t noteOnTime[3] = {0, 0, 0};
volatile bool     noteActive[3] = {false, false, false};

// ==============================================================================
// MACHINE À ÉTATS (pour organiser la démonstration avant le morceau)
// ==============================================================================
enum AppState {
    DEMO_LOW_PASS,
    DEMO_BAND_PASS,
    DEMO_HIGH_PASS,
    PLAY_MUSIC
};
AppState currentState = DEMO_LOW_PASS;
bool stateTriggered = false;
uint32_t stateTimer = 0;

// ==============================================================================
// GÉNÉRATEUR D'ONDES DE BASE
// ==============================================================================
void generateBaseWaves() {
    for (int i = 0; i < WT_SIZE; i++) {
        baseSaw[i] = (i / (float)WT_SIZE) * 2.0f - 1.0f;
        basePulse[i] = (i < WT_SIZE / 2) ? 1.0f : -1.0f;
    }
}

// ==============================================================================
// FILTRE SVF MUSICAL (OPTIMISÉ POUR NE PAS SATURER)
// ==============================================================================
void applyFilter(const float* input, int16_t* output, int mode, float cutoff, float resonance) {
    if (cutoff < 0.002f) cutoff = 0.002f; // Laisse passer les graves profonds
    if (cutoff > 0.45f)  cutoff = 0.45f;

    float f = 2.0f * sin(PI * cutoff / 2.0f);
    float q = 1.0f - resonance;
    float lp = 0, bp = 0, hp = 0;

    // Passe 1 : stabilise le filtre (enlève le "clic" de démarrage)
    for (int i = 0; i < WT_SIZE; i++) {
        hp = input[i] - lp - q * bp;
        bp += f * hp;
        lp += f * bp;
    }

    float outBuf[WT_SIZE];
    float maxPeak = 0.001f;

    // Passe 2 : calcule pour de bon
    for (int i = 0; i < WT_SIZE; i++) {
        hp = input[i] - lp - q * bp;
        bp += f * hp;
        lp += f * bp;

        float out = (mode == 0) ? lp : (mode == 1) ? bp : hp;
        outBuf[i] = out;

        float absOut = abs(out);
        if (absOut > maxPeak) maxPeak = absOut;
    }

    // Gain automatique pour que la résonance ne sature pas l'audio
    float gain = (maxPeak > 1.0f) ? (0.95f / maxPeak) : 1.0f;

    for (int i = 0; i < WT_SIZE; i++) {
        int32_t sample = (int32_t)(outBuf[i] * gain * 32767.0f);
        if (sample > 32767) sample = 32767;
        else if (sample < -32768) sample = -32768;
        output[i] = (int16_t)sample;
    }
}
```

### Explications
- **Deux passes de filtrage** : la première "passe à vide" (sans conserver la sortie) sert uniquement à laisser le filtre atteindre un régime stable (éviter le "clic" transitoire de démarrage) ; la seconde passe recalcule ensuite pour de vrai et produit la sortie utile — une astuce classique pour filtrer une **wavetable bouclée** (qui n'a pas de vrai "début") sans discontinuité audible à la jointure.
- Le **gain automatique** (`gain = 0.95f / maxPeak` si nécessaire) est important avec un filtre résonant : à haute résonance (`q` faible), le signal peut largement dépasser l'amplitude d'origine — ce recalcul de gain évite l'écrêtage (clipping) tout en gardant l'effet sonore de la résonance.
- Le **double buffering** (`wtFilteredLP[2][WT_SIZE]`, avec un index `activeBuf` qui bascule) permet de recalculer un nouveau filtrage (par exemple à une nouvelle fréquence de coupure) dans un buffer inactif pendant qu'une voix lit encore l'ancien buffer actif — évitant ainsi tout artefact audio pendant le recalcul, une technique essentielle dès qu'on modifie des wavetables "à la volée".
- La machine à états (`AppState`) structure la démo en présentant d'abord chaque mode de filtre isolément (passe-bas, passe-bande, passe-haut) avant de les utiliser ensemble dans un morceau — une bonne pratique pédagogique à réutiliser dans tes propres exemples.
