#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include "Menu.h"
#include "consoleHandler.h"
#include "main.h"
#include <algorithm>
#include <ctime>
#include "SerialHandler.h"
#include "RtMidi.h"
#include "MidiHandler.h"


//
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <fcntl.h>
//#include <xf86drm.h>
//#include <xf86drmMode.h>


int main() {
	/*drmModeRes* resources;
	int fd = open("/dev/dri/card0", O_RDWR);
	resources = drmModeGetResources(fd);*/

	//int max_width = resources->max_width;

	// Set up all the stuff for ncurses
	setlocale(LC_ALL, "en_US.utf8");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	timeout(0);
	ESCDELAY = 0;
	raw();
	if (has_colors() == FALSE) {
		endwin();
		puts("Your terminal does not support color");
	}
	start_color();
	refresh();

	// this variable is used in the loop below to exit the program
	bool run = true;
	
	// Init all the handler onjects
	consoleHandler mainWindow = consoleHandler();
	loopUpdateHandler loop = loopUpdateHandler();
	inputHandler userInput = inputHandler(&loop);
	MidiHandler midi = MidiHandler();

	// Set up the F-key shortcuts
	shortcutItem shortcutF1 = shortcutItem(1, []() {return 1; }, &mainWindow, "F1 - Main Menu", textField::textAlignment::center);
	shortcutItem shortcutF2 = shortcutItem(2, []() {return 1; }, &mainWindow, "F2 - Serial Config", textField::textAlignment::center);
	shortcutItem shortcutF3 = shortcutItem(3, []() {return 1; }, &mainWindow, "F3 - MIDI Config", textField::textAlignment::center);
	shortcutItem shortcutF4 = shortcutItem(4, []() {return 1; }, &mainWindow, "F4 - Options ", textField::textAlignment::center);
	shortcutItem shortcutF5 = shortcutItem(5, []() {return 1; }, &mainWindow, "F5 - Quit", textField::textAlignment::center);
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
	loop.addEvent([&shortcutF5]() {
		return shortcutF5.tField.draw();
		});

	
	//
	// Build the quit confirm screen.
	// 
	// 
	// Init a textfield for confirming quit. 
	textField quitConfirm(0, 0, mainWindow.width - 2, mainWindow.height - 1, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::center);
	// Connect the F4 shortcut with its key listener and instantiate all the events to happen when user responds to confirm
	shortcutF5.setInputListenerIdAndKey(
		userInput.addListener(
			[&userInput,&mainWindow,&loop,&quitConfirm,&run](int a, TIMEPOINT_T t) {
				// when the user pressed F4, clear the screen...
				mainWindow.clearScreen();
				// draw the quitConfirm textField
				int loopEvent = loop.addEvent([&quitConfirm]() {quitConfirm.draw(); return 1; });
				quitConfirm.setClearOnPrint(true);
				quitConfirm.setEnabled(true);
				char c[] = "Confirm Quit Y / N ?";
				quitConfirm.setText(c,20);
				// Add listeners for Y, y, N, n
				userInput.addListener([&](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1;}, 'Y');
				userInput.addListener([&](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1; }, 'y');
				userInput.addListener([&, loopEvent](int c2, TIMEPOINT_T t2) { 
					// if the user declines to quit, disable the quitConfirm textfield, remove the
					// listeners for Y, y, N, and n, remove the loop event for drawing the text field, 
					// and clear screen to go back to what was being done before. 
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
				KEY_F(5)),
			KEY_F(5));

	//
	// Build the menus
	// 
	//
	// build the main menu, F1
	Menu mainMenu = Menu(&loop, &mainWindow, &userInput, "Main Menu");
	mainMenu.addMenuItem("testMenu Item", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("another menu item", []() { return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item1", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item2", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item3", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item4", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item5", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item6", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item7", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item8", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("testMenu Item9", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item10", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item11", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item12", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item13", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item14", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item15", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item16", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item17", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item18", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item19", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item20", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item21", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item22", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item23", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item24", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item25", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item26", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item27", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item28", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item29", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item30", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item31", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item32", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item33", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item34", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item35", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item36", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item37", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item38", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item39", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item40", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item41", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item42", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item43", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item44", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item45", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item46", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item47", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item48", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item49", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item50", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item51", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item52", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item53", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item54", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item55", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item56", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item57", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item58", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item59", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item60", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item61", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item62", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item63", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item64", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item65", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("testMenu Item66", []() {return; }, & mainWindow);

	shortcutF1.setInputListenerIdAndKey(
		userInput.addListener(
			[&mainMenu,&userInput,&mainWindow](int c, TIMEPOINT_T time) {
				mainMenu.enableMenu();
				userInput.addListener([&](int c2, TIMEPOINT_T time2) {
					mainMenu.disableMenu();
					userInput.removeByKey(KEY_ESC);
					userInput.removeByKey(KEY_UP);
					userInput.removeByKey(KEY_DOWN);
					mainWindow.clearScreen();
					return 1; 
					},KEY_ESC);
				return 1;
			},
			KEY_F(1)),
		KEY_F(1));




	textField testTextField(0, 1, mainWindow.width / 2, 10, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::textAlignment::left);
	testTextField.setClearOnPrint(false);
	for (int i = 0; i < midi.midiInDevices.size(); i++) {
		testTextField.setText(midi.midiInDevices.at(i).name + "\r");
	}
	testTextField.draw();

	textField anotherTF(0, 13, mainWindow.width - 2, mainWindow.height/2, COLOR_WHITE, COLOR_BLACK, BORDER_DISABLED, &mainWindow, textField::textAlignment::left);
	//anotherTF.draw();
	anotherTF.setClearOnPrint(false);
	midi.openInPort(3,false,false,false,&anotherTF);
	if(midi.midiInDevices.size() > 1) midi.midiInDevices.at(3).enabled = true;
	loop.addEvent([&midi]() {
		midi.update();
		return 1;
		});

	anotherTF.setBorderColor(COLOR_CYAN);

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
		}, KEY_ALL_ASCII);

	loop.addEvent([&testTextField]() {
		return testTextField.draw();
		});
	loop.addEvent([&anotherTF]() {
		return anotherTF.draw();
		});
	SerialHandler serialPortManager = SerialHandler();

	std::string portName = "/dev/ttyUSB0";
	serialPortManager.openPort(portName);
	serialPortManager.setPortConfig(portName, 9600, 8);
	serialPortManager.setTextFieldForPort("/dev/ttyUSB0", &anotherTF);
	

	loop.addEvent([&serialPortManager]() {
		serialPortManager.update();
		return 1;
		});

	while (run) {
		loop.handleAll(); // handles all the loop events that hanve been registered
		// last thing to do in the loop is push the buffer to the dispaly.
		refresh();
		// sleep for ~1ms so that the CPU isn't being hammered all the time.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	serialPortManager.closeAllPorts();
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
			this->events.erase(this->events.begin() + i);
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