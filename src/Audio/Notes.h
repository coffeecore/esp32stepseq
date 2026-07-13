#pragma once

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "ESP32Synth.h"

// Déclarations
inline char notesStr[128][6];
inline uint32_t notesFreq[128];


inline void midiToName(uint8_t midi, char* buffer, size_t bufferSize)
{
    static const char* names[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const char* note = names[midi % 12];
    int8_t octave = (midi / 12) - 1;

    if (note[1] == '\0') {
        snprintf(buffer, bufferSize, "%s-%d", note, octave);
    } else {
        snprintf(buffer, bufferSize, "%s%d", note, octave);
    }
}

// inline uint32_t midiToFreq(uint8_t note) {
//     return (uint32_t)(44000.0 * pow(2.0, (note - 69) / 12.0));
// }
inline void initNotes()
{
    static bool initialized = false;
    if (initialized)
        return;

    initialized = true;

    for (uint8_t i = 0; i < 128; i++)
    {
        midiToName(i, notesStr[i], sizeof(notesStr[i]));
        notesFreq[i] = midiToFreq(i);
    }
}
