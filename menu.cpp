#include "menu.h"
#include "consoleHandler.h"

menu::menu() {
	this->visible = false;
}


menuItem::menuItem() {

}

shortcutItem::shortcutItem(int pos, std::function<int()> f,consoleHandler* con, std::string text, textField::textAlignment align) {
	this->tField = textField((con->width / 4) * pos - (con->width / 4) + 1, con->height - 2, con->width / 4 - 4, 1, 0, 7, 1, con, align);
	this->tField.setText(text);
	this->func = f;
	//this->tField.draw();
	//this->tField.registerDraw();
	//this->tFielf.registerKeyscan("F1");
}