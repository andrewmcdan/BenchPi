#pragma once
#include <functional>
#include "consoleHandler.h"
#include "main.h"
class Menu
{
public:
	Menu(loopUpdateHandler* l, consoleHandler* con);
	void setReferringMenu(Menu* m);
	void enableMenu();
	void disableMenu();
	void upKey();
	void downKey();
	void enterKey();
	bool visible;
private:
	int selectionPosition;
	int viewPosition;
	textField tField;
	std::string menuText;
	Menu* referringMenu;
	bool isRootMenu;
	loopUpdateHandler* loop;
	consoleHandler* mainWindow;
};

class MenuItem {
public:
	/*
	Selectable menu item. Has action associated with it.
	*/
	MenuItem();
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