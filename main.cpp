#include <ncurses.h>
#include <time.h>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
//#include <sys/poll.h>
//#include <termios.h>
#include "menu.h"
#include "consoleHandler.h"
#include "main.h"


int main() {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	raw();

	//printf("%c[?25l", 0x1b); // turn the cursor off
	printw("test");

	// setup polling file descriptor for stdin
	//struct pollfd fds;
	//int ret;
	//fds.fd = 0; /* this is STDIN */
	//fds.events = POLLIN;
	
	// setup key capture on the console
	//struct termios term;
	//tcgetattr(1, &term);
	//term.c_lflag &= (~ICANON & ~ECHO);
	//tcsetattr(1, TCSANOW, &term);

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

	

	/*
	* Setup items:
	* 1. get list of all serial ports and query them to see what sort of device is attached.
	* 2. for all ammeters attached, set them up along with ammeter objects
	* 3. build menus
	* 
	*/

	//mainWindow.setCursorPos(5, 20);
	//printf("5,20");
	//mainWindow.setCursorPos(20, 5);
	//printf("20,5 ");
	//printf(u8"\u2550");
	//printf(u8"\u2551");
	//printf(u8"\u2552");
	//printf(u8"\u2553");
	//mainWindow.clearScreen();

	textField testTextField(1, 1, 30, 1, 0, 7, 0, &mainWindow, textField::textAlignment::left);
	std::string s = "test Text abcdefghijklmnopqrsdtuvwxyz";
	char* char_array = new char[s.length() + 1];
	strcpy(char_array, s.c_str());
	testTextField.setText(char_array, s.length());
	testTextField.draw();

	loop.addEvent([&testTextField]() {
		return testTextField.draw();
		});



	bool run = true;
	while (run) {
		/*
		* Stuff that needs to go in main loop
		* 1. call draw methods of everything on screen
		* 2. monitor open serial ports for new data
		* 3. monitor stdin for user input
		* 4. call update on methods that register a loop update method
		*/

		//mainWindow.clearScreen(); // reset the screen for drawing 
		loop.handleAll(); // handles all the loop events that hanve been registered

		// check for input on stdin
		//ret = poll(&fds, 1, 0);
		if (1 == 1) {
			char a, b, c;
			a = getch();
			// first check for ESC, ESC, ESC in order to exit program
			if (a == 27) {
				std::cin >> b;
				std::cin >> c;
				if (b == 27 and c == 27) run = false;
			}
			// Next print escaped sequences, arrows, etc.
			if (a < 32) {
				std::string s = "a: ";
				s += std::to_string((int)a);
				s += " b: ";
				s += std::to_string((int)b);
				s += " c: ";
				s += std::to_string((int)c);
				testTextField.setText(s);
			}
			// print strings
			else {
				std::string s;
				std::string t = "Entered: ";
				t += a;
				std::getline(std::cin, s);
				testTextField.setText(t + s);
			}
		}

		refresh();
		// sleep for ~1ms so that the CPU isn't being hammered all the time.
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	
	//menuItem testMenuItem;
	//testMenuItem.action = [](int a, int b) {
	//	return a;
	//};

	//int x = testMenuItem.action(1, 2);

	// Move cursor to bottom right corner and print...
	printf("%c[%d;%df  width: %d  height: %d", 0x1B, mainWindow.height, mainWindow.width - 23, mainWindow.width, mainWindow.height);
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	//printf("%c[37;40m", 0x1B);
	//printf("%c[2J", 0x1B);
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