#include "consoleHandler.h"
#include "loopHandler.h"

consoleHandler::consoleHandler() {
	// get the height and width from the ncurses library
	this->width = stdscr->_maxx;
	this->height = stdscr->_maxy;
	return;
}

void consoleHandler::clearScreen() {
	clear();
}

bool consoleHandler::setCursorPos(int x, int y) {
	// bounds check the params
	if (x > this->width || y > this->height || x < 0 || y < 0) { return false; }
	// move the cursor to the location uses the ncurses method
	move(y, x);
	return true;
}


textField::textField(){
	// init all the variable within the object
	this->alignment = left;
	this->border = 0;
	this->borderColor = COLOR_WHITE;
	this->x = 0;
	this->y = 0;
	this->fgColor = COLOR_WHITE;
	this->bgColor = COLOR_BLACK;
	this->scrolling = false;
	this->invert = false;
	this->enabled = true;
	this->needDraw = true;
	this->clearOnPrint = true;
	this->width = 0;
	this->height = 0;
	this->startTime = std::chrono::high_resolution_clock::now();
	this->textLength = 0;
	// clear out theSting 
	for (int i = 0; i < MAX_TEXTFIELD_STRING_LENGTH; i++) {
		this->theString[i] = '\0';
	}
}
textField::textField(int x_coord, int y_coord, int width, int height, short int textColor, short int bgColor, int borderEn, consoleHandler* con, textField::textAlignment align) {
	this->alignment = align;
	this->borderColor = COLOR_WHITE;
	this->border = borderEn;
	this->x = x_coord;
	this->y = y_coord;
	this->fgColor = textColor;
	this->bgColor = bgColor;
	this->mainConsole = con;
	this->scrolling = false;
	this->invert = false;
	this->enabled = true;
	this->needDraw = true;
	this->clearOnPrint = true;
	this->width = width;
	this->height = height;
	this->startTime = std::chrono::high_resolution_clock::now();
	this->textLength = 0;
	for (int i = 0; i < MAX_TEXTFIELD_STRING_LENGTH; i++) {
		this->theString[i] = '\0';
	}
	return;
}

void textField::setClearOnPrint(bool b) {
	this->clearOnPrint = b;
}

void textField::setTitle(std::string t) {
	this->title = t;
}

bool textField::setText(char* s, int len) {
	// first check to make sure there is enough room in the buffer.
	// if not, shorten the string by calling...
	if (len + this->textLength > MAX_TEXTFIELD_STRING_LENGTH - 1) this->shortenTheString(len);
	// if clearOnPrint is true, clear out theString and then add the 
	// new data to it. Otherwise, append the new data to the end of it. 
	if (this->clearOnPrint) {
		for (int i = 0; i < MAX_TEXTFIELD_STRING_LENGTH; i++) {
			this->theString[i] = '\0';
		}
		this->textLength = len;
		for (int i = 0; i < len; i++) {
			this->theString[i] = s[i];
		}
	}
	else {
		int temp = this->textLength;
		this->textLength += len;
		for (int i = temp; i < this->textLength; i++) {
			this->theString[i] = s[i - temp];
		}
	}
	return true;
}

bool textField::setText(std::string s) {
	char* c = new char[s.length()+1];
	strcpy(c, s.c_str());
	this->setText(c, s.length());
	return true;
}

void textField::shortenTheString(int l) {
	for (int i = 0; i < this->textLength - l; i++) {
		this->theString[i] = this->theString[i + l];
	}
	this->textLength -= l;
}

