#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// Type "structurel" d'un item de menu : dit au MenuManager COMMENT
// naviguer/interagir avec l'item (entrer en édition ? descendre dans un
// sous-menu ? déclencher une action ?). Ne dit PAS comment la valeur en
// elle-même se modifie ou s'affiche : ça, c'est le rôle des descriptors
// polymorphes plus bas (BoolDescriptor, IntDescriptor, etc.).
enum class MenuType : uint8_t {
    ACTION,     // déclenche un callback au enter(), pas de valeur
    SUBMENU,    // descend dans un autre Menu au enter()
    BOOL,       // bascule true/false, applique immediatement au enter()
    INT,        // entre en mode Editing au enter(), next()/prev() modifient
    FLOAT,      // idem INT mais flottant
    ENUM,       // idem INT mais parcourt une liste d'options nommees
    LABEL,      // texte informatif, non interactif
    SEPARATOR   // ligne vide/trait, non interactif
};

// Union type-safe : un MenuItem ne stocke QU'UN SEUL pointeur a la fois
// (bool*, int32_t*, float* ou uint8_t* pour un enum). Les constructeurs
// constexpr evitent d'avoir a preciser quel champ de l'union remplir a
// chaque fois qu'on construit un MenuValue : le compilateur choisit le
// bon constructeur selon le type du pointeur passe.
union MenuValue {
    void* ptr;
    bool* boolean;
    int32_t* integer;
    float* floating;
    int8_t* enumeration; // pointe vers la valeur "brute" d'un enum (son underlying type)

    constexpr MenuValue() : ptr(nullptr) {}
    constexpr MenuValue(bool* v) : boolean(v) {}
    constexpr MenuValue(int32_t* v) : integer(v) {}
    constexpr MenuValue(float* v) : floating(v) {}
    constexpr MenuValue(int8_t* v) : enumeration(v) {}
};

// Callbacks de formatage optionnels : si tu veux un affichage custom
// (ex: "120 BPM" au lieu de juste "120", ou "12.5 dB" au lieu de "12.50"),
// tu fournis une de ces fonctions au descriptor. Sinon, le format par
// defaut de chaque descriptor (snprintf simple) est utilise.
using IntFormatCallback   = void (*)(char*, size_t, int32_t);
using FloatFormatCallback = void (*)(char*, size_t, float);
using BoolFormatCallback  = const char* (*)(bool);

// Une entree d'un menu ENUM : la valeur brute stockee (doit correspondre a
// l'underlying type de ton enum, caste en uint8_t) + le texte affiche.
struct EnumOption {
    int8_t value;
    const char* label;
};

// --- Descriptor polymorphe : le coeur de la logique "pas de switch" ---
//
// Au lieu que MenuManager fasse un switch(item.type) pour savoir comment
// incrementer/decrementer/formatter une valeur, chaque type de donnee
// (Bool/Int/Float/Enum) fournit sa PROPRE implementation de next/prev/format
// via l'heritage virtuel. MenuManager appelle juste
// `item->descriptor->next(item->value)` sans jamais savoir quel descriptor
// concret il manipule. Ajouter un nouveau type de valeur plus tard
// (Color, MidiChannel, Scale...) = creer UNE nouvelle classe qui herite de
// MenuDescriptor, sans toucher a MenuManager.
struct MenuDescriptor {
    MenuType type; // utile pour du debug/introspection, pas pour le dispatch (qui est virtuel)

    constexpr explicit MenuDescriptor(MenuType t) : type(t) {}

    // Implementations par defaut vides : une base "no-op" si jamais un
    // descriptor ne supporte pas next/prev (ex: on pourrait imaginer un
    // futur descriptor en lecture seule).
    virtual void next(MenuValue) const {}
    virtual void prev(MenuValue) const {}
    virtual const char* format(MenuValue, char*, size_t) const {
        return "?";
    }
};

// --- BOOL : bascule true/false. next() et prev() font la meme chose ---
// (un booleen n'a que 2 etats, "suivant" et "precedent" sont equivalents).
struct BoolDescriptor : MenuDescriptor {
    const char* trueLabel;
    const char* falseLabel;
    BoolFormatCallback formatter; // optionnel, voir plus haut

    constexpr BoolDescriptor(const char* on = "ON", const char* off = "OFF", BoolFormatCallback f = nullptr)
        : MenuDescriptor(MenuType::BOOL), trueLabel(on), falseLabel(off), formatter(f) {}

    void next(MenuValue v) const override {
        if (v.boolean) {
            *v.boolean = !*v.boolean;
        }
    }

