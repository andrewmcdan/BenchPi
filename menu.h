#pragma once
#include <functional>
#include "consoleHandler.h"
#include "loopHandler.h"

class MenuItem {
public:
	/*
	Selectable menu item. Has action associated with it.
	*/
	MenuItem(std::string text, std::function<void()> action, consoleHandler* con);
	
	textField tField;
	unsigned int tFieldDraw_loopID;
	// the action to be performed when selecting the menu item
	std::function<void()>action;
	//std::string itemText;
	bool selected;
};

class Menu
{
public:
	Menu(loopUpdateHandler* l, consoleHandler* con, inputHandler* inputHandler_, std::string text);
	void enableMenu();
	void disableMenu();
	void upKey();
	void downKey();
	void enterKey();
	void escKey();
	void setEscKey(std::function<void()>f);
	void addMenuItem(std::string text, std::function<void()> action, consoleHandler* con);
	void resetMenuItemList();
	std::vector<MenuItem>menuItems;
	consoleHandler* mainWindow;
	inputHandler* userInputHandler;
	int selectionPosition;
	
private:
	void enableMenuItems();
	bool visible;
	int viewPosition;
	unsigned long tFieldDraw_loopID;
	textField tField;
	std::string menuText;
	loopUpdateHandler* loop;
	std::function<void()>escapeKeyFunc;
};


class shortcutItem {
public:
	shortcutItem(int pos, consoleHandler* con, std::string text, textField::textAlignment align);
	void setInputListenerIdAndKey(int id, int key);
	textField tField;
	int key;
	int inputListenerID;
private:
	
};