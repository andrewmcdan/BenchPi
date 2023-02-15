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
	timeout(1);
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
	SerialHandler serialPortManager = SerialHandler();

	// Set up the F-key shortcuts
	shortcutItem shortcutF1 = shortcutItem(1, []() {return 1; }, &mainWindow, "F1 - Main Menu", textField::textAlignment::center);
	shortcutItem shortcutF2 = shortcutItem(2, []() {return 1; }, &mainWindow, "F2 - Serial Config", textField::textAlignment::center);
	shortcutItem shortcutF3 = shortcutItem(3, []() {return 1; }, &mainWindow, "F3 - MIDI Config", textField::textAlignment::center);
	shortcutItem shortcutF4 = shortcutItem(4, []() {return 1; }, &mainWindow, "F4 - Text Area Options ", textField::textAlignment::center);
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
	
	Menu aSubMenu = Menu(&loop, &mainWindow, &userInput, "A Sub Menu");
	aSubMenu.addMenuItem("sub menu menu item1", []() {return 1; }, &mainWindow);
	aSubMenu.addMenuItem("sub menu menu item2", []() {return 1; }, &mainWindow);

	Menu loadConfigMenu = Menu(&loop, &mainWindow, &userInput, "Load Configuration From Disk");
	Menu saveConfigMenu = Menu(&loop, &mainWindow, &userInput, "Save Configuration To Disk");
	Menu enDisSerialDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable Serial Devices");
	Menu enDisMidiInDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Input Devices");
	Menu enDisMidiOutDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Output Devices");
	Menu configAmmetersVoltmetersMenu = Menu(&loop, &mainWindow, &userInput, "Config Ammeters / Voltmeters");
	Menu configMultiMeterMenu = Menu(&loop, &mainWindow, &userInput, "Configure Serial MultiMeter");


	

	// build the main menu, F1
	Menu mainMenu = Menu(&loop, &mainWindow, &userInput, "Main Menu");
	Menu serialConfigMenu = Menu(&loop, &mainWindow, &userInput, "Serial Config");
	Menu midiConfigMenu = Menu(&loop, &mainWindow, &userInput, "MIDI Config");
	Menu tAreaMenu = Menu(&loop, &mainWindow, &userInput, "Text Area Options");
	aSubMenu.setReferringMenu(&mainMenu);
	loadConfigMenu.setReferringMenu(&mainMenu);
	saveConfigMenu.setReferringMenu(&mainMenu);
	enDisSerialDevsMenu.setReferringMenu(&mainMenu);
	enDisMidiInDevsMenu.setReferringMenu(&mainMenu);
	enDisMidiOutDevsMenu.setReferringMenu(&mainMenu);
	configAmmetersVoltmetersMenu.setReferringMenu(&mainMenu);
	configMultiMeterMenu.setReferringMenu(&mainMenu);


	for (int i = 0; i < serialPortManager.getNumberOfPorts(); i++) {
		std::string temp1;
		serialPortManager.getPortName(i, temp1);
		std::string temp2;
		serialPortManager.getPortAlias(i, temp2);
		std::string temp3 = serialPortManager.getPortAvailable(i) ? "Enabled" : "Disabled";
		std::string temp4 = "";
		if (temp2.length() > 0) temp4 = temp1 + " : " + temp2 + " - " + temp3;
		else temp4 = temp1 + " - " + temp3;
		enDisSerialDevsMenu.addMenuItem(temp4, [&,i]() {
			serialPortManager.setPortAvaialble(i, !serialPortManager.getPortAvailable(i));
			std::string temp5;
			serialPortManager.getPortName(i, temp5);
			std::string temp6;
			serialPortManager.getPortAlias(i, temp2);
			std::string temp7 = serialPortManager.getPortAvailable(i) ? "Enabled" : "Disabled";
			std::string temp8 = "";
			if (temp6.length() > 0) temp8 = temp5 + " : " + temp6 + " - " + temp7;
			else temp8 = temp5 + " - " + temp7;
			enDisSerialDevsMenu.menuItems.at(i).tField.setText(temp8);
			return;
			}, &mainWindow);
	}


	for (int i = 0; i < midi.midiInDevices.size(); i++) {
		std::string temp2 = midi.midiInDevices.at(i).alias;
		std::string temp3 = midi.midiInDevices.at(i).enabled ? "Enabled" : "Disabled";
		std::string temp4 = "";
		if (temp2.length() == 0)temp4 = midi.midiInDevices.at(i).name + " - " + temp3;
		else temp4 = temp2 + " - " + temp3;
		enDisMidiInDevsMenu.addMenuItem(temp4, [&, i]() {
			midi.midiInDevices.at(i).enabled = !midi.midiInDevices.at(i).enabled;
			std::string temp6 = midi.midiInDevices.at(i).alias;
			std::string temp7 = midi.midiInDevices.at(i).enabled ? "Enabled" : "Disabled";
			std::string temp8 = "";
			if (temp6.length() == 0)temp8 = midi.midiInDevices.at(i).name + " - " + temp7;
			else temp8 = temp6 + " - " + temp7;
			enDisMidiInDevsMenu.menuItems.at(i).tField.setText(temp8);
			return;
			}, &mainWindow);
	}

	for (int i = 0; i < midi.midiOutDevices.size(); i++) {
		std::string temp2 = midi.midiOutDevices.at(i).alias;
		std::string temp3 = midi.midiOutDevices.at(i).enabled ? "Enabled" : "Disabled";
		std::string temp4 = "";
		if (temp2.length() == 0)temp4 = midi.midiOutDevices.at(i).name + " - " + temp3;
		else temp4 = temp2 + " - " + temp3;
		enDisMidiOutDevsMenu.addMenuItem(temp4, [&, i]() {
			midi.midiOutDevices.at(i).enabled = !midi.midiOutDevices.at(i).enabled;
			std::string temp6 = midi.midiOutDevices.at(i).alias;
			std::string temp7 = midi.midiOutDevices.at(i).enabled ? "Enabled" : "Disabled";
			std::string temp8 = "";
			if (temp6.length() == 0)temp8 = midi.midiOutDevices.at(i).name + " - " + temp7;
			else temp8 = temp6 + " - " + temp7;
			enDisMidiOutDevsMenu.menuItems.at(i).tField.setText(temp8);
			return;
			}, &mainWindow);
	}
	


	mainMenu.addMenuItem("A Sub Menu", [&]() {
		aSubMenu.enableMenu();
		aSubMenu.setEscKey([&]() {
			aSubMenu.disableMenu();
			mainMenu.enableMenu();
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.upKey();
				return 1;
				}, KEY_UP);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.downKey();
				return 1;
				}, KEY_DOWN);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.enterKey();
				return 1;
				}, KEY_ENTER);
			userInput.addListener([&](int c2, TIMEPOINT_T time2) {
				mainMenu.escKey();
				return 1;
				}, KEY_ESC);
			mainWindow.clearScreen();
			});
		userInput.addListener([&](int c2, TIMEPOINT_T time2) {
			aSubMenu.escKey();
			return 1;
			}, KEY_ESC);
		return; 
		}, &mainWindow);
	
	
	
	mainMenu.addMenuItem("Load Config", []() { return; }, &mainWindow);
	mainMenu.addMenuItem("Save Config", []() {return; }, &mainWindow);
	mainMenu.addMenuItem("En/Disable Serial Devices", [&]() {
		enDisSerialDevsMenu.enableMenu();
		enDisSerialDevsMenu.setEscKey([&]() {
			enDisSerialDevsMenu.disableMenu();
			mainMenu.enableMenu();
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.upKey();
				return 1;
				}, KEY_UP);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.downKey();
				return 1;
				}, KEY_DOWN);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.enterKey();
				return 1;
				}, KEY_ENTER);
			userInput.addListener([&](int c2, TIMEPOINT_T time2) {
				mainMenu.escKey();
				return 1;
				}, KEY_ESC);
			});
		userInput.addListener([&](int c2, TIMEPOINT_T time2) {
			enDisSerialDevsMenu.escKey();
			return 1;
			}, KEY_ESC);
		return; 
		}, &mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Input Devices", [&]() {
		enDisMidiInDevsMenu.enableMenu();
		enDisMidiInDevsMenu.setEscKey([&]() {
			enDisMidiInDevsMenu.disableMenu();
			mainMenu.enableMenu();
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.upKey();
				return 1;
				}, KEY_UP);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.downKey();
				return 1;
				}, KEY_DOWN);
			userInput.addListener([&](int c, TIMEPOINT_T t) {
				mainMenu.enterKey();
				return 1;
				}, KEY_ENTER);
			userInput.addListener([&](int c2, TIMEPOINT_T time2) {
				mainMenu.escKey();
				return 1;
				}, KEY_ESC);
			});
			userInput.addListener([&](int c2, TIMEPOINT_T time2) {
			enDisMidiInDevsMenu.escKey();
			return 1;
			}, KEY_ESC);
		return;
		}, &mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Output Devices", [&]() {
			enDisMidiOutDevsMenu.enableMenu();
			enDisMidiOutDevsMenu.setEscKey([&]() {
				enDisMidiOutDevsMenu.disableMenu();
				mainMenu.enableMenu();
				userInput.addListener([&](int c, TIMEPOINT_T t) {
					mainMenu.upKey();
					return 1;
					}, KEY_UP);
				userInput.addListener([&](int c, TIMEPOINT_T t) {
					mainMenu.downKey();
					return 1;
					}, KEY_DOWN);
				userInput.addListener([&](int c, TIMEPOINT_T t) {
					mainMenu.enterKey();
					return 1;
					}, KEY_ENTER);
				userInput.addListener([&](int c2, TIMEPOINT_T time2) {
					mainMenu.escKey();
					return 1;
					}, KEY_ESC);
				});
			userInput.addListener([&](int c2, TIMEPOINT_T time2) {
			enDisMidiOutDevsMenu.escKey();
			return 1;
			}, KEY_ESC);
		return;
		}, &mainWindow);
	mainMenu.addMenuItem("Configure Addon Controller (Teensy)", []() {return; }, & mainWindow);
	mainMenu.addMenuItem("Configure Serial MultiMeter", []() {return; }, &mainWindow);


	shortcutF1.setInputListenerIdAndKey(
		userInput.addListener(
			[&](int c, TIMEPOINT_T time) {
				userInput.setKeyDisabled(KEY_F(1), true);
				mainMenu.setEscKey([&]() {
					userInput.remove(shortcutF1.inputListenerID);
					userInput.removeByKey(KEY_UP);
					userInput.removeByKey(KEY_DOWN);
					mainMenu.disableMenu();
					mainWindow.clearScreen(); 
					userInput.setKeyDisabled(KEY_F(1), false);
					});
				mainMenu.enableMenu();
				userInput.addListener([&](int c2, TIMEPOINT_T time2) {
					mainMenu.escKey();
						return 1;
						}, KEY_ESC);
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
		}, KEY_F(2));

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
	events_struct t = {key,this->id_index++,f,false};
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
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).key == key) {
			this->remove(this->events.at(i).id);
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
		if (this->events[i].key == c && !this->events[i].disabled) {
			this->events[i].func(c, t);
		}
		if (this->events[i].key == KEY_ALL_ASCII && c > 31 && c < 127 && !this->events[i].disabled) {
			this->events[i].func(c, t);
		}
	}	
	return;
}

void inputHandler::setKeyDisabled(int key, bool en) {
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).key == key) this->events.at(i).disabled = en;
	}
}

void inputHandler::resetEvents() {
	this->events.clear();
}