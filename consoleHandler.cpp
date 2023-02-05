#include "consoleHandler.h"
consoleHandler::consoleHandler() {
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	this->width = size.ws_col;
	this->height = size.ws_row;

	//clock_gettime(CLOCK_REALTIME, &time_);

	clearScreen();
	fflush(stdout);
	return;
}

void consoleHandler::clearScreen() {
	printf("%c[0;7m", 0x1B); // set black text on white background
	printf("%c[0;0H", 0x1B); // cursor to top left corner
	for (int i = 0; i < this->height; i++) {
		for (int u = 0; u < this->width; u++) {
			printf(" ");
		}
	}
	printf("%c[0;0H", 0x1B); // return cursor to top left corner
	fflush(stdout);
}

bool consoleHandler::setCursorPos(int x, int y) {
	if (x > this->width || y > this->height) { return false; }
	printf("%c[%d;%dH", 0x1B, y, x);
	return true;
}

void consoleHandler::hideCursor() {
	this->setCursorPos(this->width, this->height);
}



textField::textField(int x_coord, int y_coord, int width, int height, int textColor, int bgColor, int borderEn, consoleHandler* con) {
	this->border = borderEn;
	this->x = x_coord;
	this->y = y_coord;
	this->fgColor = textColor;
	this->bgColor = bgColor;
	this->mainConsole = con;
	this->scrolling = false;
	this->invert = false;
	this->enabled = true;
	this->width = width;
	this->height = height;
	this->startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 256; i++) {
		this->theString[i] = '\0';
	}
	return;
}

bool textField::setText(char* s, int len) {
	if (len > MAX_TEXTFIELD_STRING_LENGTH)return false;
	this->textLength = len;
	for (int i = 0; i < len; i++) {
		this->theString[i] = s[i];
	}
	return true;
}

bool textField::setText(std::string s) {
	// @TODO
	return true;
}

void textField::draw() {
	//@TODO
	// need to work on scrolling text


	if (!this->enabled)return;
	this->mainConsole->setCursorPos(this->x, this->y);
	if (this->border == 1) {
		// top left corner
		printf(u8"\u2554");
		for (int i = 0; i < this->width; i++) {
			// crossbar
			printf(u8"\u2550");
		}
		//top right corner
		printf(u8"\u2557");
		this->mainConsole->setCursorPos(this->x, this->y + 1);
		// left edge
		printf(u8"\u2551");
		for (int i = 0; i < this->width; i++) {
			// crossbar
			if (this->theString[i] != '\0') printf("%c", this->theString[i]);
			else printf(" ");
		}
		// right edge
		printf(u8"\u2551");
		this->mainConsole->setCursorPos(this->x, this->y + 2);
		// top left corner
		printf(u8"\u255A");
		for (int i = 0; i < this->width; i++) {
			// crossbar
			printf(u8"\u2550");
		}
		//top right corner
		printf(u8"\u255D");
	}
	else  if (this->border == 0) {
		for (int i = 0; i < this->width; i++) {
			// crossbar
			if (this->theString[i] != '\0') printf("%c", this->theString[i]);
			else printf(" ");
		}
	}
	else if (this->border == 2) {
		auto newNow = std::chrono::high_resolution_clock::now();
		auto mod = std::chrono::milliseconds(1000);
		if (newNow - this->startTime > mod) {
			this->border = 3;
			this->startTime = std::chrono::high_resolution_clock::now();
			//@TODO
			printf("!");
		}
	}
	else if (this->border == 3) {
		auto newNow = std::chrono::high_resolution_clock::now();
		auto mod = std::chrono::milliseconds(1000);
		if (newNow - this->startTime > mod) {
			this->border = 2;
			this->startTime = std::chrono::high_resolution_clock::now();
			//@TODO
			printf("X");
		}
	}
	fflush(stdout);
}

void textField::toggleBorder() {
	if (border == 0) border = 1;
	if (border == 1) border = 0;
}