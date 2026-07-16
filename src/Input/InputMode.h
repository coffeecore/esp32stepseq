#pragma once

#include "InputEvent.h"
class Input;

enum class ModalState
{
    None,
    Confirm
};

enum class ConfirmAction
{
    None,
    DeleteQuarterNote,
    ClearScreen
};
