#define _XOPEN_SOURCE_EXTENDED
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
#include <algorithm>
//#include <iterator>

bool run = true;

int main() {
	setlocale(LC_ALL, "en_US.utf8");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	timeout(0);
	raw();
	refresh();

	consoleHandler mainWindow = consoleHandler();
	loopUpdateHandler loop = loopUpdateHandler();
	inputHandler userInput = inputHandler(&loop);
	

	shortcutItem serialMonitorF1 = shortcutItem(1, []() {return 1; }, &mainWindow, "F1 - Exit Serial", textField::textAlignment::center);
	shortcutItem serialMonitorF2 = shortcutItem(2, []() {return 1; }, &mainWindow, "F2 - Something", textField::textAlignment::center);
	shortcutItem serialMonitorF3 = shortcutItem(3, []() {return 1; }, &mainWindow, "F3 - Something", textField::textAlignment::center);
	shortcutItem serialMonitorF4 = shortcutItem(4, []() {return 1; }, &mainWindow, "F4 - Quit", textField::textAlignment::center);
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
	

	/*
	* Setup items:
	* 1. get list of all serial ports and query them to see what sort of device is attached.
	* 2. for all ammeters attached, set them up along with ammeter objects
	* 3. build menus
	* 
	*/

	textField testTextField(1, 10, 30, 1, 0, 7, 1, &mainWindow, textField::textAlignment::left);
	std::string s = "test Text abcdefghijklmnopqrsdtuvwxyz";
	char* char_array = new char[s.length() + 1];
	strcpy(char_array, s.c_str());
	testTextField.setText(char_array, s.length());
	testTextField.draw();

	loop.addEvent([&testTextField]() {
		return testTextField.draw();
		});

	int a, b, c;
	
	while (run) {
		/*
		* Stuff that needs to go in main loop
		* 1. call draw methods of everything on screen
		* 2. monitor open serial ports for new data
		* 3. monitor stdin for user input
		* 4. call update on methods that register a loop update method
		*/

		loop.handleAll(); // handles all the loop events that hanve been registered
		//wprintw(aWin, "t");
		//int temp = getch();
		//if(temp>0) a = temp;
		
		//std::string s = std::to_string(a);
		//printw(s.c_str());
		// first check for ESC, ESC, ESC in order to exit program
		/*if (a == 27) {
			b = getch();
			c = getch();
			if (b == 27 and c == 27) run = false;
		}*/
		// Next print escaped sequences, arrows, etc.
		/*if (a < 32) {
			b = getch();
			c = getch();
			std::string s = "a: ";
			s += std::to_string((int)a);
			s += " b: ";
			s += std::to_string((int)b);
			s += " c: ";
			s += std::to_string((int)c);
			testTextField.setText(s);
		}*/

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
	this->id++;
	this->id_s.push_back(this->id);
	return this->id;
}

int loopUpdateHandler::remove(int id) {
	// get posistion of "id" in id_s vector
	auto itr = std::find(this->id_s.begin(), this->id_s.end(), id);
	this->funcs.erase(this->funcs.begin() + std::distance(this->id_s.begin(), itr));
	this->id_s.erase(itr);
	return this->funcs.size();
}

void loopUpdateHandler::handleAll() {
	for (unsigned int i = 0; i < this->funcs.size(); i++) {
		this->funcs[i]();
	}
}

int loopUpdateHandler::call(int id) {
	if(this->id_s.end() == std::find(this->id_s.begin(),this->id_s.end(),id))return -1;
	auto itr = std::find(this->id_s.begin(), this->id_s.end(), id);
	return this->funcs[std::distance(this->id_s.begin(),itr)]();
}

inputHandler::inputHandler(loopUpdateHandler* loop){
	this->loopEventId = loop->addEvent([this](){
		this->handleInput();
		return 0;
		});

	this->id_index = 0;
	this->printToScreenEn = false;
}

int inputHandler::addListener(std::function<int(int,int)> f, int key){
	this->funcs.push_back(f);
	events_struct t = {key,this->id_index++};
	this->events.push_back(t);
	return this->events.size();
}

int inputHandler::remove(unsigned long id){
	if (id > this->events.size())return -1;
	for(unsigned int i = 0; i < this->events.size(); i++){
		if(this->events[i].id == id) {
			this->events.erase(this->events.begin() + i);
			this->funcs.erase(this->funcs.begin() + i);
			return this->events.size();
		}
	}
	return -1;
}

int inputHandler::call(unsigned long id, int a, int b){
	for(unsigned int i = 0; i < this->events.size(); i++){
		unsigned int t = this->events[i].id;
		if(t == id){
			return this->funcs[i](a, b);
		}
	}
	
	return -1;
}

void inputHandler::printToTextField(textField* tF) {
	return;
}

void inputHandler::handleInput(){
	// 
	int c = getch();
	/*for (unsigned int i = 0; i < this->events.size(); i++) {
		printw("w");
	}*/
	if (c == 27) printw("escape     ");
	else if (c == KEY_F0 + 1) printw("F1       ");
	else if (c == KEY_F0 + 2) printw("F2       ");
	else if (c == KEY_F0 + 3) printw("F3       ");
	else if (c == KEY_F0 + 4) { 
		run = false;
		printw("F4       "); 
	}
	else if (c == 13) printw("Enter 13       ");
	else if (c == 10) printw("Enter 10       ");
	else if (c >= 32 && c <= 126) { 
		printw("%c", c); 
		printw("          ");
	}
	
	return;
}