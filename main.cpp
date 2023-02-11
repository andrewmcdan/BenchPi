#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include "menu.h"
#include "consoleHandler.h"
#include "main.h"
#include <algorithm>
#include <ctime>
#include "SerialHandler.h"
#include "RtMidi.h"
#include "MidiHandler.h"


int main() {
	setlocale(LC_ALL, "en_US.utf8");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	timeout(0);
	raw();
	if (has_colors() == FALSE) {
		endwin();
		puts("Your terminal does not support color");
	}
	start_color();
	refresh();
	bool run = true;
	

	consoleHandler mainWindow = consoleHandler();
	loopUpdateHandler loop = loopUpdateHandler();
	inputHandler userInput = inputHandler(&loop);	

	shortcutItem shortcutF1 = shortcutItem(1, []() {return 1; }, &mainWindow, "F1 - Main Menu", textField::textAlignment::center);
	shortcutItem shortcutF2 = shortcutItem(2, []() {return 1; }, &mainWindow, "F2 - Serial Options", textField::textAlignment::center);
	shortcutItem shortcutF3 = shortcutItem(3, []() {return 1; }, &mainWindow, "F3 - MIDI Config", textField::textAlignment::center);
	shortcutItem shortcutF4 = shortcutItem(4, []() {return 1; }, &mainWindow, "F4 - Quit", textField::textAlignment::center);
	loop.addEvent([&shortcutF1]() {
		return shortcutF1.tField.draw();
		});
	loop.addEvent([&shortcutF2]() {
		return shortcutF2.tField.draw();
		});
	loop.addEvent([&shortcutF3]() {
		return shortcutF3.tField.draw();
		});
	loop.addEvent([&shortcutF4]() {
		return shortcutF4.tField.draw();
		});

	textField quitConfirm(0, 0, mainWindow.width - 2, mainWindow.height - 1, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::center);

	shortcutF4.setInputListenerIdAndKey(
		userInput.addListener(
			[&userInput,&mainWindow,&loop,&quitConfirm,&run](int a, TIMEPOINT_T t) {
				mainWindow.clearScreen();
				int loopEvent = loop.addEvent([&quitConfirm]() {quitConfirm.draw(); return 1; });
				quitConfirm.setClearOnPrint(true);
				quitConfirm.setEnabled(true);
				char c[] = "Confirm Quit Y / N ?";
				quitConfirm.setText(c,20);
				userInput.addListener([&](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1;}, 'Y');
				userInput.addListener([&](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1; }, 'y');
				userInput.addListener([&, loopEvent](int c2, TIMEPOINT_T t2) { 
					quitConfirm.setEnabled(false); 
					userInput.removeByKey('Y'); 
					userInput.removeByKey('y');
					userInput.removeByKey('n'); 
					userInput.removeByKey('N');
					loop.remove(loopEvent); 
					mainWindow.clearScreen(); 
					return 1; }, 'n');
				userInput.addListener([&, loopEvent](int c2, TIMEPOINT_T t2) { 
					quitConfirm.setEnabled(false); 
					userInput.removeByKey('Y'); 
					userInput.removeByKey('y'); 
					userInput.removeByKey('n'); 
					userInput.removeByKey('N');
					loop.remove(loopEvent); 
					mainWindow.clearScreen(); 
					return 1; }, 'N');
				return 1; },
				KEY_F(4)),
			KEY_F(4));

	/*
	* Setup items:
	* 1. get list of all serial ports and query them to see what sort of device is attached.
	* 2. for all ammeters attached, set them up along with ammeter objects
	* 3. build menus
	* 
	*/

	textField testTextField(0, 0, mainWindow.width / 2, 10, COLOR_CYAN, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::textAlignment::left);
	char s[] = "test Text";
	testTextField.setText(s,9);
	testTextField.draw();

	textField anotherTF(0, 12, mainWindow.width / 4, 5, COLOR_WHITE, COLOR_RED, BORDER_ENABLED, &mainWindow, textField::textAlignment::left);
	anotherTF.setText("some more text");
	anotherTF.draw();

	std::chrono::steady_clock::now();

	// testing toggle a textField being visible or not
	userInput.addListener([&testTextField,&mainWindow](int c, TIMEPOINT_T t) {
		testTextField.toggleEnabled();
		mainWindow.clearScreen();
		
		std::string st = std::to_string((std::chrono::duration_cast<std::chrono::milliseconds> (t.time_since_epoch())).count());
		printw(st.c_str());
		return 0;
		}, KEY_F0 + 2);

	// testing printing text from keyboard into textField
	testTextField.setClearOnPrint(false);
	userInput.addListener([&testTextField](int c, TIMEPOINT_T t) {
		char c1 = c;
		testTextField.setText(&c1, 1);
		return 0;
		}, 'p');

	loop.addEvent([&testTextField]() {
		return testTextField.draw();
		});
	loop.addEvent([&anotherTF]() {
		return anotherTF.draw();
		});
	SerialHandler serialPortManager = SerialHandler();

	std::string portName = "/dev/ttyUSB0";
	//shortcutF2.tField.setText(std::to_string(serialPortManager.openPort("/dev/ttyUSB0")));
	serialPortManager.openPort(portName);
	serialPortManager.setPortConfig(portName, 9600, 8);
	serialPortManager.setTextFieldForPort("/dev/ttyUSB0", &anotherTF);

	loop.addEvent([&serialPortManager]() {
		serialPortManager.update();
		return 1;
		});

	while (run) {
		/*
		* Stuff that needs to go in main loop
		* 1. call draw methods of everything on screen
		* 2. monitor open serial ports for new data
		* 3. monitor stdin for user input
		* 4. call update on methods that register a loop update method
		*/

		loop.handleAll(); // handles all the loop events that hanve been registered
		refresh();
		// last thing to do in the loop is push the buffer to the dispaly.
		
		//wrefresh(aWin);
		// sleep for ~1ms so that the CPU isn't being hammered all the time.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
	unsigned int i = 0;
	auto it = this->id_s.begin();
	auto it2 = this->funcs.begin();
	for (; i < this->id_s.size() && it != this->id_s.end() && it2 != this->funcs.end(); it++,i++,it2++) {
		if (this->id_s.at(i) == id) {
			this->id_s.erase(it);
			this->funcs.erase(it2);
		}
	}
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

int inputHandler::addListener(std::function<int(int, TIMEPOINT_T)> f, int key){
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events[i].key == key) {
			this->removeByKey(key);
		}
		if (this->events[i].key == KEY_ALL_ASCII && key > 31 && key < 127) {
			this->removeByKey(KEY_ALL_ASCII);
		}
	}
	events_struct t = {key,this->id_index++,f};
	this->events.push_back(t);
	return this->events.size();
}

