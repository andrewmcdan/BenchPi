#pragma once
#include <functional>
#include "consoleHandler.h"
#include "main.h"

class MenuItem {
public:
	/*
	Selectable menu item. Has action associated with it.
	*/
	MenuItem(std::string text, std::function<void()> action, consoleHandler* con);
	// the action to be performed when selecting the menu item
	textField tField;
	int tFieldDraw_loopID;
	std::function<void()>action;
	//std::string itemText;
	bool selected;
};

class Menu
{
public:
	Menu(loopUpdateHandler* l, consoleHandler* con, inputHandler* inputHandler_, std::string text);
	void setReferringMenu(Menu* m);
	void enableMenu();
	void disableMenu();
	void upKey();
	void downKey();
	void enterKey();
	void escKey();
	void setEscKey(std::function<void()>f);
	void addMenuItem(std::string text, std::function<void()> action, consoleHandler* con);
	std::vector<MenuItem>menuItems;
	consoleHandler* mainWindow;
	Menu* referringMenu;
	inputHandler* userInputHandler;
	int selectionPosition;
	
private:
	void enableMenuItems();
	bool visible;
	int viewPosition;
	int tFieldDraw_loopID;
	textField tField;
	std::string menuText;
	loopUpdateHandler* loop;
	bool isRootMenu;
	int rootListenerID;
	std::function<void()>escapeKeyFunc;
};


class shortcutItem {
public:
	shortcutItem(int pos, std::function<int()> f, consoleHandler* con, std::string text, textField::textAlignment align);
	void setInputListenerIdAndKey(int id, int key);
	textField tField;
	int key;
	int inputListenerID;
private:
	
};