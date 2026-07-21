#pragma once
#include "Menu.h"

enum class MenuState : uint8_t { Browsing, Editing };
struct MenuContext { Menu* menu=nullptr; uint8_t selected=0; };

class MenuManager {
public:
    static constexpr uint8_t MAX_DEPTH=8;
    void begin(Menu* root){ _depth=0; _stack[0]={root,0}; _state=MenuState::Browsing; }
    Menu* currentMenu(){return _stack[_depth].menu;}
    const Menu* currentMenu()const{return _stack[_depth].menu;}
    MenuItem* currentItem(){ Menu* m=currentMenu(); if(!m||!m->count()) return nullptr; return &m->item(_stack[_depth].selected); }
    uint8_t selectedIndex()const{return _stack[_depth].selected;}
    MenuState state()const{return _state;}
    bool isEditing()const{return _state==MenuState::Editing;}
    void next(){ if(!_hasItems()) return; if(_state==MenuState::Editing){if(auto*i=currentItem();i->descriptor)i->descriptor->next(i->value);return;} _move(+1); }
    void prev(){ if(!_hasItems()) return; if(_state==MenuState::Editing){if(auto*i=currentItem();i->descriptor)i->descriptor->prev(i->value);return;} _move(-1); }
    bool enter(){ MenuItem* i=currentItem(); if(!i)return false; switch(i->type){case MenuType::ACTION:if(i->callback)i->callback(*this);return true;case MenuType::SUBMENU:return push(i->submenu);case MenuType::BOOL:if(i->descriptor)i->descriptor->next(i->value);return true;case MenuType::INT:case MenuType::FLOAT:case MenuType::ENUM:_state=MenuState::Editing;return true;default:return false;} }
    bool back(){ if(_state==MenuState::Editing){_state=MenuState::Browsing;return true;} return pop(); }
    bool push(Menu* m){ if(!m||_depth>=MAX_DEPTH-1)return false; ++_depth; _stack[_depth]={m,0}; _state=MenuState::Browsing; return true; }
    bool pop(){ if(!_depth)return false; --_depth; _state=MenuState::Browsing; return true; }
    void reset(){if(!_stack[0].menu)return;_depth=0;_stack[0].selected=0;_state=MenuState::Browsing;}
private:
    bool _hasItems()const{const Menu*m=currentMenu();return m&&m->count();}
    void _move(int d){Menu*m=currentMenu();if(!m||!m->count())return;uint8_t&s=_stack[_depth].selected;if(d>0)s=(s+1)%m->count();else s=s? s-1:m->count()-1;}
    MenuContext _stack[MAX_DEPTH]{};
    uint8_t _depth=0;
    MenuState _state=MenuState::Browsing;
};
