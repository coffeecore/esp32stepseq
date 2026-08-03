#pragma once

#include <Arduino.h>

#include "ControlId.h"

namespace Inputs
{

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

    int8_t direction;
};
} // namespace Input
