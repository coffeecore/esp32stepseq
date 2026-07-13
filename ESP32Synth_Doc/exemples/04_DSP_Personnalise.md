# 🎚️ Exemples : effets DSP personnalisés (hooks globaux)

Couvre : `Simple_Reverb`, `Acid_TB-303_SVF_Filter`, `BBD_Chorus`.

Ces trois exemples illustrent le mécanisme `setCustomDSP()` (§14 du guide API) : une fonction appliquée sur le **bus de mixage global** (`int32_t* mixBuffer`) après que toutes les voix ont été sommées, avant la conversion finale pour le DAC. Ils utilisent aussi souvent `setCustomControl()`, un second hook appelé au taux de contrôle (100 Hz par défaut) pour piloter la logique musicale (séquenceur, accords...) en parallèle du DSP audio.

---

## 1. Simple_Reverb — réverbération à 3 têtes (tape delay)

**Dossier d'origine :** `examples/Custom DSP/Simple_Reverb/`

Cet exemple est identique au moteur de réverbération utilisé dans `SimpleWaves` (voir `exemples/01_Formes_Ondes_Bases.md`), présenté ici isolément comme exemple dédié au DSP global. Se reporter à la section 2 de ce fichier pour le code commenté en détail (fonction `reverbDSP`, buffer circulaire de 20 000 échantillons, DC blocker, filtre passe-bas, saturation de sécurité).

---

## 2. Acid_TB-303_SVF_Filter — filtre résonant façon Roland TB-303

**Dossier d'origine :** `examples/Custom DSP/Acid_TB-303_SVF_Filter/`

### Ce que fait l'exemple
Recrée le son "acid" caractéristique de la Roland TB-303 : un séquenceur de 16 pas joue une ligne de basse en dent de scie, tandis qu'un **filtre SVF (State-Variable Filter) résonant** — modulé par une enveloppe de coupure (cutoff) à décroissance rapide — sculpte le timbre à chaque note.

### Code traduit

```cpp
#include <ESP32Synth.h>

ESP32Synth synth;

// ==============================================================================
// MOTEUR "ACID VCF" (filtre SVF de Chamberlin optimisé en virgule fixe)
// ==============================================================================
int32_t bpState = 0;   // État passe-bande
int32_t lpState = 0;   // État passe-bas
int32_t vcfCutoff = 0; // Dynamique : 10 à 120 (sûr pour 48kHz)
const int32_t vcfRes = 35; // Résonance : plus la valeur est PETITE, plus le filtre "crie" (0 à 255)

void IRAM_ATTR acidDSP(int32_t* mixBuffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        int32_t input = mixBuffer[i] >> 2; // Réduit le gain initial pour laisser de la place au filtre

        // FILTRE SVF (résonant 2 pôles) avec décalages de bits
        int32_t hp = input - lpState - ((bpState * vcfRes) >> 8);
        bpState += (vcfCutoff * hp) >> 8;
        lpState += (vcfCutoff * bpState) >> 8;

        // Limite la résonance pour ne pas faire exploser les calculs (stabilité)
        if(bpState > 32767) bpState = 32767; else if(bpState < -32768) bpState = -32768;
        if(lpState > 32767) lpState = 32767; else if(lpState < -32768) lpState = -32768;

        int32_t output = lpState;
        mixBuffer[i] = output;
    }
}

// ==============================================================================
// 2. LE SÉQUENCEUR ACID
// ==============================================================================
uint32_t acidSeq[16]  = { c2, c3, 0, ds2, c2, f2, 0, c2, c2, gs2, 0, as2, c2, c3, ds3, 0 };
uint8_t  acidAcc[16]  = { 1,  0,  0, 1,   0,  1,  0, 0,  1,  1,   0, 0,   1,  0,  1,   0 };

uint32_t tick = 0;
uint8_t  step = 0;

void IRAM_ATTR acidControl() {
    tick++;

    // Enveloppe du filtre (decay agressif)
    if (vcfCutoff > 15) vcfCutoff -= 6;

    if (tick % 12 == 0) {
        uint32_t note = acidSeq[step];
        if (note > 0) {
            // S'il y a un "accent", l'enveloppe s'ouvre au maximum.
            if (acidAcc[step]) {
                vcfCutoff = 110;
                synth.noteOn(0, note, 255);
            } else {
                vcfCutoff = 60;
                synth.noteOn(0, note, 150);
            }
        }
        step = (step + 1) & 15; // "& 15" remplace "% 16" (beaucoup plus rapide)
    }
}

// ==============================================================================
// SETUP PRINCIPAL
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    synth.setWave(0, WAVE_SAW);
    synth.setEnv(0, 5, 200, 0, 50); // Pluck rapide

    synth.setCustomDSP(acidDSP);
    synth.setCustomControl(acidControl);

    // Ajuste tes broches ici ! (BCLK, WS, DATA)
    if (synth.begin(25)) {
        Serial.println("Acid TB-303 (filtre SVF corrigé) en cours d'exécution.");
    }
}

void loop() {
    delay(1000);
}
```

