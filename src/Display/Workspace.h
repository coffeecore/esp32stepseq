#pragma once

#include "Input/InputMode.h"

enum class Workspace
{
    Sequencer
};

struct ConfirmDialog
{
    bool active = false;
    const char* text = nullptr;
};

struct UIState
{
    Workspace workspace = Workspace::Sequencer;

    ModalState modal = ModalState::None;

    uint8_t selectedInstrument = 0;

    uint8_t displayedTrack = 0;

    uint8_t selectedQuarterNote = 0;

    uint8_t selectedStep = 0;

    uint8_t selectedTrack = 0;

    ConfirmDialog confirm;

    TaskHandle_t displayTask = nullptr;

    bool autoScroll = false;

    void requestRedraw()
    {
        if (displayTask) {
            xTaskNotifyGive(displayTask);
        }
    }

};
