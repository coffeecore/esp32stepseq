#pragma once

#include "Input/InputEvent.h"
#include "Input/InputMode.h"

typedef struct {
    bool usedAsModifier = false;
    bool holdTriggered = false;
    bool consumed = false;
} FnState;

enum class ConfirmAction
{
    None,
    DeleteQuarterNote,
    ClearScreen
};

struct InputContext
{
    FnMask fnMask = 0;

    bool stepHeld = false;
    ControlId stepId = ControlId::None;

    ModalState modal = ModalState::None;

    FnState fnState[8];

    bool stepUsedAsModifier = false;

    ConfirmAction confirmAction = ConfirmAction::None;
};
