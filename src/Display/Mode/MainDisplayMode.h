#pragma once

#include "Display/Display.h"
#include "Display/DisplayMode.h"
#include "SequencerTimer.h"

class MainDisplayMode : public DisplayMode
{
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

        display.clear();

//         int x = 90;
// int y = 5;

// // Play
// u8g2.drawLine(x,   y,   x,   y+6);
// u8g2.drawLine(x+1, y+1, x+1, y+5);
// u8g2.drawLine(x+2, y+2, x+2, y+4);
// u8g2.drawPixel(x+3, y+3);

// // Pause
// u8g2.drawBox(x+10,   y, 2, 7);
// u8g2.drawBox(x+4+10, y, 2, 7);

// // Stop
// u8g2.drawBox(x+20, y, 7, 7);

// // u8g2.drawFrame(x+30, y, 7, 7);  // optionnel
// u8g2.drawBox(x+1+30, y+1, 5, 5);

        // display.drawMode("Main mode");
        // Serial.println("DEBUG");
        // Serial.println(display.displayedTrack + 2);
        // Serial.println(sequencerTimer.trackCounts);
        if (sequencerTimer.trackCounts > (display.displayedTrack + 2)) {
            // u8g2.drawTriangle(125, 30, 125, 36, 128, 33);
            u8g2.drawVLine(125, 30, 5);
            u8g2.drawVLine(126, 31, 3);
            u8g2.drawPixel(127, 32);
        }

        if (display.displayedTrack > 0) {
            // u8g2.drawTriangle(124, 30, 124, 36, 121, 33);
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
// 44,20      84,20
//    *------*
//     \    /
//      \  /
//       \/
//     64,50
// 64, 50, 44, 20, 84, 20
//         if (sequencerTimer.selectedQuarterNote == 0) {
//             // u8g2.drawTriangle(122, 64, 120, 60, 123, 60);
//             u8g2.drawTriangle(
//     62, 30,  // haut gauche
//     66, 30,  // haut droite (largeur = 5 px)
//     64, 32   // pointe en bas (hauteur = 3 px)
// );

// u8g2.drawPixel(64, 32);
// u8g2.drawHLine(63, 31, 3);
// u8g2.drawHLine(62, 30, 5);
//         }

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
            "%3d %3d %02X %01X",
            sequencerTimer.volume,
            sequencerTimer.bpm,
            sequencerTimer.selectedQuarterNote,
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

            QuarterNote& qn = sequencerTimer.tracks[i].quarterNotes[sequencerTimer.selectedQuarterNote];
            // Serial.println("STEPCOUN");
            // Serial.println(qn.stepsCount);
            for (uint8_t j = 0;j<qn.stepsCount;j++) {
                Step& step = qn.steps[j];
                uint8_t sc = qn.stepsCount;
                
                if (sc == 0 ) {
                    continue;
                }
            
                uint8_t height = 48 / sc;
                // Serial.println("DEBUG J");
                // Serial.println(j);
                // Serial.println(qn.ticksByStep);
                if (qn.ticksByStep == 0 ) {
                    continue;
                }
                uint8_t len = (height-2) * step.length / qn.ticksByStep;

                
                // u8g2.drawFrame(i*60, 16+(j * height), 60, height);


                

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

        display.draw();
    }

    private:
        SequencerTimer& sequencerTimer;
};
