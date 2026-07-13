#pragma once

#include "Display/Workspace.h"
#include "SequencerTimer.h"
#include "Display/Screen/Screen.h"
#include "Audio/Notes.h"

class SequencerScreen : public Screen
{
public:
    using Screen::Screen;

    void draw() override
    {
        // --- HEADER ---
        // Play pause stop icon
        uint8_t x = 0;
        uint8_t y = 0;
        switch (sequencer.playState)
        {
            case PlayState::Play:
                u8g2.drawLine(x, y, x, y+6);
                u8g2.drawLine(x+1, y+1, x+1, y+5);
                u8g2.drawLine(x+2, y+2, x+2, y+4);
                u8g2.drawPixel(x+3, y+3);

                break;

            case PlayState::Pause:
                u8g2.drawBox(x,   y, 2, 7);
                u8g2.drawBox(x+4, y, 2, 7);

                break;

            case PlayState::Stop:
                u8g2.drawBox(x, y, 6, 6);

                break;
        }
        

        char buffer[Constants::SCREEN_WIDTH / u8g2.getMaxCharWidth() + 1];
        x = u8g2.getMaxCharWidth() * 2;
        y = u8g2.getAscent() + 0 * u8g2.getMaxCharHeight();
        // Sequencer Volume
        snprintf(
            buffer,
            sizeof(buffer),
            "%3d",
            sequencer.volume
        );
        u8g2.drawStr(x, y, buffer);

        // Sequencer BPM
        snprintf(
            buffer,
            sizeof(buffer),
            "%3d",
            sequencer.bpm
        );
        u8g2.drawStr(x + (u8g2.getMaxCharWidth() * 3) + u8g2.getMaxCharWidth(), y, buffer);

        // Selected instrument
        snprintf(
            buffer,
            sizeof(buffer),
            "%01X",
            uiState.selectedInstrument
        );
        u8g2.drawStr(x + (u8g2.getMaxCharWidth() * 7) + u8g2.getMaxCharWidth(), y, buffer);

        // Selected quarter notes
        snprintf(
            buffer,
            sizeof(buffer),
            "%02X/%02X",
            sequencer.quarterNoteCounts > 0 ? uiState.selectedQuarterNote + 1: uiState.selectedQuarterNote,
            sequencer.quarterNoteCounts
        );
        u8g2.drawStr(x + (u8g2.getMaxCharWidth() * 9) + u8g2.getMaxCharWidth(), y, buffer);

        // Autoscroll
        snprintf(
            buffer,
            sizeof(buffer),
            "%d",
            uiState.autoScroll
        );
        u8g2.drawStr(x + (u8g2.getMaxCharWidth() * 16) + u8g2.getMaxCharWidth(), y, buffer);

        // --- TRACKS QUARTER NOTES ---
        if (sequencer.trackCounts > (uiState.displayedTrack + Constants::NUMBER_OF_DISPLAYED_TRACKS)) {
            u8g2.drawVLine(125, 30, 5);
            u8g2.drawVLine(126, 31, 3);
            u8g2.drawPixel(127, 32);
        }

        if (uiState.displayedTrack > 0) {
            u8g2.drawVLine(123, 30, 5);
            u8g2.drawVLine(122, 31, 3);
            u8g2.drawPixel(121, 32);
        }

        if (sequencer.quarterNoteCounts > (uiState.selectedQuarterNote + 1)) {
            u8g2.drawHLine(122, 61, 5);
            u8g2.drawHLine(123, 62, 3);
            u8g2.drawPixel(124, 63);
        }

        if (uiState.selectedQuarterNote > 0) {
            u8g2.drawHLine(122, 20, 5);
            u8g2.drawHLine(123, 19, 3);
            u8g2.drawPixel(124, 18);
        }

        for (uint8_t i = uiState.displayedTrack;i< uiState.displayedTrack + Constants::NUMBER_OF_DISPLAYED_TRACKS;i++) {
            // Tracks
            char buffer[Constants::SCREEN_WIDTH / Constants::NUMBER_OF_DISPLAYED_TRACKS / u8g2.getMaxCharWidth() + 1 + 7];
            Track& track = sequencer.tracks[i];

            if (i == uiState.selectedTrack) {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%01X %3d %3d A",
                    // i,
                    track.instrument,
                    track.volume,
                    track.transpose
                );

                u8g2.drawStr((i - uiState.displayedTrack)*60, u8g2.getAscent() + 1 * u8g2.getMaxCharHeight(), buffer);
            } else {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "Tr%1d %01X",
                    i,
                    track.instrument
                );
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "Tr%1d",
                    i
                );

                u8g2.drawStr((i - uiState.displayedTrack)*60, u8g2.getAscent() + 1 * u8g2.getMaxCharHeight(), buffer);
            }

            if (sequencer.quarterNoteCounts <= 0) {
                continue;
            }

            // Quarter notes
            QuarterNote& qn = sequencer.tracks[i].quarterNotes[uiState.selectedQuarterNote];

            for (uint8_t j = 0;j<qn.stepsCount;j++) {
                Step& step = qn.steps[j];
                uint8_t sc = qn.stepsCount;
                
                if (sc == 0 ) {
                    continue;
                }
            
                uint8_t height = 48 / sc;
                if (qn.ticksByStep == 0 ) {
                    continue;
                }
                uint8_t len = (height-2) * step.length / qn.ticksByStep;                

                if (step.state) {
                    u8g2.drawVLine((i - uiState.displayedTrack) *60+1, 16+(j * height)+2, len-1);

                    u8g2.drawFrame((i - uiState.displayedTrack) *60, 16+(j * height)+1, 3, height-1);

                    char buffer[11];

                    uint8_t mask = 0;

                    char fx[4];

                    fx[0] = (mask & 0x01) ? 'A' : '-';
                    fx[1] = (mask & 0x02) ? 'B' : '-';
                    fx[2] = (mask & 0x04) ? 'C' : '-';
                    fx[3] = '\0';

                    char inst = (step.instrument < 0)
                        ? '-'
                        : "0123456789ABCDEF"[step.instrument];

                    snprintf(
                        buffer,
                        sizeof(buffer),
                        "%3s %c %s",
                        notesStr[step.note],
                        inst,
                        fx
                    );

                    if (uiState.selectedStep == j && uiState.selectedTrack == i) {
                        u8g2.drawButtonUTF8(((i - uiState.displayedTrack) *60)+2+5, 24+(j * height), U8G2_BTN_INV, 0, 1, 1, buffer);
                    } else {
                        u8g2.drawStr(((i - uiState.displayedTrack) *60)+2+5, 24+(j * height), buffer);
                    }
                } else {
                    if (uiState.selectedStep == j && uiState.selectedTrack == i) {
                        u8g2.drawButtonUTF8(((i - uiState.displayedTrack) *60)+2+5, 24+(j * height), U8G2_BTN_INV, 0, 1, 1, "--- - ---");
                    } else {
                        u8g2.drawStr(((i - uiState.displayedTrack) *60)+2+5, 24+(j * height), "--- - ---");
                    }
                }
            }
        }
    }
};
