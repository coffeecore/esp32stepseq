# 🎵 Exemples : démonstrations musicales complètes

Couvre : `TheAbyss`, `desmos_sounds_like_a_church_organ`.

---

## 1. TheAbyss — composition ambient à 10 voix avec delay maison

**Dossier d'origine :** `examples/Musics/TheAbyss/`

### Ce que fait l'exemple
Une composition complète structurée en 4 parties (`part1_Intro`, `part2_Improv_Csm`, `part3_Modulation_Fsm`, `part4_Finalization`), utilisant jusqu'à 10 voix simultanées réparties par rôle (basse en dent de scie, sous-basse sinus, 5 voix d'accords en pulse, arpège, lead avec vibrato, contre-mélodie), le tout enrobé d'un delay/écho global fait maison.

### Code traduit (structure et moteur DSP)

```cpp
#include <ESP32Synth.h>

ESP32Synth synth;

const int CH_BASS_SAW = 0;
const int CH_BASS_SUB = 1;
const int CH_CHORDS   = 2;  // occupe les voix 2 à 6 (5 voix d'accord)
const int CH_LEAD     = 7;
const int CH_ARP      = 8;
const int CH_COUNTER  = 9;  // contre-mélodie

#define TAPE_LEN 20000
int32_t* abyssTape = nullptr;
int writeHead = 0;

// États des filtres
int32_t lpState = 0;          // Passe-bas (assombrit le son)

void IRAM_ATTR theAbyssDSP(int32_t* mixBuffer, int numSamples) {
    if (!abyssTape) return;

    for (int i = 0; i < numSamples; i++) {
        int32_t dry = mixBuffer[i];

        // 1. Lectures sûres du buffer circulaire (sans utiliser l'opérateur '%')
        // On choisit des délais premiers entre eux, inférieurs à TAPE_LEN (20000)
        int tap1 = writeHead - 4327;  if (tap1 < 0) tap1 += TAPE_LEN;
        int tap2 = writeHead - 11003; if (tap2 < 0) tap2 += TAPE_LEN;
        int tap3 = writeHead - 19013; if (tap3 < 0) tap3 += TAPE_LEN;

        // Somme les 3 têtes et divise par 4 (>> 2) pour ne pas saturer
        int32_t wet = (abyssTape[tap1] >> 2) + (abyssTape[tap2] >> 2) + (abyssTape[tap3] >> 2);

        // 2. Filtre passe-bas (assourdit progressivement les répétitions)
        lpState = ((wet * 50) + (lpState * 206)) >> 8;

        // 3. Calcule le feedback à écrire sur la "bande" (~78% de feedback)
        int32_t feedback = (dry >> 1) + ((lpState * 200) >> 8);

        // Si les calculs tentent de dépasser, on écrase le son à la limite du 16 bits.
        if (feedback > 32767) feedback = 32767;
        else if (feedback < -32768) feedback = -32768;

        // Écrit sur la "bande" et avance
        abyssTape[writeHead] = feedback;
        writeHead++;
        if (writeHead >= TAPE_LEN) writeHead = 0;

        // Mixage master
        mixBuffer[i] = (dry) + (lpState << 1);
    }
}

void setup(){
  synth.begin(4, 15, 2, I2S_32BIT); // Il suffit de placer un module PCM5102A à côté de l'ESP32 sur la plaque d'essai.
  synth.setMasterVolume(100);
  abyssTape = (int32_t*)heap_caps_calloc(TAPE_LEN, sizeof(int32_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  synth.setCustomDSP(theAbyssDSP);

  synth.setWave(CH_BASS_SAW, WAVE_SAW);
  synth.setWave(CH_BASS_SUB, WAVE_SINE);

  for(int i = 0; i < 5; i++){
    synth.setWave(CH_CHORDS + i, WAVE_PULSE);
    synth.setPulseWidth(CH_CHORDS + i, 128);
    synth.setEnv(CH_CHORDS + i, 50, 100, 150, 600);
  }

  synth.setWave(CH_ARP, WAVE_TRIANGLE);
  synth.setEnv(CH_ARP, 10, 50, 200, 100);
  synth.setWave(CH_LEAD, WAVE_PULSE);
  synth.setPulseWidth(CH_LEAD, 64);
  synth.setEnv(CH_LEAD, 20, 100, 200, 400);
  synth.setVibrato(CH_LEAD, 500, 15);

  synth.setWave(CH_COUNTER, WAVE_SINE);
  synth.setEnv(CH_COUNTER, 400, 0, 255, 800);
}

void loop(){
  part1_Intro();
  part2_Improv_Csm();
  part3_Modulation_Fsm();
  part4_Finalization();

  delay(5000);
}
```

Extrait de la partie finale (`part4_Finalization`), qui illustre bien l'écriture "à la main" d'une partition :
```cpp
void part4_Finalization() {

  chord(a2, e4, a4, cs5, e5, a5);
  synth.setArpeggio(CH_ARP, 100, a4, cs5, e5, a5, e5, cs5);
  synth.noteOn(CH_ARP, a4, 50);

  synth.noteOn(CH_COUNTER, a5, 60);

  synth.noteOn(CH_LEAD, cs5, 160); delay(1000);
  synth.slideFreqTo(CH_LEAD, e5, 2000); delay(2000);

  chordOff(); synth.noteOff(CH_COUNTER); synth.noteOff(CH_LEAD); delay(100);
  // ... (progression harmonique qui continue en montant vers le "tourbillon" final)

  // Fondu final : toutes les voix descendent en volume simultanément
  for(int i = 0; i < 10; i++) {
    synth.slideVolTo(i, 0, 4000);
  }

  delay(4500);

  chordOff();
  synth.noteOff(CH_LEAD);
  synth.noteOff(CH_COUNTER);
  synth.detachArpeggio(CH_ARP);
  synth.noteOff(CH_ARP);
  delay(2000);
}
```

