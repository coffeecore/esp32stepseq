#pragma once

#include <Arduino.h>

enum class InputEventType
{
    ButtonPressed,
    ButtonReleased,
    ButtonHold,

    EncoderTurned
};

enum class ControlId: int8_t
{
    Step0, Step1, Step2, Step3,
    Step4, Step5, Step6, Step7,
    Step8, Step9, Step10, Step11,
    Step12, Step13, Step14, Step15,


    Fn0,
    Fn1,
    Fn2,
    Fn3,
    Fn4,
    Fn5,
    Fn6,
    Fn7,

    Encoder0,
    Encoder1,

    None,

    ControlCount
};

typedef struct
{
    InputEventType type;

    uint8_t id;

    int16_t value;

    ControlId control;
} InputEvent;
