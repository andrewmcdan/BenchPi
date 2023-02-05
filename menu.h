#pragma once
#include <functional>
#include "consoleHandler.h"
class menu
{
public:
	menu();
	bool visible;
};

class menuItem {
public:
	/*
	Selectable menu item. Has action associated with it.
	*/
	menuItem();
	// the action to be performed when selecting the menu item
	std::function<int(int, int)>action;
};

class shortcutItem {
public:
	shortcutItem(int pos, std::function<int()> f, consoleHandler* con, std::string text, textField::textAlignment align);
	textField tField;
	std::function<int()> func;
};