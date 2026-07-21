#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

enum class MenuType : uint8_t { ACTION, SUBMENU, BOOL, INT, FLOAT, ENUM, LABEL, SEPARATOR };

union MenuValue {
    void* ptr;
    bool* boolean;
    int32_t* integer;
    float* floating;
    uint8_t* enumeration;
    constexpr MenuValue() : ptr(nullptr) {}
    constexpr MenuValue(bool* v) : boolean(v) {}
    constexpr MenuValue(int32_t* v) : integer(v) {}
    constexpr MenuValue(float* v) : floating(v) {}
    constexpr MenuValue(uint8_t* v) : enumeration(v) {}
};

using IntFormatCallback = void (*)(char*, size_t, int32_t);
using FloatFormatCallback = void (*)(char*, size_t, float);
using BoolFormatCallback = const char* (*)(bool);

struct EnumOption { uint8_t value; const char* label; };

struct MenuDescriptor {
    MenuType type;
    constexpr explicit MenuDescriptor(MenuType t) : type(t) {}
    virtual void next(MenuValue) const {}
    virtual void prev(MenuValue) const {}
    virtual const char* format(MenuValue, char*, size_t) const { return ""; }
};

struct BoolDescriptor : MenuDescriptor {
    const char* trueLabel;
    const char* falseLabel;
    BoolFormatCallback formatter;
    constexpr BoolDescriptor(const char* on="ON", const char* off="OFF", BoolFormatCallback f=nullptr)
        : MenuDescriptor(MenuType::BOOL), trueLabel(on), falseLabel(off), formatter(f) {}
    void next(MenuValue v) const override { if (v.boolean) *v.boolean = !*v.boolean; }
    void prev(MenuValue v) const override { next(v); }
    const char* format(MenuValue v, char*, size_t) const override {
        if (!v.boolean) return "?";
        return formatter ? formatter(*v.boolean) : (*v.boolean ? trueLabel : falseLabel);
    }
};

struct IntDescriptor : MenuDescriptor {
    int32_t min, max, step;
    IntFormatCallback formatter;
    constexpr IntDescriptor(int32_t lo, int32_t hi, int32_t s=1, IntFormatCallback f=nullptr)
        : MenuDescriptor(MenuType::INT), min(lo), max(hi), step(s), formatter(f) {}
    void next(MenuValue v) const override { if (!v.integer) return; *v.integer += step; if (*v.integer > max) *v.integer = max; }
    void prev(MenuValue v) const override { if (!v.integer) return; *v.integer -= step; if (*v.integer < min) *v.integer = min; }
    const char* format(MenuValue v, char* b, size_t n) const override {
        if (!v.integer || !b || !n) return "?";
        if (formatter) { formatter(b,n,*v.integer); return b; }
        snprintf(b,n,"%ld",(long)*v.integer); return b;
    }
};

struct FloatDescriptor : MenuDescriptor {
    float min, max, step;
    FloatFormatCallback formatter;
    constexpr FloatDescriptor(float lo, float hi, float s=0.1f, FloatFormatCallback f=nullptr)
        : MenuDescriptor(MenuType::FLOAT), min(lo), max(hi), step(s), formatter(f) {}
    void next(MenuValue v) const override { if (!v.floating) return; *v.floating += step; if (*v.floating > max) *v.floating = max; }
    void prev(MenuValue v) const override { if (!v.floating) return; *v.floating -= step; if (*v.floating < min) *v.floating = min; }
    const char* format(MenuValue v, char* b, size_t n) const override {
        if (!v.floating || !b || !n) return "?";
        if (formatter) { formatter(b,n,*v.floating); return b; }
        snprintf(b,n,"%.2f",(double)*v.floating); return b;
    }
};

struct EnumDescriptor : MenuDescriptor {
    const EnumOption* options;
    uint8_t count;
    constexpr EnumDescriptor(const EnumOption* o, uint8_t c)
        : MenuDescriptor(MenuType::ENUM), options(o), count(c) {}
    void next(MenuValue v) const override {
        if (!v.enumeration || !options || !count) return;
        for (uint8_t i=0;i<count;++i) if (options[i].value==*v.enumeration) { *v.enumeration=options[(i+1)%count].value; return; }
        *v.enumeration=options[0].value;
    }
    void prev(MenuValue v) const override {
        if (!v.enumeration || !options || !count) return;
        for (uint8_t i=0;i<count;++i) if (options[i].value==*v.enumeration) { *v.enumeration=options[(i+count-1)%count].value; return; }
        *v.enumeration=options[0].value;
    }
    const char* format(MenuValue v, char*, size_t) const override {
        if (!v.enumeration || !options) return "?";
        for (uint8_t i=0;i<count;++i) if (options[i].value==*v.enumeration) return options[i].label;
        return "?";
    }
};
