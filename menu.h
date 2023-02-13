#pragma once
#include <functional>
#include "consoleHandler.h"
#include "main.h"
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
private:
	std::function<void()>action;
	std::string itemText;

};

class shortcutItem {
public:
	shortcutItem(int pos, std::function<int()> f, consoleHandler* con, std::string text, textField::textAlignment align);
	void setInputListenerIdAndKey(int id, int key);
	textField tField;
	int key;
private:
	int inputListenerID;
};