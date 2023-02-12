#include "menu.h"
#include "consoleHandler.h"

menu::menu() {
	this->visible = false;
}


menuItem::menuItem() {

}

shortcutItem::shortcutItem(int pos, std::function<int()> f,consoleHandler* con, std::string text, textField::textAlignment align) {
	this->tField = textField((con->width / 4) * pos - (con->width / 4) + 1, con->height - 2, con->width / 4 - 4, 1, 7, 0, 1, con, align);
	char* c = new char[text.length()];
	strcpy(c, text.c_str());
	this->tField.setText(c,text.length());
	this->inputListenerID = -1;
}

void shortcutItem::setInputListenerIdAndKey(int id, int key) {
	this->inputListenerID = id;
	this->key = key;
}