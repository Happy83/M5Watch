/*
  Menu.h - Library for universal menu.
  Created by Petr Vavrin, 2019
  Released into the public domain.
*/

#ifndef Menu_h
#define Menu_h

#include "Arduino.h"

class Menu
{
  public:
    struct ITEM {
      Menu::ITEM * parent;
      Menu::ITEM * prev;
      Menu::ITEM * next;
      Menu::ITEM * child;
      int uid;
      char * text;
      bool disabled;
      bool (* cb)();
      bool (* value_cb)(bool isItemLine);
      char hotkey;
    };
    Menu::ITEM * mainItem;
    Menu::ITEM * activeItem;
    Menu::ITEM rootItem;
    int itemCounter = 0;

    bool showTitle = true;
    bool showValue = true;
    bool showPreviousitems = true;
    bool circular = true;
    bool markActive = true;
    bool inlineValue = true;
    byte itemsLimit = 99;

    byte KEY_UP = 1;
    byte KEY_DOWN = 2;
    byte KEY_LEFT = 3;
    byte KEY_RIGHT = 4;
    byte KEY_ENTER = 5;
    byte KEY_RIGHT_ENTER = 6;

    byte BUTTON_CLICK = Menu::KEY_DOWN;
    byte BUTTON_DOUBLE_CLICK = Menu::KEY_LEFT;
    byte BUTTON_LONG_CLICK = Menu::KEY_RIGHT_ENTER;

    void (* renderTitle)(Menu::ITEM * item) = NULL;
    void (* renderItem)(Menu::ITEM * item, int activeUid, bool markActive, bool inlineValue) = NULL;
    void (* menuExit)() = NULL;
    void (* preRender)() = NULL;
    
    Menu();
    void init(Menu::ITEM * item);
    void reset();
    void addItem (Menu::ITEM * item, char hotkey, char * text, Menu::ITEM * parent, bool (* cb)(), bool (* value_cb)(bool x));
    void action(byte key);
    void actionHotkey (char hotkey);
    void show();

  private:

};

#endif