    void prev(MenuValue v) const override {
        next(v); // meme comportement que next() pour un bool
    }

    const char* format(MenuValue v, char*, size_t) const override {
        if (!v.boolean) {
            return "?"; // valeur non liee (ne devrait pas arriver en usage normal)
        }

        return formatter ? formatter(*v.boolean) : (*v.boolean ? trueLabel : falseLabel);
    }
};

// --- INT : increment/decrement bornes par [min, max] avec un pas fixe ---
struct IntDescriptor : MenuDescriptor {
    int32_t min, max, step;
    IntFormatCallback formatter;

    constexpr IntDescriptor(int32_t lo, int32_t hi, int32_t s = 1, IntFormatCallback f = nullptr)
        : MenuDescriptor(MenuType::INT), min(lo), max(hi), step(s), formatter(f) {}

    void next(MenuValue v) const override {
        if (!v.integer) {
            return;
        }

        *v.integer += step;

        if (*v.integer > max) {
            *v.integer = max; // clamp, pas de wrap circulaire
        }
    }

    void prev(MenuValue v) const override {
        if (!v.integer) {
            return;
        }

        *v.integer -= step;

        if (*v.integer < min) {
            *v.integer = min;
        }
    }

    const char* format(MenuValue v, char* b, size_t n) const override {
        if (!v.integer || !b || !n) {
            return "?";
        }

        if (formatter) { 
            formatter(b, n, *v.integer);

            return b;
        }

        snprintf(b, n, "%ld", (long)*v.integer);

        return b;
    }
};

// --- FLOAT : identique a INT mais en flottant (formatage par defaut a 2 decimales) ---
struct FloatDescriptor : MenuDescriptor {
    float min, max, step;
    FloatFormatCallback formatter;

    constexpr FloatDescriptor(float lo, float hi, float s = 0.1f, FloatFormatCallback f = nullptr)
        : MenuDescriptor(MenuType::FLOAT), min(lo), max(hi), step(s), formatter(f) {}

    void next(MenuValue v) const override {
        if (!v.floating) {
            return;
        }

        *v.floating += step;

        if (*v.floating > max) {
            *v.floating = max;
        }
    }

    void prev(MenuValue v) const override {
        if (!v.floating) {
            return;
        }

        *v.floating -= step;

        if (*v.floating < min) {
            *v.floating = min;
        }
    }

    const char* format(MenuValue v, char* b, size_t n) const override {
        if (!v.floating || !b || !n) {
            return "?";
        }

        if (formatter) {
            formatter(b, n, *v.floating);
            return b;
        }

        snprintf(b, n, "%.2f", (double)*v.floating);

        return b;
    }
};

// --- ENUM : parcourt une liste ordonnee d'options (pas de min/max/step) ---
// next()/prev() cherchent la valeur ACTUELLE dans la liste `options`, puis
// avancent/reculent d'une position, en bouclant (dernier -> premier et
// inversement). Si la valeur stockee ne correspond a AUCUNE option (donnee
// corrompue, ou valeur jamais initialisee), on retombe silencieusement sur
// options[0] plutot que de planter ou de rester bloque.
struct EnumDescriptor : MenuDescriptor {
    const EnumOption* options;
    uint8_t count;

    constexpr EnumDescriptor(const EnumOption* o, uint8_t c)
        : MenuDescriptor(MenuType::ENUM), options(o), count(c) {}

    void next(MenuValue v) const override
    {
        if (!v.enumeration || !options || !count) {
            return;
        }

        for (uint8_t i = 0; i < count; ++i) {
            if (options[i].value == *v.enumeration) {
                *v.enumeration = options[(i + 1) % count].value;

                return;
            }
        }

        *v.enumeration = options[0].value; // valeur actuelle non trouvee -> fallback
    }

    void prev(MenuValue v) const override
    {
        if (!v.enumeration || !options || !count) {
            return;
        }

        for (uint8_t i = 0; i < count; ++i) {
            if (options[i].value == *v.enumeration) {
                *v.enumeration = options[(i + count - 1) % count].value; // "-1" circulaire

                return;
            }
        }

        *v.enumeration = options[0].value;
    }

    const char* format(MenuValue v, char*, size_t) const override
    {
        if (!v.enumeration || !options) {
            return "?";
        }

        for (uint8_t i = 0; i < count; ++i) {
            if (options[i].value == *v.enumeration) {
                return options[i].label;
            }
        }

        return "?"; // valeur sans option correspondante
    }
};
