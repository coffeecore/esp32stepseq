#pragma once
#include "MenuType.h"
class Menu; class MenuManager;
using MenuCallback = void (*)(MenuManager&);
struct MenuItem {
    const char* label = nullptr;
    MenuType type = MenuType::LABEL;
    MenuValue value;
    const MenuDescriptor* descriptor = nullptr;
    MenuCallback callback = nullptr;
    Menu* submenu = nullptr;
};
