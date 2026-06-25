# Todo

- [x] Ticks system : based on hardware timer (`hw_timer_t` or `esp_timer`)
- [ ] Sequencer
    - [ ] Set BPM
    - [ ] Tracks > Patterns > Steps
    - [ ] Step on and step off based on ticks
    - [ ] Step on length based on number of ticks
    - [ ] Catch up if drift
    - [ ] Tracks
        - [ ] Add a track
        - [ ] Delete a track
        - [ ] Volume
        - [ ] Transpose
        - [ ] Sample
    - [ ] Patterns
        - [ ] Add pattern
        - [ ] Delete pattern
        - [ ] Set number of quarter notes
        - [ ] Set number of steps by quarter notes
    - [ ] Step
        - [ ] Set state
        - [ ] Set note
        - [ ] Set length
- [ ] Display
    - [ ] Boot
    - [ ] Sequencer informations
        - [ ] Display volume
        - [ ] Display BPM
        - [ ] Display current track
        - [ ] Display current pattern
    - [ ] Track
        - [ ] Display volume
        - [ ] Display transpose
        - [ ] Display sample
    - [ ] Step
        - [ ] Display state
        - [ ] Display note
        - [ ] Display length
- [ ] Input
    - [ ] Sequencer
        - [ ] Set volume
        - [ ] Set BPM
        - [ ] Select current track
        - [ ] Select current pattern
    - [ ] Track
        - [ ] Set volume
        - [ ] Set transpose

# Pins

| Fonction | GPIO |
| --- | --- |
| I2C SDA | 21 |
| I2C SCL | 22 |
| | |
| ENC1 CLK | 34 |
| ENC1 DT | 35 |
| ENC1 SW | 32 |
| | |
| ENC2 CLK | 16 |
| ENC2 DT | 17 |
| ENC2 SW | 33 |
| | |
| SD CS | 5 |
| SD MOSI | 23 |
| SD MISO | 19 |
| SD SCK | 18 |
| | |
| I2S BCK | 26 |
| I2S LCK | 25 |
| I2S DIN | 27 |
| | |
| BTN1 | GPA0 |
| BTN2 | GPA1 |
| BTN3 | GPA2 |
| BTN4 | GPA3 |
| BTN5 | GPA4 |
| BTN6 | GPA5 |

⚠️ Détails importants (souvent oubliés)
 - MCP23017
activer pull-up via code (pas physique)
possibilité d’utiliser INT → GPIO 4 (optionnel)
 - KY-040
prévoir debounce logiciel
 - PCM5102
alimentation propre (important pour audio)
GND commun obligatoire



```
docker run --rm -u 1000:1000 -v ${PWD}:/src wokwi/builder-clang-wasm:latest make
```


https://wokwi.com/projects/461268764932519937



Bouton	GPIO
BTN0	13
BTN1	12
BTN2	14
BTN3	27
BTN4	26
BTN5	25
BTN6	33
BTN7	32

Avec INPUT_PULLUP.



~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line -pfiaC -e .pio/build/esp32dev/firmware.elf 0x400d09b8
