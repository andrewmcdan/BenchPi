#pragma once
#include <iostream>
#include <string>
#include <chrono>

#define MAX_TEXTFIELD_STRING_LENGTH 1024
class consoleHandler
{
public:
	consoleHandler();
	void clearScreen();
	bool setCursorPos(int, int);
	int width, height; 
};

class textField {
public:
	enum textAlignment { left, center, right };
	textField();
	textField(int x_coord, int y_coord, int width, int height, int textColor, int bgColor, int borderEn, consoleHandler* con,textField::textAlignment align);
	bool setText(char*, int);
	bool setText(std::string);
	int  draw();
	void toggleBorder();
	bool toggleEnabled();
	void setClearOnPrint(bool b);

private:
	std::chrono::system_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	int scrollSpeed, width, height, textLength, x, y, fgColor, bgColor, border;
	textAlignment alignment;
	bool scrolling, invert, enabled, needDraw, clearOnPrint;
	char theString[MAX_TEXTFIELD_STRING_LENGTH];
	consoleHandler* mainConsole;
};