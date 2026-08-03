#pragma once

#include "MenuItem.h"

// Un menu = une liste FIXE (pas de heap, pas de vector) de MenuItem,
// jusqu'a MAX_ITEMS. Choix delibere pour un microcontroleur : taille
// connue a la compilation, pas d'allocation dynamique, pas de risque de
// fragmentation memoire au fil du temps.
class Menu
{
public:
    static constexpr uint8_t MAX_ITEMS = 16;

    explicit Menu(const char* title = nullptr)
        : _title(title)
    {
    }

    void clear()
    {
        _count = 0;
    }

    void setTitle(const char* title)
    {
        _title = title;
    }

    // Ajoute un item "brut" deja construit. Retourne false si le menu
    // est plein (16 items atteints) plutot que de deborder silencieusement.
    bool addItem(const MenuItem& menuItem)
    {
        if (_count >= MAX_ITEMS) {
            return false;
        }

        _items[_count++] = menuItem;

        return true;
    }

    // --- Helpers de construction, un par type d'item ---
    // Chacun construit le MenuItem correspondant et l'ajoute via addItem().

    bool addAction(const char* label, MenuCallback menuCallback, void* context = nullptr)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::ACTION;
        menuItem.callback = menuCallback;
        menuItem.context = context;

        return addItem(menuItem);
    }

    bool addSubmenu(const char* label, Menu* menu)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::SUBMENU;
        menuItem.submenu = menu;

        return addItem(menuItem);
    }

    bool addBool(const char* label, bool* value, const BoolDescriptor* descriptor)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::BOOL;
        menuItem.value = MenuValue(value);
        menuItem.descriptor = descriptor;

        return addItem(menuItem);
    }

    bool addInt(const char* label, int32_t* value, const IntDescriptor* descriptor)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::INT;
        menuItem.value = MenuValue(value);
        menuItem.descriptor = descriptor;

        return addItem(menuItem);
    }

    bool addFloat(const char* label, float* value, const FloatDescriptor* descriptor)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::FLOAT;
        menuItem.value = MenuValue(value);
        menuItem.descriptor = descriptor;

        return addItem(menuItem);
    }

    bool addEnum(const char* label, int8_t* value, const EnumDescriptor* descriptor)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::ENUM;
        menuItem.value = MenuValue(value);
        menuItem.descriptor = descriptor;

        return addItem(menuItem);
    }

    bool addLabel(const char* label)
    {
        MenuItem menuItem = MenuItem();
        menuItem.label = label;
        menuItem.type = MenuType::LABEL;

        return addItem(menuItem);
    }

    bool addSeparator()
    {
        MenuItem menuItem = MenuItem();
        menuItem.type = MenuType::SEPARATOR;

        return addItem(menuItem);
    }

    const char* title() const
    {
        return _title;
    }

    uint8_t count() const
    {
        return _count;
    }

    MenuItem& item(uint8_t i)
    {
        return _items[i];
    }

    const MenuItem& item(uint8_t i) const
    {
        return _items[i];
    }

private:
    const char* _title;
    MenuItem _items[MAX_ITEMS]{};
    uint8_t _count = 0;
};
