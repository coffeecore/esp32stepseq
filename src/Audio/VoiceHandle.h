#pragma once

#include <stdint.h>

struct VoiceHandle
{
    uint8_t id = 255; // 255 = aucun handle / invalide

    bool valid() const
    {
        return id != 255;
    }
};