int inputHandler::remove(unsigned long id){
	auto it = this->events.begin();
	unsigned int i = 0;
	for (; i < this->events.size() && it != this->events.end(); i++, it++) {
		if (this->events.at(i).id == id && (this->events.begin() + i) < this->events.end()) {
			this->events.erase(it);
		}
	}
	return this->events.size();
}

int inputHandler::removeByKey(int key) {
	auto it = this->events.begin();
	unsigned int i = 0;
	for (; i < this->events.size() && it != this->events.end(); i++,it++) {
		if (this->events.at(i).key == key && (this->events.begin() + i) < this->events.end()) {
			this->events.erase(it);
		}
	}
	return this->events.size();
}

int inputHandler::call(unsigned long id, int a){
	TIMEPOINT_T t = std::chrono::steady_clock::now();
	for(unsigned int i = 0; i < this->events.size(); i++){
		if(this->events.at(i).id == id){
			return this->events.at(i).func(a, t);
		}
	}
	
	return -1;
}

void inputHandler::printToTextField(textField* tF) {
	
	return;
}

void inputHandler::handleInput(){
	// 
	auto t = std::chrono::steady_clock::now();
	int c = getch();
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events[i].key == c) {
			this->events[i].func(c, t);
		}
		if (this->events[i].key == KEY_ALL_ASCII && c > 31 && c < 127) {
			this->events[i].func(c, t);
		}
	}	
	return;
}

void inputHandler::resetEvents() {
	this->events.clear();
}