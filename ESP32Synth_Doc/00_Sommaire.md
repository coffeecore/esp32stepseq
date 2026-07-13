# 📚 Documentation ESP32Synth (v2.4.3) — en français

Cette documentation a été générée à partir du code source réel de la bibliothèque (dossier `src/`) et des exemples fournis (dossier `examples/`). Elle traduit et explique le fonctionnement de la librairie, ainsi que ses exemples, en français.

> ℹ️ Le dossier `don't open me/` du projet original est volontairement ignoré : l'auteur y range des essais ratés / exemples destructeurs pour les oreilles. Il n'y a rien d'utile à documenter là-dedans.

## Sommaire

1. **[01_Guide_API.md](01_Guide_API.md)** — Référence complète de l'API : initialisation, contrôle des voix, enveloppes, modulations, wavetables, échantillons, streaming SD, hooks DSP personnalisés, getters, etc.
2. **[02_Demarrage_Rapide.md](02_Demarrage_Rapide.md)** — Guide de démarrage rapide, installation, premier programme, philosophie de la bibliothèque (pourquoi 500 voix ?), configuration mémoire.
3. **Exemples traduits et expliqués** (dossier `exemples/`) — Chaque fichier regroupe plusieurs exemples originaux du même thème, avec le code traduit (commentaires en français) et une explication pédagogique de ce qu'il se passe.

   | Fichier | Exemples couverts |
   |---|---|
   | `exemples/01_Formes_Ondes_Bases.md` | BasicWaveforms, SimpleWaves, JunoAndDX7, Hammond B3/Leslie122, Virtual Guitar |
   | `exemples/02_Wavetables_Instruments.md` | WavetableSynth, InstrumentSequence |
   | `exemples/03_Echantillons_Streaming.md` | Amen_Break_Loop, Low_BitDepth, ESP32Synth_WavPlayer, CYD_LVGL_WAV_PLAYER |
   | `exemples/04_DSP_Personnalise.md` | Simple_Reverb, Acid_TB-303_SVF_Filter, BBD_Chorus |
   | `exemples/05_Sorties_Personnalisees.md` | MusicTest (Bluetooth A2DP), SimpleA2DP |
   | `exemples/06_Effets_Tracker.md` | EffectsDemo (vibrato/tremolo/slides), Arpeggiator |
   | `exemples/07_Musiques_Demos.md` | TheAbyss, desmos_sounds_like_a_church_organ |
   | `exemples/08_Trucs_Astuces.md` | RT_FM_on_Wavetables, SVF_filters_on_wavetable |
   | `exemples/09_Synths_Physiques.md` | ESP32Synth_Chill_Synth, CYD_Expressive_Ribbon_Synth |
   | `exemples/10_Fun.md` | SpaceShooter, PolifonyTest, ESP32SynthMidi (BETA), Poly12 (communauté) |
   | `exemples/11_Legacy.md` | Square_pwm, JSBach Toccata et Fugue, Aditive_square, Super_Saw |

## À propos de la bibliothèque

**ESP32Synth** est un moteur de synthèse audio polyphonique pour ESP32 / ESP32-S3, écrit en C++ bare-metal, optimisé en arithmétique à virgule fixe (pas de `float` dans le chemin audio critique). Elle permet de générer jusqu'à ~80 voix simultanées par défaut (jusqu'à 300-500 en poussant les réglages), avec :

- oscillateurs de base (sinus, triangle, dent de scie, pulse/carré, bruit) ;
- wavetables personnalisées ;
- lecture d'échantillons (samples) avec boucles ;
- streaming WAV depuis une carte SD (Arduino) ou le système de fichiers ESP-IDF ;
- enveloppes ADSR, vibrato, trémolo, glissandos (slides), arpégiateur ;
- « instruments » façon tracker (séquences de volume/onde) ;
- hooks DSP personnalisés (réverbération, chorus, filtres, etc.) injectables sur le bus audio global ;
- formes d'onde 100 % personnalisées par voix (callback C++ appelé à chaque échantillon) ;
- sorties I2S, PDM, DAC interne, PWM (LEDC) ou mode « custom » (Bluetooth A2DP, WiFi, etc.).

Bonne lecture, et amuse-toi bien avec ton ESP32 ! 🎹
