#pragma once

#include "InputEvent.h"
class Input;

using FnMask = uint8_t;

enum class ModalState
{
    None,
    Confirm
};

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

constexpr FnMask FN0 = 1u << 0;
constexpr FnMask FN1 = 1u << 1;
constexpr FnMask FN2 = 1u << 2;
constexpr FnMask FN3 = 1u << 3;
constexpr FnMask FN4 = 1u << 4;
constexpr FnMask FN5 = 1u << 5;
constexpr FnMask FN6 = 1u << 6;
constexpr FnMask FN7 = 1u << 7;