### Explications
- Le filtre utilisé est un **SVF de Chamberlin** (State-Variable Filter), une topologie classique permettant d'obtenir simultanément une sortie passe-bas, passe-bande et passe-haut à partir des deux mêmes états récursifs (`bpState`, `lpState`). Ici seule la sortie passe-bas est utilisée.
- La variable `vcfCutoff` (fréquence de coupure) est modulée par une **enveloppe "faite main"** dans `acidControl()` : elle est réinitialisée à chaque note (plus haute si la note a un "accent") puis décroît progressivement (`vcfCutoff -= 6` à chaque tick de contrôle) — c'est exactement le principe de l'enveloppe de filtre caractéristique du son "acid" de la TB-303.
- Le séquenceur (`acidSeq[16]`, `acidAcc[16]`) illustre un pattern très classique en musique électronique : un tableau de fréquences (0 = silence/pas de note) et un tableau parallèle d'accents (0 ou 1) qui module le volume et l'ouverture du filtre.
- `synth.begin(25)` initialise ici en mode **DAC interne** sur la broche 25 (ESP32 classique uniquement).

---

## 3. BBD_Chorus — chorus analogique façon "Bucket-Brigade Device"

**Dossier d'origine :** `examples/Custom DSP/BBD_Chorus/`

### Ce que fait l'exemple
Simule un chorus analogique à ligne à retard modulée (comme les circuits BBD des pédales analogiques vintage) et l'applique à une nappe (pad) de 4 voix jouant des accords enrichis (7e majeure, mineure, dominante), avec une largeur d'impulsion modulée doucement dans le temps.

### Code traduit

```cpp
#include <ESP32Synth.h>

ESP32Synth synth;

// ==============================================================================
// 1. MOTEUR "BBD CHORUS DOUX"
// ==============================================================================
#define CHORUS_LEN 1024
#define CHORUS_MASK 1023

int32_t chorusTape[CHORUS_LEN] = {0};
int cHead = 0;
uint32_t lfoPhase = 0;

void IRAM_ATTR chorusDSP(int32_t* mixBuffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        int32_t dry = mixBuffer[i];

        // 1. LFO TRIANGULAIRE LENT (~1.5Hz)
        lfoPhase += 2;
        int32_t lfoTri = (lfoPhase & 0x7FFF);
        if (lfoPhase & 0x8000) lfoTri = 32767 - lfoTri;

        // 2. RETARD DOUX (entre 50 et 200 échantillons = ~1ms à ~4ms)
        int delayOffset = 50 + ((lfoTri * 150) >> 15);

        int readHead = (cHead - delayOffset) & CHORUS_MASK;
        int32_t wet = chorusTape[readHead];

        chorusTape[cHead] = dry;
        cHead = (cHead + 1) & CHORUS_MASK;

        // Mixe 50% / 50% sans faire exploser le volume
        mixBuffer[i] = (dry >> 1) + (wet >> 1);
    }
}

// ==============================================================================
// 2. GÉNÉRATEUR D'ACCORDS
// ==============================================================================
uint32_t padChords[4][4] = {
    {c3,  e3,  g3,  b3},  // Do maj7
    {a2,  c3,  e3,  g3},  // La min7
    {f2,  a2,  c3,  e3},  // Fa maj7
    {g2,  b2,  d3,  f3}   // Sol7
};

uint32_t tick = 0;
uint8_t currentChord = 0;

void IRAM_ATTR dreamControl() {
    tick++;

    // LFO très lent pour donner du mouvement à la largeur d'impulsion (PWM)
    uint8_t pwmWidth = 128 + ((tick % 200) > 100 ? (tick % 100) : (100 - (tick % 100)));
    for(int i = 0; i < 4; i++) synth.setPulseWidth(i, pwmWidth);

    // Change d'accord toutes les 300 ticks (3 secondes)
    if (tick % 300 == 0) {
        currentChord = (currentChord + 1) & 3; // Boucle entre 0, 1, 2, 3 dans l'ordre

        // Déclenche les accords proprement (sans glissando de hauteur, pour éviter les dissonances)
        for(int i = 0; i < 4; i++) {
            synth.noteOn(i, padChords[currentChord][i], 60);
        }
    }
}

// ==============================================================================
// SETUP PRINCIPAL
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    for (int i = 0; i < 4; i++) {
        synth.setWave(i, (i % 2 == 0) ? WAVE_PULSE : WAVE_TRIANGLE);
        // Attaque lente et Release looongue créent la nappe (pad) atmosphérique
        synth.setEnv(i, 2000, 1000, 255, 3000);
    }

    synth.setCustomDSP(chorusDSP);
    synth.setCustomControl(dreamControl);

    // Ajuste tes broches ici ! (BCLK, WS, DATA)
    if (synth.begin(4, 15, 2, I2S_32BIT)) {
        Serial.println("Dream Pads (chorus corrigé) opérationnels.");
    }
}

void loop() {
    delay(1000);
}
```

### Explications
- Le nom **"BBD"** (Bucket-Brigade Device) fait référence aux circuits analogiques historiques (utilisés dans les pédales de chorus/flanger vintage) qui font transiter le signal de "seau en seau" (condensateur en condensateur) — d'où l'analogie avec la ligne à retard numérique `chorusTape` ici.
- Le **LFO triangulaire** (`lfoPhase`/`lfoTri`) module en continu le point de lecture (`delayOffset`) dans le buffer circulaire, entre ~1 ms et ~4 ms de retard — c'est ce léger désaccord temporel variable qui crée l'effet de "battement" caractéristique du chorus (comme si deux voix légèrement désaccordées jouaient ensemble).
- `dreamControl()` illustre bien la complémentarité des deux hooks : `setCustomDSP` transforme le signal audio, tandis que `setCustomControl` gère la logique musicale de plus haut niveau (changement d'accords, modulation lente du PWM) à un rythme beaucoup plus faible (le taux de contrôle, pas le taux d'échantillonnage).
- Utiliser à la fois `WAVE_PULSE` et `WAVE_TRIANGLE` en alternance sur les 4 voix de l'accord ajoute de la richesse timbrale à la nappe.