int textField::draw() {
	// @TODO: This method needs a bit of reworking to make sure that text always prints pretty.

	// printPos keeeps track of where in theString we are printing from
	int printPos = 0;
	if (!this->enabled)return -1;
	// try to set the cursor to the start position and if that fails,
	// jsut return.
	if(!this->mainConsole->setCursorPos(this->x, this->y))return -1;
	
	// set the colors. colorNum() returns a number unique to the fg/bg combo.
	// COLOR_PAIR() is part of ncurses and returns an attribute for the attron() func.
	attron(COLOR_PAIR(this->mainConsole->colornum(this->fgColor, this->bgColor)));
	if (this->border == 1) {
		// the border may be a different color than the text, so change the color to border color.
		attroff(COLOR_PAIR(this->mainConsole->colornum(this->fgColor, this->bgColor)));
		attron(COLOR_PAIR(this->mainConsole->colornum(this->borderColor, this->bgColor)));
		// print top left corner
		printw("\u2554");
		// print top crossbar
		for (int i = 0; i < this->width; i++) {
			printw("\u2550");
		}
		// print top right corner
		printw("\u2557");
		// print left side bar
		for (int i = 0; i < this->height; i++) {
			this->mainConsole->setCursorPos(this->x, this->y + i + 1);
			printw("\u2551");
		}
		// print right side bar
		for (int i = 0; i < this->height; i++) {
			this->mainConsole->setCursorPos(this->x + this->width + 1, this->y + i + 1);
			printw("\u2551");
		}
		this->mainConsole->setCursorPos(this->x, this->y + this->height + 1);
		// print bottom left corner
		printw("\u255A");
		//print top crossbar
		for (int i = 0; i < this->width; i++) {
			printw("\u2550");
		}
		//print bottom right vorner
		printw("\u255D");
		if (this->title.length() > 0) {
			// print title crossbar
			this->mainConsole->setCursorPos(this->x, this->y + 2);
			printw("\u2560");
			this->mainConsole->setCursorPos(this->x + this->width + 1, this->y + 2);
			printw("\u2563");
			this->mainConsole->setCursorPos(this->x + 1, this->y + 2);
			for (int i = 0; i < this->width; i++) {
				printw("\u2550");
			}
		}
		// switch back to the text color
		attroff(COLOR_PAIR(this->mainConsole->colornum(this->borderColor, this->bgColor)));
		attron(COLOR_PAIR(this->mainConsole->colornum(this->fgColor, this->bgColor)));
		int titleOffset = 0;
		if (this->title.length() > 0) {
			// the position of the first letter of the titel if found by...
			// start at the x of the text area, add half the width of the area to get to the middle, then subtract half the length
			// of the string to back up so that the string is centered.
			this->mainConsole->setCursorPos(this->x + this->width / 2 - this->title.length() / 2, this->y + 1);
			printw(this->title.c_str());
			titleOffset = 2;
		}

		// clear the textField so that no remnants of past text show up
		for (int i = 0; i < this->height - titleOffset; i++) {
			this->mainConsole->setCursorPos(this->x + 1, this->y + 1 + i + titleOffset);
			for (int p = 0; p < this->width; p++) {
				printw(" ");
			}
		}
		// print text
		// @TODO:
		// need to rework this so that when the text is too long for the window, it can show the most recent text.
		switch (this->alignment) {
		case left:
		{
			// first thing to do is count the number of lines in the text to print
			int lineCount = 1;
			for (int i = 0; i < this->textLength; i++) {
				if (this->theString[i] == '\r') {
					lineCount++;
					// skip newline if it immeditely follows a return char
					if (this->theString[i + 1] == '\n') i += 2;
				}
				else if (this->theString[i] == '\n') {
					lineCount++;
					// skip return char if it immedaitely follows a return
					if (this->theString[i + 1] == '\r') i += 2;
				}
			}
			// skip into the string as far as needed to ensure that the most
			// recent lines are shown and older lines are not.
			int lineNo = 0;
			for (; lineCount - lineNo > this->height - titleOffset; printPos++) {
				if (this->theString[printPos] == '\r') {
					lineNo++;
					if (this->theString[printPos + 1] == '\n') printPos += 2;
				}
				else if (this->theString[printPos] == '\n') {
					lineNo++;
					if (this->theString[printPos + 1] == '\r') printPos += 2;
				}
			}
			// iterate through all the characters in the string print them.
			for (int i = 0; i < this->height - titleOffset; i++) {
				this->mainConsole->setCursorPos(this->x + 1, this->y + 1 + i + titleOffset);
				for (int p = 0; p < this->width; p++) {
					//int temp = i * this->width + p;
					if (theString[printPos] == '\0') break;
					else if (theString[printPos] == '\r' || theString[printPos] == '\n') {
						printPos++;
						goto cont1;
					}
					else printw("%c", theString[printPos]);
					printPos++;
				}
			cont1:;
			}
			break;
		}
		case center:
			for (int i = 0; i < this->height; i++) {
				this->mainConsole->setCursorPos(((this->textLength + 1 - this->width * (i + 1)) > 0) ? (this->x + 1) : (this->x + this->width / 2 + 1 - (this->textLength % this->width) / 2), this->y + 1 + i + titleOffset);
				for (int p = 0; p < this->width; p++) {
					//int temp = i * this->width + p;
					if (theString[printPos] == '\0') break;
					else if (theString[printPos] == '\r' || theString[printPos] == '\n') {
						printPos++;
						goto cont2;
					}
					else printw("%c", theString[printPos]);
					printPos++;
				}
			cont2:;
			}
			break;
		case right:
			break;
		default:
			break;
		}
	}

	else  if (this->border == 0) {
		//clear the text field 
		for (int i = 0; i < this->height; i++) {
			this->mainConsole->setCursorPos(this->x, this->y + i);
			for (int p = 0; p < this->width; p++) {
				printw(" ");
			}
		}
		int titleOffset = 0;
		if (this->title.length() > 0) {
			this->mainConsole->setCursorPos(this->x + this->width / 2, this->y);
			printw(this->title.c_str());
			titleOffset = 2;
		}
		switch(this->alignment) {
		case left:
		{
			int lineCount = 1;
			for (int i = 0; i < this->textLength; i++) {
				if (this->theString[i] == '\r') {
					lineCount++;
					if (this->theString[i + 1] == '\n') i += 2;
				}
				else if (this->theString[i] == '\n') {
					lineCount++;
					if (this->theString[i + 1] == '\r') i += 2;
				}
			}
			int lineNo = 0;
			for (; lineCount - lineNo > this->height - titleOffset; printPos++) {
				if (this->theString[printPos] == '\r') {
					lineNo++;
					if (this->theString[printPos + 1] == '\n') printPos += 2;
				}
				else if (this->theString[printPos] == '\n') {
					lineNo++;
					if (this->theString[printPos + 1] == '\r') printPos += 2;
				}
			}
			for (int i = 0; i < this->height - titleOffset; i++) {
				this->mainConsole->setCursorPos(this->x, this->y + i + titleOffset);
				for (int p = 0; p < this->width; p++) {
					//int temp = i * this->width + p;
					if (theString[printPos] == '\0') break;
					else if (theString[printPos] == '\r' || theString[printPos] == '\n') {
						printPos++;
						goto cont3;
					}
					else printw("%c", theString[printPos]);
					printPos++;
				}
			cont3:;
			}
			break;
		}
		case center:
			for (int i = 0; i < this->height - titleOffset; i++) {
				this->mainConsole->setCursorPos(((this->textLength + 1 - this->width * (i + 1)) > 0) ? (this->x + 1) : (this->x + this->width / 2 + 1 - (this->textLength % this->width) / 2), this->y + i + titleOffset);
				for (int p = 0; p < this->width; p++) {
					//int temp = i * this->width + p;
					if (theString[printPos] == '\0') break;
					else if (theString[printPos] == '\r' || theString[printPos] == '\n') {
						printPos++;
						goto cont4;
					}
					else printw("%c", theString[printPos]);
					printPos++;
				}
			cont4:;
			}
			break;
		case right:
			break;
		default:
			break;
		}
	}
	// @TODO:
	// the next section is for flashing border. NOT IMPLEMENTED YET.
	else if (this->border == 2) {
		auto newNow = std::chrono::high_resolution_clock::now();
		auto mod = std::chrono::milliseconds(1000);
		if (newNow - this->startTime > mod) {
			this->border = 3;
			this->startTime = std::chrono::high_resolution_clock::now();
			//@TODO
			printw("!");
		}
	}
	else if (this->border == 3) {
		auto newNow = std::chrono::high_resolution_clock::now();
		auto mod = std::chrono::milliseconds(1000);
		if (newNow - this->startTime > mod) {
			this->border = 2;
			this->startTime = std::chrono::high_resolution_clock::now();
			//@TODO
			printw("X");
		}
	}
	attroff(COLOR_PAIR(this->mainConsole->colornum(this->fgColor, this->bgColor)));
	return 1;
}

