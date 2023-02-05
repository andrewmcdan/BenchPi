#include "consoleHandler.h"
#include <cstring>
#include <functional>
#include <ncurses.h>


consoleHandler::consoleHandler() {
	
	this->width = stdscr->_maxx;
	this->height = stdscr->_maxy;

	//clock_gettime(CLOCK_REALTIME, &time_);
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
}

bool consoleHandler::setCursorPos(int x, int y) {
	if (x > this->width || y > this->height) { return false; }
	printf("%c[%d;%dH", 0x1B, y, x);
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
	this->width = width;
	this->height = height;
	this->startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 256; i++) {
		this->theString[i] = '\0';
	}
}
textField::textField(int x_coord, int y_coord, int width, int height, int textColor, int bgColor, int borderEn, consoleHandler* con, textField::textAlignment align) {
	this->alignment = align;
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
	if (s.length() > MAX_TEXTFIELD_STRING_LENGTH)return false;
	char* c = new char[s.length()];
	strcpy(c, s.c_str());
	this->textLength = s.length();
	for (int i = 0; i < s.length(); i++) {
		this->theString[i] = c[i];
	}
	return true;
}

int textField::draw() {
	//@TODO
	// need to work on scrolling text


	if (!this->enabled)return -1;
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
		if (this->alignment == 1) {
			for (int i = 0; i < (this->width - this->textLength) / 2; i++) {
				printf(" ");
			}
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printf("%c", this->theString[i]);
			}
			for (int i = 0; i < this->width - (this->width - this->textLength) / 2 - this->textLength; i++) {
				printf(" ");
			}
		}
		else if (this->alignment == 2) {
			for (int i = 0; i < this->width - this->textLength; i++) {
				printf(" ");
			}
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printf("%c", this->theString[i]);
			}
		}
		else {
			for (int i = 0; i < this->width; i++) {
				if (this->theString[i] != '\0') printf("%c", this->theString[i]);
				else printf(" ");
			}
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
	return 1;
}

void textField::toggleBorder() {
	if (border == 0) border = 1;
	if (border == 1) border = 0;
}