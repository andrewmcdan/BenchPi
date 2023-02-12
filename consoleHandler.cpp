#include "consoleHandler.h"


consoleHandler::consoleHandler() {
	this->width = stdscr->_maxx;
	this->height = stdscr->_maxy;
	return;
}

void consoleHandler::clearScreen() {
	clear();
}

bool consoleHandler::setCursorPos(int x, int y) {
	if (x > this->width || y > this->height) { return false; }
	move(y, x);
	return true;
}



textField::textField(){
	this->border = 0;
	this->x = 0;
	this->y = 0;
	this->fgColor = 7;
	this->bgColor = 0;
	this->scrolling = false;
	this->invert = false;
	this->enabled = true;
	this->needDraw = true;
	this->width = 0;
	this->height = 0;
	this->startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < MAX_TEXTFIELD_STRING_LENGTH; i++) {
		this->theString[i] = '\0';
	}
}
textField::textField(int x_coord, int y_coord, int width, int height, short int textColor, short int bgColor, int borderEn, consoleHandler* con, textField::textAlignment align) {
	this->alignment = align;
	this->border = borderEn;
	this->x = x_coord;
	this->y = y_coord;
	this->fgColor = textColor;
	this->bgColor = bgColor;
	this->mainConsole = con;
	init_pair(this->mainConsole->colornum(this->fgColor, this->bgColor), this->fgColor, this->bgColor);
	this->scrolling = false;
	this->invert = false;
	this->enabled = true;
	this->needDraw = true;
	this->clearOnPrint = true;
	this->width = width;
	this->height = height;
	this->startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < MAX_TEXTFIELD_STRING_LENGTH; i++) {
		this->theString[i] = '\0';
	}
	return;
}

void textField::setClearOnPrint(bool b) {
	this->clearOnPrint = b;
}

bool textField::setText(char* s, int len) {
	if (len + this->textLength > MAX_TEXTFIELD_STRING_LENGTH - 1)return false;
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
	if (s.length() + this->textLength > MAX_TEXTFIELD_STRING_LENGTH - 1)return false;
	char* c = new char[s.length()];
	strcpy(c, s.c_str());
	this->setText(c, s.length());
	return true;
}

int textField::draw() {
	if (!this->enabled)return -1;
	this->mainConsole->setCursorPos(this->x, this->y);
	
	attron(COLOR_PAIR(this->mainConsole->colornum(this->fgColor, this->bgColor)));
	if (this->border == 1) {
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

		// clear the textField so that no remnants of past text show up
		for (int i = 0; i < this->height; i++) {
			this->mainConsole->setCursorPos(this->x + 1, this->y + 1 + i);
			for (int p = 0; p < this->width; p++) {
				printw(" ");
			}
		}

		// print text
		switch (this->alignment) {
		case left:
			for (int i = 0; i < this->height; i++) {
				this->mainConsole->setCursorPos(this->x + 1, this->y + 1 + i);
				for (int p = 0; p < this->width; p++) {
					int temp = i * this->width + p;
					if (theString[temp] == '\0') break;
					else printw("%c",theString[temp]);
				}
			}
			break;
		case center:
			for (int i = 0; i < this->height; i++) {
				this->mainConsole->setCursorPos(((this->textLength + 1 - this->width * (i + 1)) > 0) ? (this->x + 1) : (this->x + this->width / 2 + 1 - (this->textLength % this->width) / 2), this->y + 1 + i);
				for (int p = 0; p < this->width; p++) {
					int temp = i * this->width + p;
					if (theString[temp] == '\0') break;
					else printw("%c", theString[temp]);
				}
			}
			break;
		case right:
			break;
		default:
			break;
		}
	}

	else  if (this->border == 0) {
		if (this->alignment == 1) {
			for (int i = 0; i < (this->width - this->textLength) / 2; i++) {
				printw(" ");
			}
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printw("%c", this->theString[i]);
			}
			for (int i = 0; i < this->width - (this->width - this->textLength) / 2 - this->textLength; i++) {
				printw(" ");
			}
		}
		else if (this->alignment == 2) {
			for (int i = 0; i < this->width - this->textLength; i++) {
				printw(" ");
			}
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printw("%c", this->theString[i]);
			}
		}
		else {
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printw("%c", this->theString[i]);
				else printw(" ");
			}
		}
	}
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
	return this->enabled;
}

void textField::setEnabled(bool b) {
	this->enabled = b;
}

bool textField::getEnabled() {
	return this->enabled;
}

int consoleHandler::colornum(int fg, int bg)
{
	int B, bbb, ffff;

	B = 0;// 1 << 7;
	bbb = (7 & bg) << 4;
	ffff = 7 & fg;

	return (B | bbb | ffff);
}

textField::~textField(){}