void textField::toggleBorder() {
	if (border == 0) border = 1;
	if (border == 1) border = 0;
}

bool textField::toggleEnabled() {
	this->enabled = !this->enabled;
	if (this->enabled)this->mainConsole->clearScreen();
	return this->enabled;
}

void textField::setEnabled(bool b) {
	this->enabled = b;
	if (this->enabled)this->mainConsole->clearScreen();
}

void textField::setScroll(bool b) {
	this->scrolling = b;
}

bool textField::getEnabled() {
	return this->enabled;
}

void textField::move(int x, int y) {
	// bounds check the x and y
	if (x > this->mainConsole->width - this->width || y > this->mainConsole->height - this->height) return;
	// if x or y is -1, no change
	this->x = (x == -1) ? this->x : x;
	this->y = (y == -1) ? this->y : y;
}

int consoleHandler::colornum(int fg, int bg)
{
	int B, bbb, ffff;
	B = 0;// 1 << 7;
	bbb = (7 & bg) << 3;
	ffff = 7 & fg;
	short r = (B | bbb | ffff);
	init_pair(r, fg, bg);
	return r;
}

void textField::setBorderColor(int color) {
	this->borderColor = color;
	return;
}

void textField::setTextColor(short int fg, short int bg) {
	this->fgColor = fg;
	this->bgColor = bg;
	return;
}

