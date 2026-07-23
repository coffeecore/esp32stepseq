#pragma once

#include "Menu.h"

// Browsing = on deplace la selection (next/prev bougent le curseur).
// Editing  = on modifie la VALEUR de l'item selectionne (next/prev
// appellent le descriptor). On entre en Editing via enter() sur un
// INT/FLOAT/ENUM, et on en ressort via back().
enum class MenuState : uint8_t { Browsing, Editing };

// Un "cran" de la pile de navigation : quel Menu est affiche, et quel
// item y est selectionne. En empilant un MenuContext par sous-menu
// visite (push), back()/pop() peut restaurer exactement l'etat precedent
// (menu ET selection), sans avoir besoin d'un pointeur "parent" dans Menu.
struct MenuContext {
    Menu* menu = nullptr;
    uint8_t selected = 0;
};

// Gere la navigation (pile de menus), la selection courante et le mode
// Browsing/Editing. NE DESSINE RIEN et NE POSSEDE AUCUN Menu (les Menu*
// sont fournis par l'appelant et doivent rester valides tout le temps ou
// le MenuManager les reference).
class MenuManager {
    public:
        // Profondeur max de sous-menus imbriques (Settings > MIDI > ... )
        static constexpr uint8_t MAX_DEPTH = 8;

        // (Re)initialise la navigation a la racine `root`, selection au debut.
        void begin(Menu* root) {
            _depth = 0;
            _stack[0] = { root, 0 };
            _state = MenuState::Browsing;
        }

        Menu* currentMenu() { return _stack[_depth].menu; }
        const Menu* currentMenu() const { return _stack[_depth].menu; }

        // Item actuellement selectionne dans le menu courant, ou nullptr
        // si le menu est vide (aucun item ajoute, ou pointeur null).
        MenuItem* currentItem() {
            Menu* m = currentMenu();
            if (!m || !m->count()) return nullptr;
            return &m->item(_stack[_depth].selected);
        }

        uint8_t selectedIndex() const { return _stack[_depth].selected; }
        MenuState state() const { return _state; }
        bool isEditing() const { return _state == MenuState::Editing; }

        // Profondeur actuelle dans la pile (0 = menu racine). Utile pour
        // afficher un fil d'Ariane ("Settings > MIDI") sans dupliquer la
        // logique de pile ailleurs.
        uint8_t depth() const { return _depth; }

        // En Browsing : deplace la selection au suivant (boucle en fin de liste).
        // En Editing   : delegue au descriptor de l'item courant (ex: +1 sur un int).
        void next() {
            if (!_hasItems()) return;

            if (_state == MenuState::Editing) {
                if (auto* i = currentItem(); i->descriptor) {
                    i->descriptor->next(i->value);
                }
                return;
            }

            _move(+1);
        }

        // Symetrique de next().
        void prev() {
            if (!_hasItems()) return;

            if (_state == MenuState::Editing) {
                if (auto* i = currentItem(); i->descriptor) {
                    i->descriptor->prev(i->value);
                }
                return;
            }

            _move(-1);
        }

        // Comportement au clic/validation, qui depend du type d'item :
        //   ACTION  -> execute le callback
        //   SUBMENU -> descend dans le sous-menu (push)
        //   BOOL    -> bascule IMMEDIATEMENT (pas de mode Editing pour un bool,
        //              un seul clic suffit puisqu'il n'y a que 2 etats)
        //   INT/FLOAT/ENUM -> entre en mode Editing (next/prev ajusteront la valeur)
        //   LABEL/SEPARATOR (default) -> non interactifs, ne fait rien
        bool enter() {
            MenuItem* i = currentItem();
            if (!i) return false;

            switch (i->type) {
                case MenuType::ACTION:
                    if (i->callback) i->callback(*this);
                    return true;

                case MenuType::SUBMENU:
                    return push(i->submenu);

                case MenuType::BOOL:
                    // Garde ajoutee : un BOOL sans descriptor (ex: construit
                    // "a la main" sans passer par Menu::addBool) ne doit pas
                    // faire planter enter(), juste rester sans effet.
                    if (i->descriptor) i->descriptor->next(i->value);
                    return true;

                case MenuType::INT:
                case MenuType::FLOAT:
                case MenuType::ENUM:
                    _state = MenuState::Editing;
                    return true;

                default:
                    return false;
            }
        }

        // Retour arriere : si on editait une valeur, on repasse juste en
        // Browsing (PAS d'annulation/undo -> la valeur deja modifiee par
        // next()/prev() reste appliquee). Sinon, on remonte d'un niveau
        // dans la pile de menus (pop).
        bool back() {
            if (_state == MenuState::Editing) {
                _state = MenuState::Browsing;
                return true;
            }
            return pop();
        }

        // Empile un nouveau menu (entree dans un sous-menu). Refuse si la
        // pile est pleine (MAX_DEPTH) ou si `m` est null.
        bool push(Menu* m) {
            if (!m || _depth >= MAX_DEPTH - 1) return false;
            ++_depth;
            _stack[_depth] = { m, 0 };
            _state = MenuState::Browsing;
            return true;
        }

        // Depile (retour au menu parent). La selection du parent est
        // automatiquement restauree puisqu'elle n'a jamais ete ecrasee
        // (elle etait juste plus bas dans la pile, inchangee).
        bool pop() {
            if (!_depth) return false; // deja a la racine
            --_depth;
            _state = MenuState::Browsing;
            return true;
        }

        // Revient a la racine (_stack[0]) sans perdre le pointeur de menu
        // racine, en reinitialisant la selection.
        void reset() {
            if (!_stack[0].menu) return;
            _depth = 0;
            _stack[0].selected = 0;
            _state = MenuState::Browsing;
        }

    private:
        bool _hasItems() const {
            const Menu* m = currentMenu();
            return m && m->count();
        }

        // Deplace la selection de `d` (+1 ou -1) avec bouclage circulaire
        // (dernier item -> premier, et premier -> dernier).
        void _move(int d) {
            Menu* m = currentMenu();
            if (!m || !m->count()) return;

            uint8_t& s = _stack[_depth].selected;
            if (d > 0) {
                s = (s + 1) % m->count();
            } else {
                s = s ? s - 1 : m->count() - 1;
            }
        }

        MenuContext _stack[MAX_DEPTH]{};
        uint8_t _depth = 0;
        MenuState _state = MenuState::Browsing;
};
