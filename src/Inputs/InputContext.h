#pragma once

#include "InputEvent.h"
#include "InputMode.h"
#include "ControlId.h"

namespace Inputs
{

using FnMask = uint8_t;

constexpr FnMask FN0 = 1u << 0;
constexpr FnMask FN1 = 1u << 1;
constexpr FnMask FN2 = 1u << 2;
constexpr FnMask FN3 = 1u << 3;
constexpr FnMask FN4 = 1u << 4;
constexpr FnMask FN5 = 1u << 5;
constexpr FnMask FN6 = 1u << 6;
constexpr FnMask FN7 = 1u << 7;

struct FnState
{
    bool usedAsModifier = false;
    bool holdTriggered = false;
    bool consumed = false;
};

struct InputContext
{
    FnMask fnMask = 0;

    bool stepHeld = false;
    ControlId stepId = ControlId::None;

    // ModalState modal = ModalState::None;

    FnState fnState[8];

    bool stepUsedAsModifier = false;

    ConfirmAction confirmAction = ConfirmAction::None;
};
} // namespace Input