void textField::changeHW(int width, int height) {
	this->width = width;
	this->height = height;
}

textField::~textField(){}

WindowManager::WindowManager(consoleHandler* con, loopUpdateHandler* loop, inputHandler* input_h) {
	this->mainConsole = con;
	this->loopHandler = loop;
	this->userInput = input_h;
	this->id_ = 1;
	for (int x = 0; x < con->width - 1; x++) {
		std::vector<unsigned long> temp;
		for (int y = 0; y < con->height - 4; y++) {
			temp.push_back(0);
		}
		this->fieldArray.push_back(temp);
	}
	this->createWindow(0, 0, con->width - 1, con->height - 4, "", 0);
	this->userInput->addListener([this](int c, TIMEPOINT_T t) {
		this->selectNextWindow();
		return 1;
	}, KEY_F(12));
	this->userInput->addListener([this](int c, TIMEPOINT_T t) {
		this->selectPrevWindow();
		return 1;
	}, KEY_F(11));
	this->loopHandler->addEvent([&]() {this->update(); return 1; });
}

bool WindowManager::createWindow(int x, int y, int width, int height, std::string title, int priority) {
	for (int x_ = x; x_ < x + width; x_++) {
		for (int y_ = y; y_ < y + height; y_++) {
			this->fieldArray.at(x_).at(y_) = this->id_;
		}
	}
	struct window_s window;
	window.x = x;
	window.y = y;
	window.height = height;
	window.width = width;
	window.id = this->id_;
	window.type = windowType::NOT_SET;
	window.source = dataSource::SOURCE_NOT_SET;
	window.tField = textField(x, y, width, height, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, this->mainConsole, textField::left);
	window.tField.setTitle(title);
	window.loopEventId = 0;
	window.priority = priority;
	window.titleEnabled = false;
	this->windows.push_back(window);
	size_t t = this->windows.size() - 1;
	this->windows.at(t).loopEventId = this->loopHandler->addEvent([this,t]() {
		this->windows.at(t).tField.draw(); 
		return 1; 
	}, priority);
	this->id_++;
	return true; 
}

bool WindowManager::destroyWindow(unsigned long id) { 
	auto itr = this->windows.begin();
	for (size_t i = 0; i < this->windows.size() && itr != this->windows.end(); i++, itr++) {
		if (this->windows.at(i).id == id) {
			this->loopHandler->remove(this->windows.at(i).loopEventId);
			this->windows.erase(itr);
		}
	}
	return true; 
}

void WindowManager::selectNextWindow(){
	if (this->selectedWindowID == -1) {
		this->selectedWindowID = this->windows.at(0).id;
	}
	else {
		for (std::size_t i = 0; i < this->windows.size(); i++) {
			if ((this->selectedWindowID == this->windows.at(i).id) && (i < (this->windows.size() - 1))) {
				this->selectedWindowID = this->windows.at(i + 1).id;
				break;
			}
			else if ((this->selectedWindowID == this->windows.at(i).id) && (i == (this->windows.size() - 1))) {
				this->selectedWindowID = -1;
			}
		}
	}
}

void WindowManager::selectPrevWindow(){
	if (this->selectedWindowID == -1) {
		this->selectedWindowID = this->windows.at(this->windows.size() - 1).id;
	}
	else {
		for (size_t i = 0; i < this->windows.size(); i++) {
			if ((this->selectedWindowID == this->windows.at(i).id) && (i > 0)) {
				this->selectedWindowID = this->windows.at(i - 1).id;
			}
			else if ((this->selectedWindowID == this->windows.at(i).id) && (i == 0)) {
				this->selectedWindowID = -1;
			}
		}
	}
}

