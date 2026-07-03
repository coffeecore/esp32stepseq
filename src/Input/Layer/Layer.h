#pragma once

#include "Input/InputMode.h"
#include "SequencerTimer.h"
#include "Display/Display.h"
#include "Input/RotaryEncoder.h"

enum class LayerId : uint8_t
{
    Global,
    StepEdit,
    // StepLength,
    // StepInstrument,
    // Navigation,
    // Modal
};

typedef struct {
    bool usedAsModifier = false;
    bool holdTriggered = false;
} FnState;

enum class ConfirmAction
{
    None,
    DeleteQuarterNote,
    ClearScreen
};

typedef struct
{
    FnMask fnMask = 0;

    bool stepHeld = false;
    ControlId stepId = ControlId::None;

    ModalState modal = ModalState::None;

    FnState fnState[8];

    bool stepUsedAsModifier = false;

    ConfirmAction confirmAction = ConfirmAction::None;
} InputContext;


class Layer
{
    public:
        InputContext& inputContext;
        RotaryEncoder& rotaryEncoders;
        SequencerTimer& sequencerTimer;
        Display& display;

        explicit Layer(InputContext& _inputContext, RotaryEncoder& _rotaryEncoders, SequencerTimer& _sequencerTimer, Display& _display)
            : inputContext(_inputContext),
            rotaryEncoders(_rotaryEncoders),
            sequencerTimer(_sequencerTimer),
            display(_display)
        {
        }

        virtual void onStepPressed(const InputEvent& inputEvent) {}
        virtual void onStepReleased(const InputEvent& inputEvent) {}

        virtual void onEncoder(const InputEvent& inputEvent) {}

        virtual void applyEncoderMapping() {}
        virtual void applyEncoderValues() {}

        virtual void onButtonTap(const InputEvent& event) {}
        virtual void onButtonHold(const InputEvent& event) {}
};
