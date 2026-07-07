#pragma once
#include <U8g2lib.h>

class Screen
{
public:
    UIState& uiState;
    SequencerTimer& sequencer;
    U8G2& u8g2;

    explicit Screen(UIState& u, SequencerTimer& s, U8G2& u8g2)
        : uiState(u),
          sequencer(s),
          u8g2(u8g2)
    {
    }

    virtual void draw() = 0;
};
