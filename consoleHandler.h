#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <cstring>
#include <functional>
#include <ncurses.h>
#include <vector>
#include "loopHandler.fwd.h"

#define BORDER_ENABLED 1
#define BORDER_DISABLED 0
#define MAX_TEXTFIELD_STRING_LENGTH 32768
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
	void setTitle(std::string t);
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
	void changeHW(int width, int height);
	~textField();

private:
	std::chrono::system_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	int scrollSpeed, width, height, textLength, x, y, border, borderColor;
	short int fgColor, bgColor;
	textAlignment alignment;
	bool scrolling, invert, enabled, needDraw, clearOnPrint;
	char theString[MAX_TEXTFIELD_STRING_LENGTH];
	std::string title;
	consoleHandler* mainConsole;
};

class WindowManager {
public:
	enum windowType : unsigned int;
	enum dataSource : unsigned int;
	bool firstWindow = true;
	
private:
	struct window_s {
		textField tField;
		int x, y, width, height, priority;
		std::string title;
		dataSource source;
		windowType type;
		unsigned long id;
		unsigned long loopEventId;
		bool titleEnabled;
		bool destroyed;
	};
	unsigned long id_;
	consoleHandler* mainConsole;
	loopUpdateHandler* loopHandler;
	inputHandler* userInput;
	std::vector<std::vector<unsigned long>>fieldArray;
	long selectedWindowID = -1;
	long secondSelectedWindowID = -1;

public:
	WindowManager(consoleHandler* con, loopUpdateHandler* loop, inputHandler* input_h);
	bool createWindow(int x, int y, int width, int height, std::string title, int priority);
	bool destroyWindow(unsigned long id);
	void selectNextWindow();
	void selectPrevWindow();
	void enableWindowTitle(std::string title, unsigned long id);
	void disableWindowTitle(unsigned long id);
	void setWindowTitle(std::string title, unsigned long id);
	void splitWindowVert(unsigned int numDivs);
	void splitWindowHoriz(unsigned int numDivs);
	void mergeWindows(unsigned int id_windowToKeep, unsigned int id_windowToMerge);
	void setWindowType(unsigned long id);
	void update();
	void increaseWinodwPriority(unsigned long id);
	void decreaseWinodwPriority(unsigned long id);
	long getSelectedWindowIndex();
	long getSecondSelectedWindowIndex();
	void selectSecondWindow();
	void resetSelectSecondWindow();
	window_s* getSelectedWindow();
	window_s* getSecondSelectedWindow();
	std::vector<window_s> windows;
	enum windowType : unsigned int {
		NOT_SET, SERIAL_MON, MIDI_MON, KEYBOARD_INPUT, AMMETER, VOLTMETER, MULTIMETER,
	};
	enum dataSource : unsigned int {
		SOURCE_NOT_SET, SERIAL, MIDI, KEYBOARD_ASCII, KEYBOARD_MIDI, KEYBOARD_MIDI_BYTES
	};
};