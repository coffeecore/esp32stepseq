# Menu V2

Petite bibliothèque de menus, uniquement en headers.

- `MenuType.h`: types, valeurs et descriptors intelligents. Les descriptors savent modifier et formater leur valeur.
- `MenuItem.h`: structure d'un item de menu.
- `Menu.h`: stockage fixe des items et helpers `addBool`, `addInt`, `addFloat`, `addEnum`, `addAction`, `addSubmenu`, etc.
- `MenuManager.h`: sélection, édition et navigation par stack. Le manager ne dessine rien et ne possède pas les menus.

## Exemple

```cpp
enum class ClockSource : uint8_t { Internal, MIDI, USB };

uint8_t clock = static_cast<uint8_t>(ClockSource::Internal);
int32_t tempo = 120;

constexpr EnumOption clockOptions[] = {
    { static_cast<uint8_t>(ClockSource::Internal), "Internal" },
    { static_cast<uint8_t>(ClockSource::MIDI), "MIDI" },
    { static_cast<uint8_t>(ClockSource::USB), "USB" }
};
constexpr EnumDescriptor clockDescriptor(clockOptions, 3);
constexpr IntDescriptor tempoDescriptor(20, 300, 1);

Menu midi("MIDI");
Menu settings("Settings");
MenuManager manager;

void setupMenu() {
    midi.addEnum("Clock", &clock, &clockDescriptor);
    settings.addInt("Tempo", &tempo, &tempoDescriptor);
    settings.addSubmenu("MIDI", &midi);
    manager.begin(&settings);
}
```

## Dans ton Screen

```cpp
void onNext() { manager.next(); }
void onPrev() { manager.prev(); }
void onEnter() { manager.enter(); }
void onBack() { manager.back(); }
```

Pour afficher une valeur:

```cpp
char buffer[32];
MenuItem* item = manager.currentItem();
const char* text = item->descriptor->format(item->value, buffer, sizeof(buffer));
```

La stack mémorise le menu et l'index sélectionné. En revenant avec `back()`, le menu parent et sa sélection sont restaurés.
