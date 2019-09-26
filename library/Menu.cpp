/*
  Menu.cpp - Library for universal menu.
  Created by Petr Vavrin, 2019
  Released into the public domain.
*/

#include "Arduino.h"
#include "Menu.h"

Menu::Menu()
{
}

void Menu::init(Menu::ITEM * item) {
  this->mainItem = item;
  this->activeItem = item;
}

void Menu::reset() {
  this->activeItem = this->mainItem;
}

void Menu::addItem (Menu::ITEM * item, char hotkey, char * text, Menu::ITEM * parent, bool (* cb)(), bool (* value_cb)(bool x)) {
  Menu::ITEM * childItem;
  if (parent == NULL) {
    parent = &this->rootItem;
  }
  item->parent = parent;
  item->prev = NULL;
  item->next = NULL;
  item->child = NULL;
  item->text = text;
  item->cb = cb;
  item->value_cb = value_cb;
  item->uid = this->itemCounter;
  item->hotkey = hotkey;
  item->disabled = false;
  this->itemCounter += 1;
  if (parent->child == NULL) {
    parent->child = item;
  } else {
    childItem = parent->child;
    while (childItem->next) {
      childItem = childItem->next;
    }
    childItem->next = item;
    item->prev = childItem;
  }
}

void Menu::action(byte key) {
  if (this->circular && key == this->KEY_UP && this->activeItem->prev == NULL) {
    while (this->activeItem->next) {
      this->activeItem = this->activeItem->next;
    }

  } else if (this->circular && key == this->KEY_DOWN && this->activeItem->next == NULL) {
    while (this->activeItem->prev) {
      this->activeItem = this->activeItem->prev;
    }

  } else if (key == this->KEY_UP && this->activeItem->prev != NULL) {
    this->activeItem = this->activeItem->prev;

  } else if (key == this->KEY_DOWN && this->activeItem->next != NULL) {
    this->activeItem = this->activeItem->next;

  } else if (key == this->KEY_LEFT && this->activeItem->parent != NULL && this->activeItem->parent->text != NULL) {

    this->activeItem = this->activeItem->parent;

  } else if (key == this->KEY_LEFT && (this->activeItem->parent == NULL || this->activeItem->parent->text == NULL) && this->menuExit != NULL) {
      (this->menuExit)();
        return;
  } else if (this->activeItem->disabled == false) {
    if ((key == this->KEY_RIGHT || key == this->KEY_RIGHT_ENTER) && this->activeItem->child != NULL) {
      this->activeItem = this->activeItem->child;
    
    } else if ((key == this->KEY_ENTER || key == this->KEY_RIGHT_ENTER) && this->activeItem->cb != NULL) {
      (this->activeItem->cb)();
    
    }
  }
  
    this->show();
}

void Menu::actionHotkey (char hotkey) {
  Menu::ITEM * item = this->activeItem;
  while (item->prev) {
    item = item->prev;
  }
  do {
    if (item->hotkey == hotkey) {
      this->activeItem = item;
      this->action(this->KEY_RIGHT_ENTER);
      break;
    } else if (item->next != NULL){
      item = item->next;
    } else {
      break;
    }
  } while (true);
}

void Menu::show() {
  if (this->preRender != NULL) {
      (this->preRender)();
  }
  Menu::ITEM * showItem = this->activeItem;
  int itemsCounter = this->itemsLimit;
  bool iterate = true;
  if (this->showTitle) {
    if (this->renderTitle != NULL) {
      (this->renderTitle)(this->activeItem->parent);
    }
    itemsCounter -= 1;
  }
  if (this->showPreviousitems) { 
    while (showItem->prev) {
      showItem = showItem->prev;
    }
  }
  do {
    if (this->renderItem != NULL) {
      (this->renderItem)(showItem, this->activeItem->uid, this->markActive, this->inlineValue);
    }

    itemsCounter -= 1;
    if (itemsCounter > 0 && showItem->next != NULL){
      showItem = showItem->next;
    } else {
      iterate = false;
    }
  } while (iterate);
  if (this->showValue && this->activeItem->parent != NULL) {
    showItem = this->activeItem->parent;
    if (showItem->value_cb != NULL) {
      (showItem->value_cb)(false);
    }
  }
}