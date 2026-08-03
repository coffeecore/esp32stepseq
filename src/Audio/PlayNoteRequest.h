#pragma once

#include <stdint.h>

constexpr uint8_t MAX_COMMANDS = 4;

enum class CommandType : uint8_t
{
    None,

    Arpeggio,
    Portamento,
    Vibrato
};

struct Command
{
    CommandType type = CommandType::None;

    // Interprétation de data[] selon le type (à adapter à ton usage réel) :
    // - Arpeggio    : data[0..2] = offsets en demi-tons ajoutés à `note` (ex: 0,4,7)
    //                 data[3]    = durée d'un pas en ms
    // - Portamento  : data[0]    = durée du glissando en ms (utilisé avec slideFreqTo)
    // - Vibrato     : data[0]    = taux LFO (Hz), data[1] = profondeur (Hz)
    uint8_t data[4] = {0, 0, 0, 0};
};

// Objet neutre qui sort du séquenceur : le moteur audio ne connaît pas Step/Track.
struct PlayNoteRequest
{
    uint8_t note = 60;      // note MIDI (0-127), PAS une fréquence
    uint8_t velocity = 255; // 0-255
    uint8_t instrument = 0; // index dans la banque d'instruments (0..NUMBER_OF_INSTRUMENTS-1)
    uint16_t gate = 0;      // durée en ticks (le séquenceur gère le stop, l'audio l'ignore)

    const Command* commands = nullptr;
    uint8_t commandCount = 0;
};
