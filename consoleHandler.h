#pragma once
#include <iostream>
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#include <string>
#include <chrono>

#define MAX_TEXTFIELD_STRING_LENGTH 1024
class consoleHandler
{
public:
	consoleHandler();
	void clearScreen();
	bool setCursorPos(int, int);
	void hideCursor();
	int width, height; 
};

class textField {
public:
	textField(int, int, int, int, int, int, int, consoleHandler*);
	bool setText(char*, int);
	bool setText(std::string);
	void draw();
	void toggleBorder();
private:
	std::chrono::system_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	int scrollSpeed, width, height, textLength, x, y, fgColor, bgColor, border;
	bool scrolling, invert, enabled;
	char theString[MAX_TEXTFIELD_STRING_LENGTH];
	consoleHandler* mainConsole;
};