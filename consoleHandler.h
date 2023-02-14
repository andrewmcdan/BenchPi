#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <functional>
#include <ncurses.h>


#define MAX_TEXTFIELD_STRING_LENGTH 8192
class consoleHandler
{
public:
	consoleHandler();
	void clearScreen();
	bool setCursorPos(int, int);
	int colornum(int fg, int bg);
	int width, height; 
};

class textField {
public:
	enum textAlignment { left, center, right };
	textField();
	textField(int x_coord, int y_coord, int width, int height, short int textColor, short int bgColor, int borderEn, consoleHandler* con,textField::textAlignment align);
	bool setText(char*, int);
	bool setText(std::string);
	int  draw();
	void toggleBorder();
	bool toggleEnabled();
	void setEnabled(bool b);
	bool getEnabled();
	void setClearOnPrint(bool b);
	void setBorderColor(int color);
	void setTextColor(short int fg, short int bg);
	void setScroll(bool b);
	void shortenTheString(int l);
	void move(int x, int y);
	~textField();

private:
	std::chrono::system_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	int scrollSpeed, width, height, textLength, x, y, border, borderColor;
	short int fgColor, bgColor;
	textAlignment alignment;
	bool scrolling, invert, enabled, needDraw, clearOnPrint;
	char theString[MAX_TEXTFIELD_STRING_LENGTH];
	consoleHandler* mainConsole;
};