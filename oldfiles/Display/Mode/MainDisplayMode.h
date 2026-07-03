#pragma once

#include "Display/Display.h"
#include "Display/DisplayMode.h"
#include "SequencerTimer.h"

class MainDisplayMode : public DisplayMode
{
    private:
        SequencerTimer& sequencerTimer;
    public:
        MainDisplayMode(
            Display& _display,
            SequencerTimer& _sequencerTimer
        )
            : DisplayMode(_display),
            sequencerTimer(_sequencerTimer)
        {
        }

        void handleEvent() override
        {
            U8G2& u8g2 = display.u8g2;

            if (sequencerTimer.trackCounts > (display.displayedTrack + 2)) {
                u8g2.drawVLine(125, 30, 5);
                u8g2.drawVLine(126, 31, 3);
                u8g2.drawPixel(127, 32);
            }

            if (display.displayedTrack > 0) {
                u8g2.drawVLine(123, 30, 5);
                u8g2.drawVLine(122, 31, 3);
                u8g2.drawPixel(121, 32);
            }

            if (sequencerTimer.quarterNoteCounts > (sequencerTimer.selectedQuarterNote + 1)) {
                u8g2.drawHLine(122, 61, 5);
                u8g2.drawHLine(123, 62, 3);
                u8g2.drawPixel(124, 63);
            }

            if (sequencerTimer.selectedQuarterNote > 0) {
                u8g2.drawHLine(122, 20, 5);
                u8g2.drawHLine(123, 19, 3);
                u8g2.drawPixel(124, 18);
            }

            uint8_t x = 0;
            uint8_t y = 0;

            if (sequencerTimer.playState == PlayState::Play) {
                u8g2.drawLine(x, y, x, y+6);
                u8g2.drawLine(x+1, y+1, x+1, y+5);
                u8g2.drawLine(x+2, y+2, x+2, y+4);
                u8g2.drawPixel(x+3, y+3);
            }

            if (sequencerTimer.playState == PlayState::Pause) {
                u8g2.drawBox(x,   y, 2, 7);
                u8g2.drawBox(x+4, y, 2, 7);
            }

            if (sequencerTimer.playState == PlayState::Stop) {
                u8g2.drawBox(x, y, 6, 6);
            }

            char buffer[30];

            snprintf(
                buffer,
                sizeof(buffer),
                "%3d %3d %02X/%02X %01X",
                sequencerTimer.volume,
                sequencerTimer.bpm,
                sequencerTimer.quarterNoteCounts > 0 ? sequencerTimer.selectedQuarterNote + 1: sequencerTimer.selectedQuarterNote,
                sequencerTimer.quarterNoteCounts,
                sequencerTimer.selectedInstrument
            );

            u8g2.drawStr(10, u8g2.getAscent() + 0 * u8g2.getMaxCharHeight(), buffer);

            // 2 is max track to display
            for (uint8_t i = display.displayedTrack;i< min<uint8_t>(
                display.displayedTrack + 2,
                sequencerTimer.trackCounts
            );i++) {
                char buffer[21];
                    Track& track = sequencerTimer.tracks[i];

                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%sTr%1d %01X",
                    sequencerTimer.selectedTrack == i ? ">":" ",
                    i,
                    track.instrument
                );

                u8g2.drawStr((i - display.displayedTrack)*60, u8g2.getAscent() + 1 * u8g2.getMaxCharHeight(), buffer);

                if (sequencerTimer.quarterNoteCounts <= 0) {
                    continue;
                }

                QuarterNote& qn = sequencerTimer.tracks[i].quarterNotes[sequencerTimer.selectedQuarterNote];

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

                        // u8g2.drawLine(i*60+3, 16+(j * height)+2, i*60+1, (16+(j * height)+len));
                        u8g2.drawVLine((i - display.displayedTrack) *60+1, 16+(j * height)+2, len-1);

                        u8g2.drawFrame((i - display.displayedTrack) *60, 16+(j * height)+1, 3, height-1);

                        char buffer[11];
                        /**
                         * @todo get from step
                         */
                        
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
                            "%4s %c %s",
                            step.noteStr,
                            inst,
                            fx
                        );

                        u8g2.drawStr(((i - display.displayedTrack) *60)+2, 24+(j * height), buffer);
                    } else {
                        u8g2.drawStr(((i - display.displayedTrack) *60)+2, 24+(j * height), "---- - ---");
                    }
                }
            }
        }
};
