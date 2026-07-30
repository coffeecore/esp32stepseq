#pragma once

#include "Input/InputMode.h"

enum class Workspace
{
    Sequencer,
    InstrumentMenu,
    InstrumentAdsr

};

enum class UIOverlay : uint8_t
{
    None,

    Menu,
    Confirm
};

struct ConfirmDialog
{
    const char* text = nullptr;

    void open(const char* message)
    {
        text = message;
    }

    void close()
    {
        text = nullptr;
    }
};

struct UIState
{
    Workspace workspace = Workspace::Sequencer;

    UIOverlay uiOverlay = UIOverlay::None;

    // ModalState modal = ModalState::None;


    uint8_t selectedInstrument = 0;

    uint8_t displayedTrack = 0;

    uint8_t selectedQuarterNote = 0;

    uint8_t selectedStep = 0;

    uint8_t selectedTrack = 0;

    ConfirmDialog confirm;

    TaskHandle_t displayTask = nullptr;

    bool autoScroll = false;

    bool displayTrackInfo = false;

    void openConfirm(const char* message)
    {
        confirm.open(message);
        uiOverlay = UIOverlay::Confirm;
    }

    void closeConfirm()
    {
        confirm.close();
        uiOverlay = UIOverlay::None;
    }

    void requestRedraw()
    {
        if (displayTask) {
            xTaskNotifyGive(displayTask);
        }
    }

};
