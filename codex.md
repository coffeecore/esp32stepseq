# Analyse rapide de `src/`

Aucune modification du code source n'a été effectuée.

## Priorités

OK 1. `setup_audio()` n'est jamais appelée : le synthé n'exécute donc pas `synth.begin(...)`. C'est probablement bloquant pour l'audio. Ajouter simplement `setup_audio();` au début de `setup()`.
   - Fichier : `src/main.cpp:50`

OK 2. Le chargement des samples a un bug de sélection : `allocateSample()` retourne presque toujours le slot 0, car le test est inversé/placé dans une double boucle. Les 12 instruments risquent donc d'écraser le même sample enregistré.
   - Fichier : `src/Audio/ESP32SynthAudioEngine.h:35`

OK 3. Changer le nombre de pas d'une noire met uniquement à jour `stepsCount`, sans recalculer `ticksByStep` ni les longueurs des notes. L'UI peut afficher 1-4 pas, mais le timing reste celui d'avant.
   - Fichier : `src/Sequencer/Sequencer.h:198`

OK 4. Plusieurs setters de step accèdent aux tableaux sans valider `trackIndex`, `quarterNoteIndex` et `stepIndex`. `toggleStep()` oublie notamment le contrôle de piste. Un petit garde-fou éviterait les accès hors limites depuis l'UI.
   - Fichier : `src/Sequencer/Sequencer.h:132`

OK 5. `mute` et `transpose` sont modifiables et affichés, mais ne sont pas appliqués au déclenchement audio : `process()` joue toutes les pistes et `triggerStepOn()` transmet `step.note` sans transposition.
   - Fichier : `src/Sequencer/Sequencer.h:405`

## Secondaire

- Les accès `notesFreq[request.note + offset]` ne sont pas bornés : un arpège ou une octave extrême peut sortir des 128 notes MIDI.
  - Fichier : `src/Audio/ESP32SynthAudioEngine.h:199`

OK - Démarrer le séquenceur avant d'ajouter pistes/noires ouvre une petite fenêtre de concurrence au démarrage. Mettre la configuration du pattern avant `sequencer.begin()` simplifierait cela.
  - Fichier : `src/main.cpp:111`

OK - `Wire.begin(21, 22)` est immédiatement suivi de `Wire.begin()`, ce qui est redondant et peut réinitialiser les pins selon la plateforme.
  - Fichier : `src/main.cpp:70`

## Vérification

Le projet est utilisé avec PIOArduino dans VS Code. La compilation n'a simplement pas pu être lancée depuis l'environnement d'analyse, où les commandes `platformio` et `pio` ne sont pas disponibles ; cela ne concerne pas l'installation locale VS Code.



codex resume 019fcd15-5c19-7bc1-ab75-45c52fec3b32
