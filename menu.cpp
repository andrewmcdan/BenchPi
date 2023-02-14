#include "Menu.h"
#include "consoleHandler.h"

Menu::Menu(loopUpdateHandler* l, consoleHandler* con, std::string text) {
	this->menuText = text;
	this->visible = false;
	this->loop = l;
	this->isRootMenu = true;
	tField = textField(con->width / 4, 0, con->width / 2, con->height - 4, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, con, textField::center);
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
	this->visible = true;
	if (!this->tField.getEnabled()) {
		tField.setEnabled(true);
		tField.setText(this->menuText);
		this->tFieldDraw_loopID = this->loop->addEvent([&]() {
			this->tField.draw();
			return 1;
			});
		for (size_t itr = 0; itr < this->menuItems.size(); itr++) {
			if ((this->mainWindow->height - 4 - 3) > itr) {
				if (!this->menuItems.at(itr).tField.getEnabled()) {
					this->menuItems.at(itr).tField.setEnabled(true);
					this->menuItems.at(itr).tFieldDraw_loopID = this->loop->addEvent([&, itr]() {
						this->menuItems.at(itr).tField.draw();
						return 1;
						});
				}
			}

		}
	}
	
}

void Menu::disableMenu() {
	this->tField.setEnabled(false);
	this->loop->remove(this->tFieldDraw_loopID);
	this->visible = false;
	for (unsigned int itr = 0; itr < this->menuItems.size(); itr++) {
		this->menuItems.at(itr).tField.setEnabled(false);
		this->loop->remove(this->menuItems.at(itr).tFieldDraw_loopID);
	}
}

void Menu::addMenuItem(std::string text, std::function<void()> action, consoleHandler* con) {
	//MenuItem tempItem = MenuItem(text,action,this->mainWindow);
	this->menuItems.push_back(MenuItem(text, action, con));
}

void Menu::upKey(){}
void Menu::downKey(){}
void Menu::enterKey(){}


MenuItem::MenuItem(std::string text, std::function<void()> act, consoleHandler* con) {
	tField = textField((con->width / 4) + 1, 5, (con->width / 2) - 2, 1, COLOR_WHITE, COLOR_BLACK, BORDER_DISABLED, con, textField::center);
	tField.setText(text);
	tField.setEnabled(false);
	action = act;
	//tField.setTextColor(COLOR_WHITE,COLOR_BLACK);
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