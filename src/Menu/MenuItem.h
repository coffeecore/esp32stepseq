#pragma once

#include "MenuType.h"

class Menu;
class MenuManager;

// Callback pour un item de type ACTION : recoit le MenuManager pour
// pouvoir, si besoin, naviguer depuis le callback lui-meme (ex: pousser
// un autre menu, revenir en arriere...).
using MenuCallback = void (*)(void* context, MenuManager&);

// Un item de menu. Selon `type`, seuls certains champs ont un sens :
//   ACTION    -> callback
//   SUBMENU   -> submenu
//   BOOL/INT/FLOAT/ENUM -> value + descriptor
//   LABEL/SEPARATOR -> seulement label (le reste reste a nullptr)
struct MenuItem
{
    const char* label = nullptr;
    MenuType type = MenuType::LABEL;
    MenuValue value;
    const MenuDescriptor* descriptor = nullptr;
    MenuCallback callback = nullptr;
    Menu* submenu = nullptr;

    void* context = nullptr;
};
