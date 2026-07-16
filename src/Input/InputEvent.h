#pragma once

#include <Arduino.h>

#include "Input/ControlId.h"

enum class InputEventType
{
    ButtonPressed,
    ButtonReleased,
    ButtonHold,

    EncoderTurned
};

struct InputEvent
{
    InputEventType type;

    uint8_t id;

    int32_t value;

    ControlId control;

    int16_t delta;
};
