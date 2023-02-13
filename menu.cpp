#include "Menu.h"
#include "consoleHandler.h"

Menu::Menu(loopUpdateHandler* l, consoleHandler* con) {
	this->visible = false;
	this->loop = l;
	this->isRootMenu = true;
	tField = textField(con->width / 4, 0, con->width / 2, con->height - 3, COLOR_WHITE, COLOR_BLACK, TRUE, con, textField::center);
	tField.setEnabled(false);
	tField.setText(this->menuText);
	this->selectionPosition = 0;
	this->viewPosition = 0;
}

void Menu::setReferringMenu(Menu* m) {
	this->referringMenu = m;
	this->isRootMenu = false;
}

void Menu::enableMenu() {
	this->selectionPosition = 0;
	this->viewPosition = 0;
	tField.setEnabled(true);
}

void Menu::disableMenu() {

}

void Menu::upKey(){}
void Menu::downKey(){}
void Menu::enterKey(){}


MenuItem::MenuItem() {

}

shortcutItem::shortcutItem(int pos, std::function<int()> f,consoleHandler* con, std::string text, textField::textAlignment align) {
	this->tField = textField((con->width / 5) * pos - (con->width / 5) + 1, con->height - 2, con->width / 5 - 3, 1, 7, 0, 1, con, align);
	char* c = new char[text.length()];
	strcpy(c, text.c_str());
	this->tField.setText(c,text.length());
	this->inputListenerID = -1;
}

void shortcutItem::setInputListenerIdAndKey(int id, int key) {
	this->inputListenerID = id;
	this->key = key;
}