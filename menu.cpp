#include "Menu.h"
#include "consoleHandler.h"

Menu::Menu(loopUpdateHandler* l, consoleHandler* con, inputHandler* inputHandler_, std::string text) {
	this->menuText = text;
	this->visible = false;
	this->loop = l;
	this->isRootMenu = true;
	tField = textField(con->width / 4, 0, con->width / 2, con->height - 4, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, con, textField::center);
	tField.setEnabled(false);
	tField.setText(this->menuText);
	this->selectionPosition = 0;
	this->viewPosition = 0;
	this->mainWindow = con;
	this->userInputHandler = inputHandler_;
}

void Menu::setReferringMenu(Menu* m) {
	this->referringMenu = m;
	this->isRootMenu = false;
}

void Menu::enableMenu() {
	//if (rootListenerID == -1)this->rootListenerID = this->referringMenu->rootListenerID;
	//else this->rootListenerID = rootListenerID;
	//this->rootListenerID = rootListenerID;
	this->selectionPosition = 0;
	this->viewPosition = 0;
	this->visible = true;
	this->tField.setText("test");
	if (!this->tField.getEnabled()) {
		this->tField.setEnabled(true);
		this->tField.setText(this->menuText);
		this->tFieldDraw_loopID = this->loop->addEvent([&]() {
			this->tField.draw();
			return 1;
			});
		this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
			this->downKey();
			return 1;
			}, KEY_DOWN);
		this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
			this->upKey();
			return 1;
			}, KEY_UP);
		this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
			this->enterKey();
			return 1;
			}, KEY_ENTER);
		this->enableMenuItems();
	}

	if (this->isRootMenu) {
		/*this->userInputHandler->addListener([&, rootListenerID](int c2, TIMEPOINT_T time2) {
			this->userInputHandler->remove(rootListenerID);
		this->userInputHandler->removeByKey(KEY_UP);
		this->userInputHandler->removeByKey(KEY_DOWN);
		this->disableMenu();
		this->mainWindow->clearScreen();
		return 1;
			}, KEY_ESC);*/
	}
	else {
		//this->userInputHandler->addListener([&,rootListenerID](int c2, TIMEPOINT_T time2) {
		//	//this->disableMenu();
		//	this->referringMenu->enableMenu(rootListenerID);
		//	this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
		//		this->referringMenu->upKey();
		//		return 1;
		//		}, KEY_UP);
		//	this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
		//		this->referringMenu->downKey();
		//		return 1;
		//		}, KEY_DOWN);
		//	this->userInputHandler->addListener([&](int c, TIMEPOINT_T t) {
		//		this->referringMenu->enterKey();
		//		return 1;
		//		}, KEY_ENTER);
		//	this->mainWindow->clearScreen();
		//	return 1;
		//	}, KEY_ESC);
	}
}

void Menu::enableMenuItems(){
	int firstItemIndex = this->viewPosition;
	int lastItemIndex = this->viewPosition + this->mainWindow->height - 8;

	for (size_t itr = 0; itr < this->menuItems.size(); itr++) {
		if (firstItemIndex <= itr && lastItemIndex >= itr) {
			if (!this->menuItems.at(itr).tField.getEnabled()) {
				this->menuItems.at(itr).tField.setEnabled(true);
				this->menuItems.at(itr).tFieldDraw_loopID = this->loop->addEvent([&, itr]() {
					this->menuItems.at(itr).tField.draw();
				return 1;
					});
			}
		}
		else {
			this->menuItems.at(itr).tField.setEnabled(false);
			this->loop->remove(this->menuItems.at(itr).tFieldDraw_loopID);
		}

		this->menuItems.at(itr).tField.move(-1, 4 + itr - this->viewPosition);

		if (this->selectionPosition == itr) {
			this->menuItems.at(itr).selected = true;
			this->menuItems.at(itr).tField.setTextColor(COLOR_BLACK, COLOR_WHITE);
		}
		else {
			this->menuItems.at(itr).selected = false;
			this->menuItems.at(itr).tField.setTextColor(COLOR_WHITE, COLOR_BLACK);
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
	if (!this->isRootMenu)this->referringMenu->enableMenu();
}


void Menu::addMenuItem(std::string text, std::function<void()> action, consoleHandler* con) {
	this->menuItems.push_back(MenuItem(text, action, con));
}

void Menu::upKey(){
	if (this->selectionPosition > 0) {
		this->selectionPosition--;
		if (this->viewPosition > 0)this->viewPosition--;
	}
	this->enableMenuItems();
}

void Menu::downKey(){
	if (this->selectionPosition < this->menuItems.size() - 1) {
		this->selectionPosition++;
		if (this->selectionPosition > this->mainWindow->height - 8)this->viewPosition++;
	}
	this->enableMenuItems();
}

void Menu::enterKey(){
	this->menuItems.at(this->selectionPosition).action();
}

void Menu::escKey() {
	this->escapeKeyFunc();
}
void Menu::setEscKey(std::function<void()>f) {
	this->escapeKeyFunc = f;
}


MenuItem::MenuItem(std::string text, std::function<void()> act, consoleHandler* con) {
	tField = textField((con->width / 4) + 1, 4, (con->width / 2) - 1, 1, COLOR_WHITE, COLOR_BLACK, BORDER_DISABLED, con, textField::center);
	tField.setText(text);
	tField.setEnabled(false);
	action = act;
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