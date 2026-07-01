Pour un **ESP32 DevKitC V4 (38 broches)**, en gardant les interfaces matérielles standards (I²C, SPI, I²S) et en minimisant les risques, voici le câblage que je retiendrais.

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

## Pourquoi ce choix ?

### I²C

Les GPIO **21/22** sont les broches utilisées par défaut sur l'ESP32.

### SPI

Le bus **VSPI** matériel :

* SCK → 18
* MISO → 19
* MOSI → 23
* CS → 5

C'est le câblage le plus courant et le mieux supporté.

### I²S

Le PCM5102 fonctionne parfaitement avec :

* BCLK → 26
* LRCK → 25
* DIN → 27

Aucun conflit avec le SPI ou l'I²C.

### Encodeurs

Les GPIO **34** et **35** sont **entrée uniquement**, ce qui est parfait pour les signaux A/B d'un encodeur.

L'autre encodeur sur **32/33** est également un excellent choix.

### Matrice

J'utiliserais :

* **Lignes (sorties)** : 4, 13, 14, 16
* **Colonnes (entrées avec pull-up)** : 17, 0, 2, 15

Ainsi, les GPIO de strapping (**0**, **2** et **15**) sont **uniquement utilisés comme entrées**. Si ta matrice est câblée correctement (les colonnes au repos restent au niveau haut grâce aux résistances de tirage), ces broches conserveront un état compatible au démarrage.

## Les GPIO à ne pas utiliser

Évite complètement :

* GPIO6 à GPIO11 → mémoire flash interne.
* GPIO12 → broche de strapping sensible (configuration de la flash).
* GPIO1 et GPIO3 → liaison série USB (programmation et logs).

## Schéma d'ensemble

```
ESP32

I2C
21 -> SDA OLED
22 -> SCL OLED

SPI
18 -> SD SCK
19 -> SD MISO
23 -> SD MOSI
 5 -> SD CS

I2S
26 -> PCM5102 BCLK
25 -> PCM5102 LRCK
27 -> PCM5102 DIN

Encodeur 1
32 -> A
33 -> B

Encodeur 2
34 -> A
35 -> B

Matrice
Sorties :
 4
13
14
16

Entrées :
17
 0
 2
15
```

C'est le câblage que je choisirais pour un projet comme ton séquenceur. Il utilise les interfaces matérielles de façon classique, évite les broches réellement problématiques (flash, GPIO12, UART) et limite l'utilisation des GPIO de strapping à des **entrées** de la matrice, ce qui est la solution la plus sûre dans les contraintes du DevKitC V4.
