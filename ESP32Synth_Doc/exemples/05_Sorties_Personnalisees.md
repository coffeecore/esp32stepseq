# 📡 Exemples : sorties personnalisées (Bluetooth A2DP)

Couvre : `SimpleA2DP`, `MusicTest`.

Ces deux exemples utilisent le **mode "pull" `SMODE_CUSTOM`** (§14 du guide API, `beginCustom()`) pour envoyer l'audio généré par ESP32Synth vers une enceinte Bluetooth, via la bibliothèque externe `ESP32-A2DP` (`BluetoothA2DPSource`). Ce ne sont pas des fonctionnalités natives d'ESP32Synth, mais des démonstrations d'intégration.

> ⚠️ **Avertissement mémoire important** (présent dans les deux exemples) : à partir du Core Arduino ESP32 version **3.2.0**, Espressif a ajouté d'importantes couches de pilotes internes qui augmentent fortement l'usage statique de la DRAM, ne laissant presque plus de tas libre pour le Bluetooth classique (Bluedroid). **ESP32Synth n'est pas en cause** — la bibliothèque est compatible avec toutes les versions de Core de 3.0.0 à la plus récente. Pour ces deux exemples Bluetooth spécifiquement, il faut revenir à une version de Core **entre 3.0.0 et 3.1.3** dans le gestionnaire de cartes Arduino, ce qui libère plus de 30 Ko de RAM essentielle.

---

## 1. SimpleA2DP — enceinte Bluetooth interactive, optimisée basse RAM

**Dossier d'origine :** `examples/Custum Outputs/SimpleA2DP/`

### Ce que fait l'exemple
Scanne les enceintes Bluetooth A2DP à proximité, propose une interface série interactive pour choisir et s'y connecter, puis joue une progression d'accords simple (mélodie + basse) une fois connecté. Le tout est optimisé pour consommer un minimum de RAM (table d'appareils limitée à 8 entrées, pas d'allocation dynamique dans le rendu audio, parseur série non bloquant caractère par caractère).

### Partie ESP32Synth traduite (le cœur du sujet)

```cpp
// ====================================================================================
// == MODULE 2 : DSP AUDIO ET LOGIQUE MUSICALE
// ====================================================================================

int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    if (__builtin_expect(frame == nullptr || frame_count <= 0, 0)) return 0;

    synth.generateSamplesStereo((int16_t*)frame, frame_count);
    return frame_count;
}

void synth_setup() {
    synth.beginCustom(44100, nullptr);
    synth.setMasterVolume(100);

    // Voix 0 : mélodie principale (onde pulse)
    synth.setWave(0, WAVE_PULSE);
    synth.setPulseWidth(0, 128); // Onde carrée
    synth.setVolume(0, 150);
    synth.setEnv(0, 10, 80, 120, 200);

    // Voix 1 : ligne de basse (onde triangle)
    synth.setWave(1, WAVE_TRIANGLE);
    synth.setVolume(1, 130);
    synth.setEnv(1, 40, 150, 180, 250);
}

void music_loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastChangeTime >= 2000) {
        lastChangeTime = currentMillis;

        switch (chordIndex) {
            case 0:
                synth.noteOn(0, c4, 150);
                synth.noteOn(1, c2, 130);
                chordIndex = 1;
                break;
            case 1:
                synth.noteOn(0, e4, 150);
                synth.noteOn(1, a1, 130);
                chordIndex = 2;
                break;
            case 2:
                synth.noteOn(0, f4, 150);
                synth.noteOn(1, f1, 130);
                chordIndex = 3;
                break;
            case 3:
                synth.noteOn(0, g4, 150);
                synth.noteOn(1, g1, 130);
                chordIndex = 0;
                break;
        }
    }
}

// ====================================================================================
// == MODULE 3 : POINTS D'ENTRÉE PRINCIPAUX DU SYSTÈME
// ====================================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("[Système] Initialisation de l'A2DP interactif optimisé mémoire...");

    synth_setup();

    a2dp_source.set_data_callback_in_frames(get_data_frames);
    bt_setup(); // Initialise le module Bluetooth (scan, connexion...)
}

void loop() {
    // Traite les états du gestionnaire de tâches Bluetooth
    bt_loop();

    // Déclenche la séquence musicale si l'enceinte est connectée
    if (a2dp_source.is_connected()) {
        music_loop();
    } else {
        delay(5); // Pause non bloquante, alimente en douceur le watchdog du Core 1
    }
}
```

### Explications
- `synth.beginCustom(44100, nullptr)` initialise le moteur **sans timer DMA automatique** : c'est la bibliothèque `BluetoothA2DPSource` qui va, de son côté, demander périodiquement des échantillons via son callback `set_data_callback_in_frames()`.
- `get_data_frames()` est exactement le callback attendu par `ESP32-A2DP` : à chaque appel, on lui fournit `frame_count` trames stéréo, produites en interne par `synth.generateSamplesStereo()` — c'est le pont direct entre le moteur ESP32Synth et la pile Bluetooth.
- Le reste du fichier (modules "Bluetooth Manager", scan, connexion, parseur série) est de la gestion Bluetooth générique, indépendante d'ESP32Synth — non détaillée ici.

---

## 2. MusicTest — variante complète avec découverte d'appareils

**Dossier d'origine :** `examples/Custum Outputs/MusicTest/`

### Ce que fait l'exemple
Version plus complète de la démonstration A2DP, avec une table de 15 appareils Bluetooth découverts (au lieu de 8), pensée pour explorer davantage les capacités de scan/connexion de la pile Bluetooth, tout en réutilisant exactement le même principe d'intégration audio qu'au-dessus (`beginCustom` + `generateSamplesStereo` dans le callback A2DP).

### Remarque de compatibilité (en commentaire dans le fichier)
```cpp
// POUR QUE ÇA FONCTIONNE, MERCI D'UTILISER LE CORE ESP32 (Arduino IDE) EN VERSION 3.0.0 À 3.1.3
```

### Explications
- L'intégration audio ESP32Synth ↔ Bluetooth est identique à `SimpleA2DP` : c'est la partie "découverte et gestion des appareils" (`MAX_DISCOVERED_DEVICES = 15`, structures `DiscoveredDevice`) qui diffère, avec une empreinte mémoire plus généreuse.
- Ces deux exemples démontrent que **le mode `SMODE_CUSTOM` d'ESP32Synth est totalement découplé du support matériel réel** : le moteur ne sait pas (et n'a pas besoin de savoir) que les échantillons finissent sur une enceinte Bluetooth plutôt que sur un DAC I2S — c'est le même principe qui permettrait de brancher ESP32Synth sur du streaming WiFi, un WebSocket, ou tout autre protocole de sortie audio "pull".
