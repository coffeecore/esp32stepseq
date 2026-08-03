#pragma once
#include <U8g2lib.h>
#include "Menu/MenuManager.h"
namespace Display::Screens
{

class Screen
{
public:
    UIState& uiState;
    Sequencer::Sequencer& sequencer;
    U8G2& u8g2;
    IAudioEngine& audioEngine;

    explicit Screen(UIState& uiState, Sequencer::Sequencer& sequencer, U8G2& u8g2, IAudioEngine& audioEngine)
        : uiState(uiState)
        , sequencer(sequencer)
        , u8g2(u8g2)
        , audioEngine(audioEngine)
    {
    }

    virtual void draw() = 0;

    // Cycle de vie optionnel, appele par DisplayEngine a chaque fois que
    // ui.uiOverlay change (quel que soit l'overlay, Menu, Confirm, etc.),
    // que l'ecran actif change en meme temps ou non. No-op par defaut :
    // un ecran qui n'en a pas besoin (SequencerScreen par ex.) n'a rien
    // a faire. `overlay` precise CE QUI est quitte / entre, a l'ecran de
    // filtrer s'il ne s'interesse qu'a un overlay en particulier.
    // Un MenuScreen qui edite des valeurs via des copies locales (cas des
    // enums, cf. InstrumentScreen) surchargera onEnter() pour initialiser
    // ces copies et onExit() pour les reporter vers l'objet reel.
    virtual void onEnter(UIOverlay overlay)
    {
    }

    virtual void onExit(UIOverlay overlay)
    {
    }
};

class MenuScreen : public Screen
{
public:
    MenuScreen(UIState& uiState, Sequencer::Sequencer& sequencer, U8G2& u8g2, IAudioEngine& audioEngine, MenuManager& menuManager)
        : Screen(uiState, sequencer, u8g2, audioEngine)
        , menuManager(menuManager)
    {
    }

    virtual void drawMenu()
    {
        const Menu* menu = menuManager.currentMenu();

        if (nullptr == menu) {
            return;
        }

        constexpr uint8_t FIRST_ROW = 21;
        constexpr uint8_t ROW_HEIGHT = 8;
        uint8_t x = 0;
        uint8_t y = 0;
        x = u8g2.getMaxCharWidth() * 2;
        y = u8g2.getAscent() + 0 * u8g2.getMaxCharHeight();

        // Title
        u8g2.drawStr(0, 8, menu->title());

        u8g2.drawHLine(0, 10, 128);

        for (uint8_t i = 0; i < menu->count(); ++i) {
            const MenuItem& item = menu->item(i);

            const uint8_t y = FIRST_ROW + (i * ROW_HEIGHT);

            const bool selected = i == menuManager.selectedIndex();

            if (item.type == MenuType::SEPARATOR) {
                u8g2.drawHLine(0, y - 6, 128);

                continue;
            }

            if (selected) {
                u8g2.drawStr(0, y, ">");
            }

            if (item.label) {
                u8g2.drawStr(8, y, item.label);
            }

            if (!item.descriptor || !item.value.ptr) {
                continue;
            }

            char buffer[24];

            const char* value = item.descriptor->format(item.value, buffer, sizeof(buffer));

            if (!value) {
                continue;
            }

            const uint8_t valueWidth = u8g2.getStrWidth(value);

            const uint8_t valueX = 127 - valueWidth;

            if (selected && menuManager.isEditing()) {
                u8g2.drawStr(valueX - 6, y, "[");

                u8g2.drawStr(valueX, y, value);

                u8g2.drawStr(124, y, "]");
            }
            else {
                u8g2.drawStr(valueX, y, value);
            }
        }
    }

protected:
    MenuManager& menuManager;
};

class ConfirmScreen : public Screen
{
    using Screen::Screen;

public:
    virtual void drawConfirm()
    {
        if (uiState.uiOverlay == UIOverlay::Confirm) {
            drawConfirm();
            u8g2.sendBuffer();
        }
    }
};
}
