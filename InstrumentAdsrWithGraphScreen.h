#pragma once

#include "Display/Workspace.h"
#include "Sequencer/Sequencer.h"
#include "Display/Screen/Screen.h"
#include "Audio/Notes.h"
#include "Audio/IAudioEngine.h"

class InstrumentAdsrScreen : public MenuScreen
{
public:
    using MenuScreen::MenuScreen;

    void draw() override
    {
        char title[32];

        snprintf(title, sizeof(title), "Instrument %u", uiState.selectedInstrument + 1);

        // Title
        u8g2.drawStr(0, 8, title);

        u8g2.drawHLine(0, 10, 128);

        drawEnvelope();
    }

private:
    // Dessine l'enveloppe ADSR sous forme de 4 segments de droite.
    // Repond exactement a la meme logique que ESP32SynthAudioEngine::
    // un stage desactive (bit de adsr.state a 0) est instantane (duree 0),
    // et un sustain desactive vaut plein volume (255).
    void drawEnvelope()
    {
        const Adsr& adsr = audioEngine.instruments[uiState.selectedInstrument].adsr;

        const uint16_t attackMs = (adsr.state & ATTACK) ? adsr.attackMs : 0;
        const uint16_t decayMs = (adsr.state & DECAY) ? adsr.decayMs : 0;
        const uint8_t sustainLvl = (adsr.state & SUSTAIN) ? adsr.sustainLvl : 255;
        const uint16_t releaseMs = (adsr.state & RELEASE) ? adsr.releaseMs : 0;

        // Zone de dessin (sous le titre / la ligne horizontale a y=10).
        constexpr uint8_t HEIGHT_SCREEN = 32;
        constexpr uint8_t X0 = 4;
        constexpr uint8_t X1 = 124;
        constexpr uint8_t TOP_Y = 14;
        constexpr uint8_t BOTTOM_Y = TOP_Y + HEIGHT_SCREEN; // 14 + 32
        constexpr uint8_t WIDTH = X1 - X0;
        constexpr uint8_t HEIGHT = BOTTOM_Y - TOP_Y;

        // Le sustain n'a pas de duree propre (tenu tant que la note est
        // maintenue): on lui reserve une largeur fixe a l'ecran, le reste
        // se repartit entre attack/decay/release au prorata de leur duree.
        constexpr uint8_t SUSTAIN_WIDTH = WIDTH / 4;
        constexpr uint8_t TIME_WIDTH = WIDTH - SUSTAIN_WIDTH;
        constexpr uint8_t MIN_SEGMENT_WIDTH = 2;

        const uint32_t totalMs =
            static_cast<uint32_t>(attackMs) + static_cast<uint32_t>(decayMs) + static_cast<uint32_t>(releaseMs);

        uint8_t attackWidth;
        uint8_t decayWidth;
        uint8_t releaseWidth;

        if (totalMs == 0) {
            // Les 3 stages sont instantanes: on garde une largeur
            // minimale visible pour chacun plutot que des points confondus.
            attackWidth = MIN_SEGMENT_WIDTH;
            decayWidth = MIN_SEGMENT_WIDTH;
            releaseWidth = MIN_SEGMENT_WIDTH;
        }
        else {
            attackWidth =
                MIN_SEGMENT_WIDTH + (static_cast<uint32_t>(TIME_WIDTH - 3 * MIN_SEGMENT_WIDTH) * attackMs) / totalMs;
            decayWidth =
                MIN_SEGMENT_WIDTH + (static_cast<uint32_t>(TIME_WIDTH - 3 * MIN_SEGMENT_WIDTH) * decayMs) / totalMs;
            releaseWidth = TIME_WIDTH - attackWidth - decayWidth;
        }

        const uint8_t sustainY = BOTTOM_Y - (static_cast<uint16_t>(sustainLvl) * HEIGHT) / 255;

        const uint8_t x0 = X0;
        const uint8_t x1 = x0 + attackWidth;
        const uint8_t x2 = x1 + decayWidth;
        const uint8_t x3 = x2 + SUSTAIN_WIDTH;
        const uint8_t x4 = x3 + releaseWidth;

        u8g2.drawLine(x0, BOTTOM_Y, x1, TOP_Y);    // Attack: silence -> pic
        u8g2.drawLine(x1, TOP_Y, x2, sustainY);    // Decay: pic -> niveau sustain
        u8g2.drawLine(x2, sustainY, x3, sustainY); // Sustain: maintenu
        u8g2.drawLine(x3, sustainY, x4, BOTTOM_Y); // Release: sustain -> silence

        u8g2.drawHLine(X0, BOTTOM_Y, WIDTH);
    }
};
