#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include "menu.h"
#include "consoleHandler.h"
#include "main.h"


int main() {
	setlocale(LC_ALL, "en_US.UTF-8");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	timeout(0);
	raw();

	consoleHandler mainWindow = consoleHandler();
	loopUpdateHandler loop = loopUpdateHandler();
	
	int i = loop.addEvent([]() {return 654; });

	shortcutItem serialMonitorF1 = shortcutItem(1, []() {return 1; }, &mainWindow, "F1 - Exit Serial", textField::textAlignment::center);
	shortcutItem serialMonitorF2 = shortcutItem(2, []() {return 1; }, &mainWindow, "F2 - Something", textField::textAlignment::center);
	shortcutItem serialMonitorF3 = shortcutItem(3, []() {return 1; }, &mainWindow, "F3 - Something", textField::textAlignment::center);
	shortcutItem serialMonitorF4 = shortcutItem(4, []() {return 1; }, &mainWindow, "F4 - Something", textField::textAlignment::center);
	loop.addEvent([&serialMonitorF1]() {
		return serialMonitorF1.tField.draw();
		});
	loop.addEvent([&serialMonitorF2]() {
		return serialMonitorF2.tField.draw();
		});
	loop.addEvent([&serialMonitorF3]() {
		return serialMonitorF3.tField.draw();
		});
	loop.addEvent([&serialMonitorF4]() {
		return serialMonitorF4.tField.draw();
		});

	WINDOW* aWin = newwin(3, 30, 5, 10);
	box(aWin, 1, 1);
	wprintw(aWin, "test");
	wrefresh(aWin);
	refresh();

	/*
	* Setup items:
	* 1. get list of all serial ports and query them to see what sort of device is attached.
	* 2. for all ammeters attached, set them up along with ammeter objects
	* 3. build menus
	* 
	*/

	textField testTextField(1, 10, 30, 1, 0, 7, 0, &mainWindow, textField::textAlignment::left);
	std::string s = "test Text abcdefghijklmnopqrsdtuvwxyz";
	char* char_array = new char[s.length() + 1];
	strcpy(char_array, s.c_str());
	testTextField.setText(char_array, s.length());
	testTextField.draw();

	loop.addEvent([&testTextField]() {
		return testTextField.draw();
		});

	int a, b, c;
	bool run = true;
	while (run) {
		/*
		* Stuff that needs to go in main loop
		* 1. call draw methods of everything on screen
		* 2. monitor open serial ports for new data
		* 3. monitor stdin for user input
		* 4. call update on methods that register a loop update method
		*/

		loop.handleAll(); // handles all the loop events that hanve been registered
		wprintw(aWin, "t");
		int temp = getch();
		if(temp>0) a = temp;
		
		std::string s = std::to_string(a);
		printw(s.c_str());
		// first check for ESC, ESC, ESC in order to exit program
		if (a == 27) {
			b = getch();
			c = getch();
			if (b == 27 and c == 27) run = false;
		}
		// Next print escaped sequences, arrows, etc.
		if (a < 32) {
			b = getch();
			c = getch();
			std::string s = "a: ";
			s += std::to_string((int)a);
			s += " b: ";
			s += std::to_string((int)b);
			s += " c: ";
			s += std::to_string((int)c);
			testTextField.setText(s);
		}

		// last thing to do in the loop is puch the buffer to the dispaly.
		refresh();
		wrefresh(aWin);
		// sleep for ~1ms so that the CPU isn't being hammered all the time.
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	endwin();
	return 0;
}


loopUpdateHandler::loopUpdateHandler() {
	id = 0;
}

int loopUpdateHandler::addEvent(std::function<int()> f) {
	this->funcs.push_back(f);
	int t = this->id;
	this->id++;
	return t;
}

int loopUpdateHandler::remove(int id) {
	this->funcs.erase(funcs.begin() + id);
	this->id--;
	return this->funcs.size();
}

void loopUpdateHandler::handleAll() {
	for (int i = 0; i < this->funcs.size(); i++) {
		this->funcs[i]();
	}
}

int loopUpdateHandler::call(int id) {
	if (id >= this->id) return 0;
	return this->funcs[id]();
}