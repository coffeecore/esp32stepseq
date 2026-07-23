#pragma once

#include "MenuItem.h"

// Un menu = une liste FIXE (pas de heap, pas de vector) de MenuItem,
// jusqu'a MAX_ITEMS. Choix delibere pour un microcontroleur : taille
// connue a la compilation, pas d'allocation dynamique, pas de risque de
// fragmentation memoire au fil du temps.
class Menu {
    public:
        static constexpr uint8_t MAX_ITEMS = 16;

        explicit Menu(const char* title = nullptr) : _title(title) {}

        // Ajoute un item "brut" deja construit. Retourne false si le menu
        // est plein (16 items atteints) plutot que de deborder silencieusement.
        bool addItem(const MenuItem& i) {
            if (_count >= MAX_ITEMS) return false;
            _items[_count++] = i;
            return true;
        }

        // --- Helpers de construction, un par type d'item ---
        // Chacun construit le MenuItem correspondant et l'ajoute via addItem().
        // C'est ICI que l'ordre des champs de MenuItem doit rester synchronise
        // (voir l'avertissement dans MenuItem.h).

        bool addAction(const char* l, MenuCallback cb) {
            return addItem({ l, MenuType::ACTION, {}, nullptr, cb, nullptr });
        }

        bool addSubmenu(const char* l, Menu* m) {
            return addItem({ l, MenuType::SUBMENU, {}, nullptr, nullptr, m });
        }

        bool addBool(const char* l, bool* v, const BoolDescriptor* d) {
            return addItem({ l, MenuType::BOOL, MenuValue(v), d, nullptr, nullptr });
        }

        bool addInt(const char* l, int32_t* v, const IntDescriptor* d) {
            return addItem({ l, MenuType::INT, MenuValue(v), d, nullptr, nullptr });
        }

        bool addFloat(const char* l, float* v, const FloatDescriptor* d) {
            return addItem({ l, MenuType::FLOAT, MenuValue(v), d, nullptr, nullptr });
        }

        bool addEnum(const char* l, uint8_t* v, const EnumDescriptor* d) {
            return addItem({ l, MenuType::ENUM, MenuValue(v), d, nullptr, nullptr });
        }

        bool addLabel(const char* l) {
            return addItem({ l, MenuType::LABEL, {}, nullptr, nullptr, nullptr });
        }

        bool addSeparator() {
            return addItem({ nullptr, MenuType::SEPARATOR, {}, nullptr, nullptr, nullptr });
        }

        const char* title() const { return _title; }
        uint8_t count() const { return _count; }

        MenuItem& item(uint8_t i) { return _items[i]; }
        const MenuItem& item(uint8_t i) const { return _items[i]; }

    private:
        const char* _title;
        MenuItem _items[MAX_ITEMS]{};
        uint8_t _count = 0;
};
