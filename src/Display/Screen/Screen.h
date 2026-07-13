#pragma once
#include <U8g2lib.h>

class Screen
{
public:
    UIState& uiState;
    Sequencer& sequencer;
    U8G2& u8g2;

    explicit Screen(UIState& u, Sequencer& s, U8G2& u8g2)
        : uiState(u),
          sequencer(s),
          u8g2(u8g2)
    {
    }

    virtual void draw() = 0;
};