void WindowManager::enableWindowTitle(std::string title, unsigned long id){
	for (size_t i = 0; i < this->windows.size(); i++) {
		if (this->windows.at(i).id == id) {
			this->windows.at(i).tField.setTitle(title);
			this->windows.at(i).title = title;
			this->windows.at(i).titleEnabled = true;
		}
	}
}

void WindowManager::setWindowTitle(std::string title, unsigned long id) {
	this->enableWindowTitle(title, id);
}

void WindowManager::disableWindowTitle(unsigned long id) {
	for (size_t i = 0; i < this->windows.size(); i++) {
		if (this->windows.at(i).id == id) {
			this->windows.at(i).tField.setTitle("");
			this->windows.at(i).titleEnabled = false;
		}
	}
}

void WindowManager::splitWindowVert(unsigned int numDivs){
	int currentH = this->getSelectedWindow()->height;
	int currentW = this->getSelectedWindow()->width;
	int newH = currentH;
	int newW = (currentW / numDivs) - 2;
	if (newW < 5) return;
	this->getSelectedWindow()->tField.changeHW(newW, newH);
	this->getSelectedWindow()->height = newH;
	this->getSelectedWindow()->width = newW;
	int startX = this->getSelectedWindow()->x + newW + 2;
	int startY = this->getSelectedWindow()->y;

	for (int i = 0; i < (numDivs - 1); i++) {
		this->createWindow(startX + ((newW + 2)* i), startY, newW, newH, "", this->getSelectedWindow()->priority);
	}
}

void WindowManager::splitWindowHoriz(unsigned int numDivs){
	int currentH = this->getSelectedWindow()->height;
	int currentW = this->getSelectedWindow()->width;
	int newH = (currentH / numDivs) - 2;
	int newW = currentW;
	if (newH < 1)return;
	this->getSelectedWindow()->tField.changeHW(newW, newH);
	this->getSelectedWindow()->height = newH;
	this->getSelectedWindow()->width = newW;
	int startX = this->getSelectedWindow()->x;
	int startY = this->getSelectedWindow()->y + newH + 2;

	for (int i = 0; i < (numDivs - 1); i++) {
		this->createWindow(startX, startY + ((newH + 2) * i), newW, newH, "", this->getSelectedWindow()->priority);
	}
}

void WindowManager::setWindowType(unsigned long id){}

void WindowManager::update() {
	for (unsigned int i = 0; i < this->windows.size(); i++) {
		if (this->selectedWindowID == this->windows.at(i).id) {
			this->windows.at(i).tField.setBorderColor(COLOR_CYAN);
		}
		else {
			this->windows.at(i).tField.setBorderColor(COLOR_WHITE);
		}
	}
}

void WindowManager::increaseWinodwPriority(unsigned long id) {
	unsigned long eventId;
	unsigned long priority_t;
	for (size_t i = 0; i < this->windows.size(); i++) {
		if (this->windows.at(i).id = id) {
			eventId = this->windows.at(i).loopEventId; 
			priority_t = this->windows.at(i).priority;
			this->loopHandler->remove(eventId);
			this->windows.at(i).loopEventId = this->loopHandler->addEvent([&, i]() {
				this->windows.at(i).tField.draw();
				return 1;
			}, priority_t + 1);
			this->windows.at(i).priority++;
		}
	}
}

void WindowManager::decreaseWinodwPriority(unsigned long id) {
	unsigned long eventId;
	unsigned long priority_t;
	for (size_t i = 0; i < this->windows.size(); i++) {
		if (this->windows.at(i).id = id) {
			eventId = this->windows.at(i).loopEventId;
			priority_t = this->windows.at(i).priority;
			this->loopHandler->remove(eventId);
			this->windows.at(i).loopEventId = this->loopHandler->addEvent([&, i]() {
				this->windows.at(i).tField.draw();
				return 1;
			}, priority_t - 1);
			this->windows.at(i).priority--;
		}
	}
}

long WindowManager::getSelectedWindowIndex() {
	if (this->selectedWindowID == -1)return -1;
	for (unsigned int i = 0; i < this->windows.size(); i++) {
		if (this->windows.at(i).id == this->selectedWindowID) return i;
	}
}

WindowManager::window_s* WindowManager::getSelectedWindow() {
	if (this->getSelectedWindowIndex() == -1)return nullptr;
	return &this->windows.at(this->getSelectedWindowIndex());
}