### Explications
- Cet exemple montre comment **répartir les rôles musicaux sur des voix dédiées** (constantes `CH_BASS_SAW`, `CH_CHORDS`, `CH_LEAD`, `CH_ARP`, `CH_COUNTER`...), une pratique essentielle dès qu'on écrit une composition avec plus de 2-3 voix.
- Le DSP global (`theAbyssDSP`) est structurellement identique à la réverbération vue dans `SimpleWaves`/`Simple_Reverb`, mais ici volontairement calibré comme un **delay/écho** long (les trois "têtes de lecture" créent un écho rythmique plutôt qu'une réverbération dense), renforçant l'ambiance "abyssale" du morceau.
- Le fondu final (`for(int i = 0; i < 10; i++) synth.slideVolTo(i, 0, 4000);`) est une technique simple et efficace pour terminer un morceau en fondu sur toutes les voix actives d'un coup.

---

## 2. desmos_sounds_like_a_church_organ — orgue piloté par le bouton BOOT

**Dossier d'origine :** `examples/Musics/desmos_sounds_like_a_church_organ/`

### Ce que fait l'exemple
Un exemple à la fois technique et amusant : l'auteur a défini une grande wavetable 16 bits de 2048 échantillons (`wt[]`, probablement conçue à l'origine dans l'outil en ligne Desmos, d'où le nom du fichier), l'a assignée à 12 voix, et propose une "partition" jouée **manuellement pas à pas** en appuyant sur le bouton BOOT de l'ESP32 (broche GPIO 0) à chaque nouvelle note.

### Code traduit (extraits représentatifs)

```cpp
// je m'ennuyais...
// appuie sur le bouton BOOT de l'ESP32 pour avancer d'un pas dans le morceau.
#include <ESP32Synth.h>

// Wavetable : wt
// Taille : 2048 échantillons | Profondeur : 16 bits
const int16_t wt[] = {
    0, 434, 868, 1299, 1727, 2150, 2568, 2979, /* ... 2048 valeurs au total ... */
};

const int BCK_PIN = 4;
const int WS_PIN = 15;
const int DATA_PIN = 2;

void setup(){
  synth.begin(BCK_PIN, WS_PIN, DATA_PIN, I2S_32BIT);
  synth.setMasterVolume(80);
  for(int i = 0; i<12; i++){
    synth.setWave(i,WAVE_WAVETABLE);
    synth.setEnv(i,2,0,255,150);
    synth.setWavetable(i,wt, sizeof(wt) / 2, BITS_16);
  }

  pinMode(0,INPUT); // Broche 0 = bouton BOOT de l'ESP32
  Serial.begin(115200);
}

void loop(){
  tocarSequencia(); // "jouerSequence()"
}

void tocarSequencia() {

    // --- Ligne 1 : accord de 1 note ---
    synth.noteOn(0, 20765, 127); // Note (Fréq : 20765)
    while(!digitalRead(0) == 0){
      NOP(); // Attend que le bouton BOOT soit pressé (niveau bas)
    }
    delay(10);
    while(!digitalRead(0) == 1){
      NOP(); // Attend que le bouton BOOT soit relâché (niveau haut)
    }
    delay(10);

    // --- Ligne 2 : ajoute une note à -12 demi-tons (une octave en dessous) ---
    synth.noteOn(1, 10382, 127);
    while(!digitalRead(0) == 0){ NOP(); }
    delay(10);
    while(!digitalRead(0) == 1){ NOP(); }
    delay(10);

    // --- Ligne 3 : ajoute une nouvelle note à -5 demi-tons ---
    synth.noteOn(2, 15556, 127);
    while(!digitalRead(0) == 0){ NOP(); }
    delay(10);
    while(!digitalRead(0) == 1){ NOP(); }
    delay(10);

    // --- Ligne 4 : la voix 0 change de note (accord qui évolue) ---
    synth.noteOff(0); // Note précédente arrêtée
    synth.noteOn(0, 26162, 127); // Nouvelle note
    while(!digitalRead(0) == 0){ NOP(); }
    // ... (le motif se répète pendant des centaines de lignes, construisant
    //      progressivement un choral d'orgue façon Jean-Sébastien Bach,
    //      chaque appui sur BOOT faisant avancer la partition d'une note)
}
```

### Explications
- La technique consistant à générer une seule grande **wavetable 16 bits de 2048 points** (`wt[]`) pour simuler un timbre d'orgue riche en harmoniques (plutôt qu'un simple sinus) est un excellent exemple pratique de synthèse par table d'onde complexe — l'auteur a manifestement dessiné cette forme d'onde dans un outil graphique de courbes (Desmos), d'où le nom de l'exemple.
- Le "séquenceur" ici n'est pas automatique : chaque `while(!digitalRead(0) == 0) { NOP(); }` bloque le programme jusqu'à ce que le bouton BOOT soit pressé, puis attend son relâchement — une façon très simple (bien que "bloquante", ce qui n'est généralement pas recommandé dans du code de production) de rejouer un morceau **note après note, au rythme de l'utilisateur**, comme un métronome manuel.
- `NOP()` est une instruction assembleur "No Operation" — elle ne fait rien, elle sert juste à occuper le CPU pendant l'attente active (busy-wait).
- 12 voix sont préparées dès `setup()`, chacune avec sa propre enveloppe très rapide (Attaque 2ms, Decay 0, Sustain 255, Release 150ms) — un profil typique d'orgue à tuyaux, où le son atteint son plein volume quasi instantanément et se coupe net (avec un léger relâchement) dès le `noteOff()`.
