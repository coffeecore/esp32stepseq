## Controls

|| Col 0 | Col 1 | Col 2 | Col 3 |
| --- | --- | --- | --- | --- |
| **Row 0** | Fn0 | Fn1 | Fn2 / RE0 | Fn3 / RE1 |
| **Row 1** | Fn4 | Fn5 | Fn6 | Fn7 |
| **Row 2** | S01 | S01 | S02 | S03 |
| **Row 3** | S10 | S11 | S12 | S13 |

### Main screen

#### Global

| Function | Keys | Layer | Comment | 
| -------- | ---- | ------- | ----------- |
| Play/Pause | Fn0 | GlobalLayer |
| Stop | Hold Fn0 | GlobalLayer |
| Volume | RE0 | GlobalLayer |
| BPM | RE1 | GlobalLayer |
| Navigate step | Fn1 + RE1 | NavigationLayer | |
| Navigate track | Fn1 + RE0 | NavigationLayer | |
| Navigate quarter note | Fn1 + Fn2 + RE1 | | |
| Select bank | Fn1 + Fn2 + RE1 | |
| Global instrument | Fn3 + RE1 | GlobalInstrumentLayer |

#### Track

| Function | Keys | Layer | Comment | 
| -------- | ---- | ------- | ----------- |
| Track volume | Fn2 + RE0 | TrackLayer |
| Track transpose | Fn2 + RE1 | TrackLayer |
| Track instrument | Fn3 + RE0 | GlobalInstrumentLayer |
| Mute track | Hold Fn2 | GlobalLayer |

#### Quater note (pattern)

| Function | Keys | Layer | Comment | 
| -------- | ---- | ------- | ----------- |
| Step state | Syx | StepEditLayer |
| Step note | Syx + RE0 | StepEditLayer | |
| Step octave | Syx + RE1 | StepEditLayer | |
| Step length | Syx + Fn2 + RE0 | StepLengthLayer | |
| Step instrument | Syx + Fn3 + RE1 | StepInstrumentLayer | |
| Quarter note length | Syx + Fn1 | QuarterNoteLengthLayer |
| Add quarter note | Fn1 | GlobalLayer |
| Delete last quarter note | Hold Fn1 | GlobalLayer + ModalLayer |
| Select step | Hold Syx | |

## Wiring

| Fonction              |                     GPIO |
| --------------------- | -----------------------: |
| **OLED SDA**          |                       21 |
| **OLED SCL**          |                       22 |
| **SD SCK**            |                       18 |
| **SD MISO**           |                       19 |
| **SD MOSI**           |                       23 |
| **SD CS**             |                        5 |
| **PCM5102 BCLK**      |                       26 |
| **PCM5102 LRCK (WS)** |                       25 |
| **PCM5102 DIN**       |                       27 |
| **Encodeur 1 A**      |                       32 |
| **Encodeur 1 B**      |                       33 |
| **Encodeur 2 A**      | 34 *(entrée uniquement)* |
| **Encodeur 2 B**      | 35 *(entrée uniquement)* |
| **Matrice Ligne 0**   |                        4 |
| **Matrice Ligne 1**   |                       13 |
| **Matrice Ligne 2**   |                       14 |
| **Matrice Ligne 3**   |                       16 |
| **Matrice Colonne 0** |                       17 |
| **Matrice Colonne 1** |                        0 |
| **Matrice Colonne 2** |                        2 |
| **Matrice Colonne 3** |                       15 |

## Todo

- Remove buttons wokwi
