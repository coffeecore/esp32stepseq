#pragma once

#include "MenuType.h"

class Menu;
class MenuManager;

// Callback pour un item de type ACTION : recoit le MenuManager pour
// pouvoir, si besoin, naviguer depuis le callback lui-meme (ex: pousser
// un autre menu, revenir en arriere...).
using MenuCallback = void (*)(MenuManager&);

// Un item de menu. Selon `type`, seuls certains champs ont un sens :
//   ACTION    -> callback
//   SUBMENU   -> submenu
//   BOOL/INT/FLOAT/ENUM -> value + descriptor
//   LABEL/SEPARATOR -> seulement label (le reste reste a nullptr)
//
// ATTENTION si tu modifies l'ordre des champs ci-dessous : Menu.h
// construit des MenuItem par initialisation positionnelle
// (ex: `{l, MenuType::ACTION, {}, nullptr, cb, nullptr}`), qui doit
// correspondre EXACTEMENT a cet ordre. Reordonner les champs sans mettre
// a jour Menu.h casserait silencieusement l'assignation des valeurs.
struct MenuItem {
    const char* label = nullptr;
    MenuType type = MenuType::LABEL;
    MenuValue value;
    const MenuDescriptor* descriptor = nullptr;
    MenuCallback callback = nullptr;
    Menu* submenu = nullptr;
};
