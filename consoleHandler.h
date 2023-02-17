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
	/// <summary>
	/// Handles everything related to the screen and the ncurses library
	/// </summary>
	consoleHandler();
	/// <summary>
	/// /Just clears the screen.
	/// </summary>
	void clearScreen();
	/// <summary>
	/// Move the cursor the specifide lcoation. X, Y are indexed at 0 and start at top left corner.
	/// </summary>
	/// <param name="x">X Coordinate</param>
	/// <param name="y">Y Coordinate</param>
	/// <returns></returns>
	bool setCursorPos(int x, int y);
	/// <summary>
	/// Returns a number unique to the pair of colors passed in
	/// </summary>
	/// <param name="fg">Foreground color</param>
	/// <param name="bg">Background color</param>
	/// <returns></returns>
	int colornum(int fg, int bg);
	int width, height; 
};

class textField {
public:
	enum textAlignment { left, center, right };
	textField();
	/// <summary>
	/// A text area that can receive data from any source
	/// </summary>
	/// <param name="x_coord"> - Starting x xoordinate</param>
	/// <param name="y_coord"> - Starting y coordinate</param>
	/// <param name="width"> - overrall width not including border</param>
	/// <param name="height"> - overrall hiehgt not including border</param>
	/// <param name="textColor"> - foreground color</param>
	/// <param name="bgColor"> - background color</param>
	/// <param name="borderEn"> - border enable disable</param>
	/// <param name="con"> - consolehandler object reference</param>
	/// <param name="align"> - text alignemtn within area: left, center, or right</param>
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

class WindowManager {
public:
	WindowManager();
};