#pragma once

// #include "Display.h"
class Display;

enum class DisplayModes
{
    None,
    Main,
    Step
};

class DisplayMode
{
    public:
        explicit DisplayMode(Display& _display)
            : display(_display)
        {
        }

        virtual void handleEvent() = 0;

     protected:
        Display& display;
};
