#pragma once

#include <Arduino.h>

#include "Input/ControlId.h"
#include "Input/InputContext.h"

constexpr bool isFn(ControlId c)
{
    return c >= ControlId::Fn0 &&
           c <= ControlId::Fn7;
}

constexpr bool isStep(ControlId c)
{
    return c >= ControlId::Step0 &&
           c <= ControlId::Step11;
}

constexpr bool isEncoder(ControlId c)
{
    return c == ControlId::Encoder0 ||
           c == ControlId::Encoder1;
}

constexpr uint8_t fnIndex(ControlId c)
{
    return static_cast<uint8_t>(c)
         - static_cast<uint8_t>(ControlId::Fn0);
}

constexpr FnMask fnBit(ControlId c)
{
    return static_cast<FnMask>(1u << fnIndex(c));
}
