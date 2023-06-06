///////////////////////////////////////////////////////////////////////////////
// The following block of defines is my hacky way of getting intellisense to
// play nice with all the lambdas in the menu structure. #define one at a time
// to work on that section of code. The project solution has a user defined 
// compiler switch that enables all the menus on compile.
///////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_ALL_MENUS
#define _TEXT_AREA_OPT_MENU_
#define _METER_CONFIG_MENU_
#define _MIDI_CONFIG_MENU_
#define _SERIAL_CONFIG_MENU_
#define _MAIN_MENU_
#endif

#define _METER_CONFIG_MENU_
/////////////////////////////////////////////////////////////////////////////

#include "main.h" 

namespace fs = std::filesystem;

#define AVG_LOOP_TIME_NUM 250
int64_t maxLoopTime_1s;
int64_t maxLoopTime;
int64_t avgLoopTime;
int64_t avgLoopTimeAvergerArr[AVG_LOOP_TIME_NUM];
uint16_t avgIndex;

bool displayLoopTime = true;
bool debugToRemoteDisplay = true;

int main() {
	avgIndex = 0;
	std::chrono::steady_clock::time_point programStartTime = std::chrono::steady_clock::now();
	bool emitOnce1 = false;
	bool emitOnce2 = false;
	bool emitOnce3 = false;
	bool emitOnce4 = false;
	bool emitOnce5 = false;
	bool emitOnce6 = false;
	bool emitOnce7 = false;
	bool emitOnce8 = false;
	bool emitOnce9 = false;


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
		//endwin();
		puts("Your terminal does not support color");
	}
	start_color();
	refresh();


	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Testing out some stuff with mouse input. Maybe use ncurses for mouse stuff?
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*int mouseFD = open("/dev/input/mice", O_RDONLY);
	printf(std::to_string(mouseFD).c_str()); 
	char read_buf[8];
	for (int i = 0; i < 8; i++) read_buf[i] = 0;
	while (true) {
		int n = read(mouseFD, &read_buf, sizeof(read_buf));
		if (n > 0) {
			for (int i = 0; i < 8; i++) {
				printf((std::to_string((int)read_buf[i]) + " ").c_str());
	
			}
			printf("\n\r\n\r");
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}*/
	// This should be implemented using events instead of reading raw data from the device.
	// see https://jiafei427.wordpress.com/2017/03/13/linux-reading-the-mouse-events-datas-from-devinputmouse0/ 
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this variable is used in the loop below to exit the program
	bool run = true;
	
	// Init all the handler objects
	consoleHandler mainWindow = consoleHandler();
	loopUpdateHandler loop = loopUpdateHandler();
	inputHandler userInput = inputHandler(&loop);
	MidiHandler midi = MidiHandler();
	SerialHandler serialPortManager = SerialHandler();
	WindowManager windowManager = WindowManager(&mainWindow, &loop, &userInput);
	AddonController teensyController = AddonController(&serialPortManager);

	loop.addEvent([&serialPortManager]() {
		serialPortManager.update();
		return 1;
	});
	

	// Set up the F-key shortcuts
	shortcutItem shortcutF1 = shortcutItem(1, &mainWindow, "F1 - Main Menu", textField::textAlignment::center);
	shortcutItem shortcutF2 = shortcutItem(2, &mainWindow, "F2 - Serial Config", textField::textAlignment::center);
	shortcutItem shortcutF3 = shortcutItem(3, &mainWindow, "F3 - MIDI Config", textField::textAlignment::center);
	shortcutItem shortcutF4 = shortcutItem(4, &mainWindow, "F4 - Meter Config", textField::textAlignment::center);
	shortcutItem shortcutF5 = shortcutItem(5, &mainWindow, "F5 - Text Area Options ", textField::textAlignment::center);
	shortcutItem shortcutF6 = shortcutItem(6, &mainWindow, "F6 - Quit", textField::textAlignment::center);
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
	loop.addEvent([&shortcutF6]() {
		return shortcutF6.tField.draw();
		});
	

	// Build the quit confirm screen.
	// Init a textfield for confirming quit. 
	textField quitConfirm(0, 0, mainWindow.width + 1, mainWindow.height + 1, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::center);
	// Connect the F4 shortcut with its key listener and instantiate all the events to happen when user responds to confirm

	shortcutF6.setInputListenerIdAndKey(
		userInput.addListener(
			[&userInput,&mainWindow,&loop,&quitConfirm,&run](int a, TIMEPOINT_T t) {
				// when the user pressed F4, clear the screen...
				mainWindow.clearScreen();
				userInput.setKeyDisabled(KEY_F(1), true);
				userInput.setKeyDisabled(KEY_F(2), true);
				userInput.setKeyDisabled(KEY_F(3), true);
				userInput.setKeyDisabled(KEY_F(4), true);
				userInput.setKeyDisabled(KEY_F(5), true);
				userInput.setKeyDisabled(KEY_F(11), true);
				userInput.setKeyDisabled(KEY_F(12), true);
				// draw the quitConfirm textField
				int loopEvent = loop.addEvent([&quitConfirm]() {quitConfirm.draw(); return 1; });
				quitConfirm.setClearOnPrint(true);
				quitConfirm.setEnabled(true);
				char c[] = "Confirm Quit Y / N ?";
				quitConfirm.setText(c,20);
				// Add listeners for Y, y, N, n
				userInput.addListener([&run, &mainWindow](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1; }, 'Y');
				userInput.addListener([&run, &mainWindow](int c2, TIMEPOINT_T t2) { run = false; mainWindow.clearScreen(); return 1; }, 'y');
				userInput.addListener([&loopEvent, &quitConfirm, &userInput, &loop, &mainWindow](int c2, TIMEPOINT_T t2) { 
					// if the user declines to quit, disable the quitConfirm textfield, remove the
					// listeners for Y, y, N, and n, remove the loop event for drawing the text field, 
					// and clear screen to go back to what was being done before. 
					quitConfirm.setEnabled(false); 
					userInput.removeListenerByKey('Y'); 
					userInput.removeListenerByKey('y');
					userInput.removeListenerByKey('n'); 
					userInput.removeListenerByKey('N');
					userInput.setKeyDisabled(KEY_F(1), false);
					userInput.setKeyDisabled(KEY_F(2), false);
					userInput.setKeyDisabled(KEY_F(3), false);
					userInput.setKeyDisabled(KEY_F(4), false);
					userInput.setKeyDisabled(KEY_F(5), false);
					userInput.setKeyDisabled(KEY_F(11), false);
					userInput.setKeyDisabled(KEY_F(12), false);
					loop.remove(loopEvent); 
					mainWindow.clearScreen(); 
					return 1; }, 'n');
				userInput.addListener([&loopEvent, &quitConfirm, &userInput, &loop, &mainWindow](int c2, TIMEPOINT_T t2) {
					quitConfirm.setEnabled(false); 
					userInput.removeListenerByKey('Y'); 
					userInput.removeListenerByKey('y'); 
					userInput.removeListenerByKey('n'); 
					userInput.removeListenerByKey('N');
					userInput.setKeyDisabled(KEY_F(1), false);
					userInput.setKeyDisabled(KEY_F(2), false);
					userInput.setKeyDisabled(KEY_F(3), false);
					userInput.setKeyDisabled(KEY_F(4), false);
					userInput.setKeyDisabled(KEY_F(5), false);
					userInput.setKeyDisabled(KEY_F(11), false);
					userInput.setKeyDisabled(KEY_F(12), false);
					loop.remove(loopEvent); 
					mainWindow.clearScreen(); 
					return 1; }, 'N');
				return 1; },
				KEY_F(6)),
			KEY_F(6));

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Build the menus
	// 
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// MAIN Menu
	///////////////////////////////////////////////////////////////////////////////////////////////////
	
#ifdef _MAIN_MENU_
	Menu mainMenu = Menu(&loop, &mainWindow, &userInput, "Main Menu");
	
	Menu loadConfigMenu = Menu(&loop, &mainWindow, &userInput, "Load Configuration From Disk");
	Menu saveConfigMenu = Menu(&loop, &mainWindow, &userInput, "Save Configuration To Disk");
	Menu enDisSerialDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable Serial Devices");
	Menu enDisMidiInDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Input Devices");
	Menu enDisMidiOutDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Output Devices");
	Menu configAddonControllerMenu = Menu(&loop, &mainWindow, &userInput, "Configure Addon Controller (Teensy)");
	Menu configMultiMeterMenu = Menu(&loop, &mainWindow, &userInput, "Configure Serial MultiMeter");
	
	Menu addonControllerSubMenu_selectSerialPort = Menu(&loop, &mainWindow, &userInput, "Select Serial Port");
	Menu serialMultiMeterSubMenu_SelectSerialPort = Menu(&loop, &mainWindow, &userInput, "Select Serial Port(s)");

	for (int i = 0; i < serialPortManager.getNumberOfPorts(); i++) {
		std::string temp1;
		serialPortManager.getPortName(i, temp1);
		std::string temp2;
		serialPortManager.getPortAlias(i, temp2);
		std::string temp3 = serialPortManager.getPortAvailable(i) ? "Enabled" : "Disabled";
		std::string temp4 = "";
		if (temp2.length() > 0) temp4 = temp1 + " : " + temp2 + " - " + temp3;
		else temp4 = temp1 + " - " + temp3;
		enDisSerialDevsMenu.addMenuItem(temp4, [&serialPortManager, &temp2, &enDisSerialDevsMenu, i]() {
			serialPortManager.setPortAvailable(i, !serialPortManager.getPortAvailable(i));
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
		enDisMidiInDevsMenu.addMenuItem(temp4, [&midi, &enDisMidiInDevsMenu, i]() {
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
		enDisMidiOutDevsMenu.addMenuItem(temp4, [&midi, &enDisMidiOutDevsMenu, i]() {
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

	configAddonControllerMenu.addMenuItem("Select Serial Port", [&addonControllerSubMenu_selectSerialPort, &serialPortManager, &teensyController, &mainWindow, &configAddonControllerMenu]() {
		addonControllerSubMenu_selectSerialPort.resetMenuItemList();
		for (int i = 0, j = 0; i < serialPortManager.getNumberOfPorts(); i++) {
			std::string name;
			serialPortManager.getPortName(i, name);
			std::string alias;
			serialPortManager.getPortAlias(i, alias);
			bool available = serialPortManager.getPortAvailable(i);
			if (available) {
				addonControllerSubMenu_selectSerialPort.addMenuItem((alias.length() > 0) ? alias : name, [&teensyController, &addonControllerSubMenu_selectSerialPort, j, name]() {
					if (teensyController.setPort(name)) {
						addonControllerSubMenu_selectSerialPort.menuItems.at(j).tField.setText("Successful");
					}
					else {
						addonControllerSubMenu_selectSerialPort.menuItems.at(j).tField.setText("ERROR");
					}
					}, &mainWindow);
				j++;
			}
		}
		addonControllerSubMenu_selectSerialPort.enableMenu();
		addonControllerSubMenu_selectSerialPort.setEscKey([&addonControllerSubMenu_selectSerialPort, &configAddonControllerMenu]() {
			addonControllerSubMenu_selectSerialPort.disableMenu();
			configAddonControllerMenu.enableMenu();
			});
		}, & mainWindow);
	configMultiMeterMenu.addMenuItem("Select Serial Port(s)", [&serialMultiMeterSubMenu_SelectSerialPort, &configMultiMeterMenu]() {
		serialMultiMeterSubMenu_SelectSerialPort.enableMenu();
		serialMultiMeterSubMenu_SelectSerialPort.setEscKey([&serialMultiMeterSubMenu_SelectSerialPort, &configMultiMeterMenu]() {
			serialMultiMeterSubMenu_SelectSerialPort.disableMenu();
			configMultiMeterMenu.enableMenu();
			});
		}, & mainWindow);

	mainMenu.addMenuItem("Load Config", []() { /* @TODO: */ }, &mainWindow);
	mainMenu.addMenuItem("Save Config", []() { /* @TODO: */ }, &mainWindow);
	mainMenu.addMenuItem("En/Disable Serial Devices", [&enDisSerialDevsMenu,&mainMenu]() {
		enDisSerialDevsMenu.enableMenu();
		enDisSerialDevsMenu.setEscKey([&enDisSerialDevsMenu,&mainMenu]() {
			enDisSerialDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Input Devices", [&enDisMidiInDevsMenu, &mainMenu]() {
		enDisMidiInDevsMenu.enableMenu();
		enDisMidiInDevsMenu.setEscKey([&enDisMidiInDevsMenu, &mainMenu]() {
			enDisMidiInDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Output Devices", [&enDisMidiOutDevsMenu, &mainMenu]() {
		enDisMidiOutDevsMenu.enableMenu();
		enDisMidiOutDevsMenu.setEscKey([&enDisMidiOutDevsMenu, &mainMenu]() {
			enDisMidiOutDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);
	mainMenu.addMenuItem("Configure Addon Controller (Teensy)", [&configAddonControllerMenu, &mainMenu]() {
		configAddonControllerMenu.enableMenu();
		configAddonControllerMenu.setEscKey([&configAddonControllerMenu, &mainMenu]() {
			configAddonControllerMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);
	mainMenu.addMenuItem("Configure Serial MultiMeter", [&configMultiMeterMenu, &mainMenu]() {
		configMultiMeterMenu.enableMenu();
		configMultiMeterMenu.setEscKey([&configMultiMeterMenu, &mainMenu]() {
			configMultiMeterMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);

	shortcutF1.setInputListenerIdAndKey(
		userInput.addListener([&userInput, &mainMenu, &mainWindow](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(5), true);
			userInput.setKeyDisabled(KEY_F(11), true);
			userInput.setKeyDisabled(KEY_F(12), true);
			mainMenu.setEscKey([&mainMenu, &userInput, &mainWindow]() {
				mainMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeListenerByKey(KEY_UP);
				userInput.removeListenerByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(5), false);
				userInput.setKeyDisabled(KEY_F(11), false);
				userInput.setKeyDisabled(KEY_F(12), false);
				});
			mainMenu.enableMenu();
			return 1;
			},
			KEY_F(1)),
		KEY_F(1));

#endif
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Serial Config Menu
	///////////////////////////////////////////////////////////////////////////////////////////////////
	
#ifdef _SERIAL_CONFIG_MENU_
	Menu serialConfigMenu = Menu(&loop, &mainWindow, &userInput, "Serial Config");
	std::vector<Menu> serialConfigMenuItems;
	std::vector<std::vector<Menu>> serialConfigSubMenuItems;
	Menu serialPortConfig_openClosePort(&loop, &mainWindow, &userInput, "Open / Close Port");

	shortcutF2.setInputListenerIdAndKey(
		userInput.addListener([&serialPortManager, &loop, &mainWindow, &userInput, &serialConfigSubMenuItems, &serialConfigMenu, &serialConfigMenuItems, &serialPortConfig_openClosePort, &teensyController](int c, TIMEPOINT_T time) {
			for (int i = 0; i < serialPortManager.getNumberOfPorts(); i++) {
				if (serialPortManager.getPortAvailable(i)) {
					std::string tempString;
					serialPortManager.getPortName(i, tempString);
					std::string portName = tempString;
					std::string tempString2;
					serialPortManager.getPortAlias(i, tempString2);
					if (tempString2.length() > 0)tempString += " - " + tempString2;
					serialConfigMenuItems.push_back(Menu(&loop, &mainWindow, &userInput, tempString));
					serialConfigSubMenuItems.push_back(std::vector<Menu>());
					serialConfigMenu.addMenuItem(tempString, [&loop, &mainWindow, &userInput, &serialConfigSubMenuItems, &serialConfigMenuItems, &serialPortConfig_openClosePort, &serialPortManager, &serialConfigMenu, i, portName]{
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Open / Close Port"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Set / Change Alias"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Baud"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Bits Per Byte"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Parity"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Stop bits"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Hardware flow control"));

						serialConfigMenuItems.at(i).addMenuItem("Open / Close Port", [&serialPortManager, &serialPortConfig_openClosePort, &mainWindow, &serialConfigMenuItems, i, portName]() {
							if (serialPortManager.isPortOpen(portName)) {
								serialPortConfig_openClosePort.addMenuItem("Port is open", []{}, &mainWindow);
								serialPortConfig_openClosePort.addMenuItem("Close port", [&serialPortManager, &serialPortConfig_openClosePort, portName]{
									if (!serialPortManager.closePort(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									if (!serialPortManager.isPortOpen(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Port closed");
									}
									else {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									}, &mainWindow);
							}
							else {
								serialPortConfig_openClosePort.addMenuItem("Port is closed (or unavailable)", [] {}, & mainWindow);
								serialPortConfig_openClosePort.addMenuItem("Attempt to open port", [&serialPortManager, &serialPortConfig_openClosePort, portName]{
									if (!serialPortManager.openPort(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									if (serialPortManager.isPortOpen(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Port opened");
									}
									else {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}


									}, &mainWindow);
							}
							serialPortConfig_openClosePort.enableMenu();
							serialPortConfig_openClosePort.setEscKey([&serialConfigMenuItems, &serialPortConfig_openClosePort, i] {
								serialPortConfig_openClosePort.disableMenu();
								serialPortConfig_openClosePort.resetMenuItemList();
								serialConfigMenuItems.at(i).enableMenu();
								});
							}, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Set / Change Alias", [i] {/* @TODO: */return; }, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Baud", [&, i]() {
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("600", [&, i, portName] {
								if (serialPortManager.setPortConfig(portName, 600) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(0).tField.setText("Successfully set to 600 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(0).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1200", [&, i, portName] {
								if (serialPortManager.setPortConfig(portName, 1200) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(1).tField.setText("Successfully set to 1200 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(1).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1800", [&, i, portName] {
								if (serialPortManager.setPortConfig(portName, 1800) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(2).tField.setText("Successfully set to 1800 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(2).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("2400", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 2400) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(3).tField.setText("Successfully set to 2400 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(3).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("4800", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 4800) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(4).tField.setText("Successfully set to 4800 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(4).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("9600", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 9600) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(5).tField.setText("Successfully set to 9600 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(5).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("19200", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 19200) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(6).tField.setText("Successfully set to 19200 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(6).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("31250 - MIDI", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 31250) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(7).tField.setText("Successfully set to 31250 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(7).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("38400", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 38400) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(8).tField.setText("Successfully set to 38400 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(8).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("57600", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 57600) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(9).tField.setText("Successfully set to 57600 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(9).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("115200", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 115200) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(10).tField.setText("Successfully set to 115200 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(10).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("230400", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 230400) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(11).tField.setText("Successfully set to 230400 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(11).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("460800", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 460800) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(12).tField.setText("Successfully set to 460800 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(12).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("500000", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 500000) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(13).tField.setText("Successfully set to 500000 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(13).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("750000", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 750000) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(14).tField.setText("Successfully set to 750000 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(14).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1000000", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 1000000) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(15).tField.setText("Successfully set to 1000000 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(15).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1250000", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 1250000) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(16).tField.setText("Successfully set to 1250000 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(16).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1500000", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 1500000) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(17).tField.setText("Successfully set to 1500000 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(17).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).setEscKey([&, i]() {
								serialConfigSubMenuItems.at(i).at(2).disableMenu();
								serialConfigMenuItems.at(i).enableMenu();
								serialConfigSubMenuItems.at(i).at(2).resetMenuItemList();
								});
							serialConfigSubMenuItems.at(i).at(2).enableMenu();
							return;
							}, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Bits Per Byte !TODO!", [&, i]() {/* @TODO: */return; }, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Parity !TODO!", [&, i]() {/* @TODO: */return; }, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Stop Bits !TODO!", [&, i]() {/* @TODO: */return; }, &mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Hardware Flow Control !TODO!", [&, i]() {/* @TODO: */return; }, &mainWindow);
						serialConfigMenuItems.at(i).setEscKey([&, i]() {
							serialConfigMenuItems.at(i).disableMenu();
							while (serialConfigSubMenuItems.at(i).size() > 0) {
								serialConfigSubMenuItems.at(i).pop_back();
							}
							serialConfigMenu.enableMenu();
							serialConfigMenuItems.at(i).resetMenuItemList();
							});
						serialConfigMenuItems.at(i).enableMenu();
						return;
						}, &mainWindow);
				}
			}
			for (int i = 0; i < teensyController.serialPorts_v.size(); i++) {
				std::string portName = teensyController.serialPorts_v.at(i).name;
				std::string alias = teensyController.serialPorts_v.at(i).alias;
				if (alias.length() > 0) portName += " - " + alias;
				serialConfigMenuItems.push_back(Menu(&loop, &mainWindow, &userInput, portName));
				serialConfigMenu.addMenuItem(portName, [&, i, portName]() {
					serialConfigSubMenuItems.push_back(std::vector<Menu>());
					serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Set / Change Alias"));
					serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Baud"));
					serialConfigMenuItems.at(i).addMenuItem("Set / Change Alias", [&, i]() {/* @TODO: */return; }, &mainWindow);
					serialConfigMenuItems.at(i).addMenuItem("Baud", [&, i]() {
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("600", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 600) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(0).tField.setText("Successfully set to 600 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(0).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("1200", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 1200) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(1).tField.setText("Successfully set to 1200 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(1).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("1800", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 1800) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(2).tField.setText("Successfully set to 1800 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(2).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("2400", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 2400) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(3).tField.setText("Successfully set to 2400 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(3).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("4800", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 4800) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(4).tField.setText("Successfully set to 4800 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(4).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("9600", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 9600) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(5).tField.setText("Successfully set to 9600 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(5).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("19200", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 19200) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(6).tField.setText("Successfully set to 19200 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(6).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("31250 - MIDI", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 31250) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(7).tField.setText("Successfully set to 31250 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(7).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("38400", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 38400) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(8).tField.setText("Successfully set to 38400 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(8).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("57600", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 57600) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(9).tField.setText("Successfully set to 57600 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(9).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("115200", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 115200) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(10).tField.setText("Successfully set to 115200 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(10).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("230400", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 230400) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(11).tField.setText("Successfully set to 230400 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(11).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("460800", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 460800) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(12).tField.setText("Successfully set to 460800 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(12).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("500000", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 500000) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(13).tField.setText("Successfully set to 500000 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(13).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("750000", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 750000) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(14).tField.setText("Successfully set to 750000 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(14).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("1000000", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 1000000) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(15).tField.setText("Successfully set to 1000000 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(15).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("1250000", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 1250000) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(16).tField.setText("Successfully set to 1250000 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(16).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).addMenuItem("1500000", [&, i, portName]() {
							if (teensyController.setSerialBaud(i, 1500000) == 1) {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(17).tField.setText("Successfully set to 1500000 baud");
							}
							else {
								serialConfigSubMenuItems.at(i).at(1).menuItems.at(17).tField.setText("Error setting baud. Is the port open?");
							}
							return;
							}, &mainWindow);
						serialConfigSubMenuItems.at(i).at(1).setEscKey([&, i]() {
							serialConfigSubMenuItems.at(i).at(1).disableMenu();
							serialConfigMenuItems.at(i).enableMenu();
							serialConfigSubMenuItems.at(i).at(1).resetMenuItemList();
							});
						serialConfigSubMenuItems.at(i).at(1).enableMenu();
						return;
						}, &mainWindow);
					serialConfigMenuItems.at(i).setEscKey([&, i]() {
						serialConfigMenuItems.at(i).disableMenu();
						while (serialConfigSubMenuItems.at(i).size() > 0) {
							serialConfigSubMenuItems.at(i).pop_back();
						}
						serialConfigMenu.enableMenu();
						serialConfigMenuItems.at(i).resetMenuItemList();
						});
					serialConfigMenuItems.at(i).enableMenu();
					}, &mainWindow);
			}
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(5), true);
			userInput.setKeyDisabled(KEY_F(11), true);
			userInput.setKeyDisabled(KEY_F(12), true);
			serialConfigMenu.setEscKey([&]() {
				userInput.removeListenerByKey(KEY_UP);
				userInput.removeListenerByKey(KEY_DOWN);
				serialConfigMenu.disableMenu();
				serialConfigMenu.resetMenuItemList();
				while (serialConfigMenuItems.size() > 0) {
					serialConfigMenuItems.pop_back();
				}
				while (serialConfigSubMenuItems.size() > 0) {
					serialConfigSubMenuItems.pop_back();
				}
				mainWindow.clearScreen();
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(5), false);
				userInput.setKeyDisabled(KEY_F(11), false);
				userInput.setKeyDisabled(KEY_F(12), false);
				});
			serialConfigMenu.enableMenu();
			return 1;
			}, KEY_F(2)
		), KEY_F(2)
	);
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Midi Config Menu
	///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MIDI_CONFIG_MENU_
	Menu midiConfigMenu = Menu(&loop, &mainWindow, &userInput, "MIDI Config");

	shortcutF3.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(5), true);
			midiConfigMenu.setEscKey([&]() {
				midiConfigMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeListenerByKey(KEY_UP);
				userInput.removeListenerByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(5), false);
				});
			midiConfigMenu.enableMenu();
			return 1;
			},
			KEY_F(3)),
		KEY_F(3));
#endif
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Meter Config Menu
	///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _METER_CONFIG_MENU_
	Menu meterConfigMenu = Menu(&loop, &mainWindow, &userInput, "Meter Config");


	// @TODO: this entire section needs to be rewritten
	/*
    Menu meterConfigSubMenu_meterSelect = Menu(&loop, &mainWindow, &userInput, "Meter Select");
    Menu meterConfigSubMenu_meterType = Menu(&loop, &mainWindow, &userInput, "Meter Type");
    Menu meterConfigSubMenu_meterRange = Menu(&loop, &mainWindow, &userInput, "Meter Range");
    Menu meterConfigSubMenu_meterScale = Menu(&loop, &mainWindow, &userInput, "Meter Scale");
    Menu meterConfigSubMenu_meterOptions = Menu(&loop, &mainWindow, &userInput, "Meter Options");
    Menu meterConfigSubMenu_meterDisplay = Menu(&loop, &mainWindow, &userInput, "Meter Display");
    Menu meterConfigSubMenu_meterColor = Menu(&loop, &mainWindow, &userInput, "Meter Color");
	*/
	/*
	meterConfigMenu.addMenuItem("Meter Select", [&]() {
		meterConfigSubMenu_meterSelect.resetMenuItemList();
		if (windowManager.getSelectedWindow()->type == WindowManager::windowType::AMMETER) {
			for (size_t i = 0; i < teensyController.ammeters_v.size(); i++) {
				meterConfigSubMenu_meterSelect.addMenuItem(teensyController.ammeters_v[i].name, [&, i]() {
					
					}, &mainWindow);
			}
		}
		if (windowManager.getSelectedWindow()->type == WindowManager::windowType::VOLTMETER) {
			for (size_t i = 0; i < teensyController.voltmeters_v.size(); i++) {
				meterConfigSubMenu_meterSelect.addMenuItem(teensyController.voltmeters_v[i].name, [&, i]() {
					
					}, &mainWindow);
			}
		}

        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterSelect.enableMenu();
		meterConfigSubMenu_meterSelect.setEscKey([&]() {
			meterConfigSubMenu_meterSelect.disableMenu();
			meterConfigMenu.enableMenu();
			});
    }, &mainWindow);
	meterConfigMenu.addMenuItem("Type", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterType.enableMenu();
        meterConfigSubMenu_meterType.setEscKey([&]() {
            meterConfigSubMenu_meterType.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
    meterConfigMenu.addMenuItem("Range", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterRange.enableMenu();
        meterConfigSubMenu_meterRange.setEscKey([&]() {
            meterConfigSubMenu_meterRange.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
    meterConfigMenu.addMenuItem("Scale", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterScale.enableMenu();
        meterConfigSubMenu_meterScale.setEscKey([&]() {
            meterConfigSubMenu_meterScale.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
    meterConfigMenu.addMenuItem("Options", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterOptions.enableMenu();
        meterConfigSubMenu_meterOptions.setEscKey([&]() {
            meterConfigSubMenu_meterOptions.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
    meterConfigMenu.addMenuItem("Display", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterDisplay.enableMenu();
        meterConfigSubMenu_meterDisplay.setEscKey([&]() {
            meterConfigSubMenu_meterDisplay.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
    meterConfigMenu.addMenuItem("Color", [&]() {
        meterConfigMenu.disableMenu();
        meterConfigSubMenu_meterColor.enableMenu();
        meterConfigSubMenu_meterColor.setEscKey([&meterConfigSubMenu_meterColor, &meterConfigMenu]() {
            meterConfigSubMenu_meterColor.disableMenu();
            meterConfigMenu.enableMenu();
        });
    }, &mainWindow);
	*/
    // @TODO: add all the stuff to the menus above

	std::vector<Menu> meterConfigMenuItems;
	std::vector<std::vector<Menu>> meterConfigSubMenuItems;

	shortcutF4.setInputListenerIdAndKey(
		userInput.addListener([&meterConfigMenu, &mainWindow, &userInput, &teensyController, &meterConfigMenuItems, &meterConfigSubMenuItems, &loop](int c, TIMEPOINT_T time) {
			for (size_t i = 0; i < teensyController.ammeters_v.size(); i++) {
				std::string meterName = teensyController.ammeters_v.at(i).name;
				meterConfigMenuItems.push_back(Menu(&loop, &mainWindow, &userInput, meterName));
				meterConfigSubMenuItems.push_back(std::vector<Menu>());
				meterConfigMenu.addMenuItem(meterName, [&meterConfigMenu, &loop, &mainWindow, &userInput, &meterConfigMenuItems, &meterConfigSubMenuItems, &teensyController, i, meterName] {
					meterConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Scale"));
					meterConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Options"));
					meterConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Display"));
					meterConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Color"));
					std::string menuItemName = meterName + " - Scale";
					meterConfigMenuItems.at(i).addMenuItem(menuItemName, [&, i, meterName] {
						meterConfigSubMenuItems.at(i).at(0).addMenuItem("Milli-Amps", [&, i, meterName] {
							teensyController.ammeters_v.at(i).scale = AddonController::ampScale::MILLIAMPS;
							meterConfigSubMenuItems.at(i).at(0).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(0).resetMenuItemList();
							}, &mainWindow);
						meterConfigSubMenuItems.at(i).at(0).addMenuItem("Amps", [&, i, meterName] {
							teensyController.ammeters_v.at(i).scale = AddonController::ampScale::AMPS;
							meterConfigSubMenuItems.at(i).at(0).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(0).resetMenuItemList();
							}, &mainWindow);
						meterConfigSubMenuItems.at(i).at(0).setEscKey([&, i] {
							meterConfigSubMenuItems.at(i).at(0).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(0).resetMenuItemList();
							});
						meterConfigSubMenuItems.at(i).at(0).enableMenu();
						}, &mainWindow);

					menuItemName = meterName + " - Options";
					meterConfigMenuItems.at(i).addMenuItem(menuItemName, [&, i, meterName] {
						meterConfigSubMenuItems.at(i).at(1).addMenuItem("Milli-Amps", [&, i, meterName] {
							teensyController.ammeters_v.at(i).scale = AddonController::ampScale::MILLIAMPS;
							meterConfigSubMenuItems.at(i).at(1).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(1).resetMenuItemList();
							}, &mainWindow);
						meterConfigSubMenuItems.at(i).at(1).addMenuItem("Amps", [&, i, meterName] {
							teensyController.ammeters_v.at(i).scale = AddonController::ampScale::AMPS;
							meterConfigSubMenuItems.at(i).at(1).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(1).resetMenuItemList();
							}, &mainWindow);
						meterConfigSubMenuItems.at(i).at(1).setEscKey([&, i] {
							meterConfigSubMenuItems.at(i).at(1).disableMenu();
							meterConfigMenuItems.at(i).enableMenu();
							meterConfigSubMenuItems.at(i).at(1).resetMenuItemList();
							});
						meterConfigSubMenuItems.at(i).at(1).enableMenu();
						}, &mainWindow);
					meterConfigMenuItems.at(i).setEscKey([&, i] {
						meterConfigMenuItems.at(i).disableMenu();
						while (meterConfigSubMenuItems.at(i).size() > 0) {
							meterConfigSubMenuItems.at(i).pop_back();
						}
						meterConfigMenu.enableMenu();
						meterConfigMenuItems.at(i).resetMenuItemList();
						});
					meterConfigMenuItems.at(i).enableMenu();
					}, &mainWindow);
			}
			for (size_t i = 0; i < teensyController.voltmeters_v.size(); i++) {

			}
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(5), true);
			userInput.setKeyDisabled(KEY_F(11), true);
			userInput.setKeyDisabled(KEY_F(12), true);
			meterConfigMenu.setEscKey([&meterConfigMenu, &mainWindow, &userInput, &meterConfigMenuItems, &meterConfigSubMenuItems]() {
				meterConfigMenu.disableMenu();
				meterConfigMenu.resetMenuItemList();
				while (meterConfigMenuItems.size() > 0) {
					meterConfigMenuItems.pop_back();
				}
				while (meterConfigSubMenuItems.size() > 0) {
					meterConfigSubMenuItems.pop_back();
				}
				mainWindow.clearScreen();
				userInput.removeListenerByKey(KEY_UP);
				userInput.removeListenerByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(5), false);
				userInput.setKeyDisabled(KEY_F(11), false);
				userInput.setKeyDisabled(KEY_F(12), false);
				});
			meterConfigMenu.enableMenu();
			return 1;
			},
			KEY_F(4)),
		KEY_F(4));
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Text Area Options Menu
	///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _TEXT_AREA_OPT_MENU_
	Menu tAreaMenu = Menu(&loop, &mainWindow, &userInput, "Text Area Options");

	Menu tAreaSubMenu_enDisTitle(&loop, &mainWindow, &userInput, "Area Title");
	Menu tAreaSubMenu_mode(&loop, &mainWindow, &userInput, "Mode");
	Menu tAreaSubMenu_Split(&loop, &mainWindow, &userInput, "Split");
	Menu tAreaSubMenu_Merge(&loop, &mainWindow, &userInput, "Merge");
	Menu tAreaSubMenu_Resize(&loop, &mainWindow, &userInput, "Resize");
	Menu tAreaSubMenu_Move(&loop, &mainWindow, &userInput, "Move");
	Menu tAreaSubMenu_zAdjust(&loop, &mainWindow, &userInput, "Z Adjustment");
	Menu tAreaSubMenu_templates(&loop, &mainWindow, &userInput, "Templates");
	Menu tAreaSubMenu_mode_changeMode(&loop, &mainWindow, &userInput, "Select Mode");
	Menu tAreaSubMenu_mode_changeMode_selectSource(&loop, &mainWindow, &userInput, "Select Source");
	Menu tAreaSubMenu_splitVert(&loop, &mainWindow, &userInput, "Vertical Split");
	Menu tAreaSubMenu_splitHoriz(&loop, &mainWindow, &userInput, "Horizontal Split");

	tAreaSubMenu_mode_changeMode.addMenuItem("Ammeter View", [&]() {
        tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
		if (windowManager.getSelectedWindowIndex() != -1) {
			windowManager.getSelectedWindow()->type = WindowManager::windowType::AMMETER;
            for(size_t i = 0; i < teensyController.ammeters_v.size(); i++){
                tAreaSubMenu_mode_changeMode_selectSource.addMenuItem(teensyController.ammeters_v[i].name, [&, i](){
                    teensyController.ammeters_v[i].tField = &windowManager.getSelectedWindow()->tField;
                    teensyController.ammeters_v[i].tFready = true;
                    windowManager.getSelectedWindow()->source = WindowManager::dataSource::SOURCE_NOT_SET;
                    tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
                    tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
                    switch (windowManager.getSelectedWindow()->type) {
						case WindowManager::windowType::NOT_SET:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Mode not set.");
							break;
						}
						case WindowManager::windowType::SERIAL_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Monitor");
							break;
						}
						case WindowManager::windowType::MIDI_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: MIDI Monitor");
							break;
						}
						case WindowManager::windowType::KEYBOARD_INPUT:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Keyboard input");
							break;
						}
						case WindowManager::windowType::AMMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Ammeter");
							break;
						}
						case WindowManager::windowType::VOLTMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Voltmeter");
							break;
						}
						case WindowManager::windowType::MULTIMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Multimter");
							break;
						}
					}
                    tAreaSubMenu_mode.enableMenu();
                }, &mainWindow);
            }
            tAreaSubMenu_mode_changeMode.disableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.enableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.setEscKey([&]() {
				tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
				tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
				tAreaSubMenu_mode.enableMenu();
				});
        }
		else {
			tAreaSubMenu_mode_changeMode.menuItems.at(0).tField.setText("Error. No window selected.");
		}
		}, & mainWindow);

	tAreaSubMenu_mode_changeMode.addMenuItem("Voltmeter View", [&]() {
		if (windowManager.getSelectedWindowIndex() != -1) {
			windowManager.getSelectedWindow()->type = WindowManager::windowType::VOLTMETER;
            for(size_t i = 0; i < teensyController.voltmeters_v.size(); i++){
                tAreaSubMenu_mode_changeMode_selectSource.addMenuItem(teensyController.voltmeters_v[i].name, [&, i](){
                    teensyController.voltmeters_v[i].tField = &windowManager.getSelectedWindow()->tField;
                    teensyController.voltmeters_v[i].tFready = true;
                    windowManager.getSelectedWindow()->source = WindowManager::dataSource::SOURCE_NOT_SET;
                    tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
                    tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
                    switch (windowManager.getSelectedWindow()->type) {
						case WindowManager::windowType::NOT_SET:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Mode not set.");
							break;
						}
						case WindowManager::windowType::SERIAL_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Monitor");
							break;
						}
						case WindowManager::windowType::MIDI_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: MIDI Monitor");
							break;
						}
						case WindowManager::windowType::KEYBOARD_INPUT:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Keyboard input");
							break;
						}
						case WindowManager::windowType::AMMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Ammeter");
							break;
						}
						case WindowManager::windowType::VOLTMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Voltmeter");
							break;
						}
						case WindowManager::windowType::MULTIMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Multimter");
							break;
						}
					}
                    tAreaSubMenu_mode.enableMenu();
                }, &mainWindow);
            }
            tAreaSubMenu_mode_changeMode.disableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.enableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.setEscKey([&]() {
				tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
				tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
				tAreaSubMenu_mode.enableMenu();
				});
		}
		else {
			tAreaSubMenu_mode_changeMode.menuItems.at(0).tField.setText("Error. No window selected.");
		}
		}, & mainWindow);

	tAreaSubMenu_mode_changeMode.addMenuItem("Serial Monitor", [&]() {
        tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
		if (windowManager.getSelectedWindowIndex() != -1) {
			windowManager.getSelectedWindow()->type = WindowManager::windowType::SERIAL_MON;
			for (unsigned int i = 0; i < serialPortManager.getNumberOfPorts(); i++) {
				std::string portName;
				serialPortManager.getPortName(i, portName);
				std::string alias;
				serialPortManager.getPortAlias(i, alias);
				bool available = serialPortManager.getPortAvailable(i);
				if (available) {
					tAreaSubMenu_mode_changeMode_selectSource.addMenuItem((alias.length() > 0) ? alias : portName, [&, i, alias, portName] {
						serialPortManager.setTextFieldForPort(portName, &windowManager.getSelectedWindow()->tField);
						windowManager.getSelectedWindow()->source = WindowManager::dataSource::SERIAL;
						tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
						tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
						switch (windowManager.getSelectedWindow()->type) {
						case WindowManager::windowType::NOT_SET:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Mode not set.");
							break;
						}
						case WindowManager::windowType::SERIAL_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Monitor");
							break;
						}
						case WindowManager::windowType::MIDI_MON:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: MIDI Monitor");
							break;
						}
						case WindowManager::windowType::KEYBOARD_INPUT:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Keyboard input");
							break;
						}
						case WindowManager::windowType::AMMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Ammeter");
							break;
						}
						case WindowManager::windowType::VOLTMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Voltmeter");
							break;
						}
						case WindowManager::windowType::MULTIMETER:
						{
							tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Multimter");
							break;
						}
						}
						tAreaSubMenu_mode.enableMenu();
						}, &mainWindow);
				}
			}
			for (unsigned int i = 0; i < teensyController.serialPorts_v.size(); i++) {
				std::string portName = teensyController.serialPorts_v.at(i).name;
				std::string alias = teensyController.serialPorts_v.at(i).alias;
				if (alias.length() > 0) portName += " - " + alias;
				tAreaSubMenu_mode_changeMode_selectSource.addMenuItem(portName, [&, i, portName] {
					teensyController.serialPorts_v.at(i).tField = &windowManager.getSelectedWindow()->tField;
					teensyController.serialPorts_v.at(i).tFready = true;
					tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
					tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
					switch (windowManager.getSelectedWindow()->type) {
					case WindowManager::windowType::NOT_SET:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Mode not set.");
						break;
					}
					case WindowManager::windowType::SERIAL_MON:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Monitor");
						break;
					}
					case WindowManager::windowType::MIDI_MON:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: MIDI Monitor");
						break;
					}
					case WindowManager::windowType::KEYBOARD_INPUT:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Keyboard input");
						break;
					}
					case WindowManager::windowType::AMMETER:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Ammeter");
						break;
					}
					case WindowManager::windowType::VOLTMETER:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Voltmeter");
						break;
					}
					case WindowManager::windowType::MULTIMETER:
					{
						tAreaSubMenu_mode.menuItems.at(0).tField.setText("Current Mode: Serial Multimter");
						break;
					}
					}
					tAreaSubMenu_mode.enableMenu();
					},&mainWindow);
			}
			tAreaSubMenu_mode_changeMode.disableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.enableMenu();
			tAreaSubMenu_mode_changeMode_selectSource.setEscKey([&]() {
				tAreaSubMenu_mode_changeMode_selectSource.disableMenu();
				tAreaSubMenu_mode_changeMode_selectSource.resetMenuItemList();
				tAreaSubMenu_mode.enableMenu();
				});
		}
		else {
			tAreaSubMenu_mode_changeMode.menuItems.at(0).tField.setText("Error. No window selected.");
		}
		}, & mainWindow);

	for (int i = 2; i < 8; i++) {
		tAreaSubMenu_splitVert.addMenuItem(std::to_string(i), [&,i]() {
			windowManager.splitWindowVert(i);
			tAreaSubMenu_splitVert.disableMenu();
			tAreaSubMenu_Split.enableMenu();
			}, &mainWindow);
	}
	for (int i = 2; i < 11; i++) {
		tAreaSubMenu_splitHoriz.addMenuItem(std::to_string(i), [&,i]() {
			windowManager.splitWindowHoriz(i);
			tAreaSubMenu_splitHoriz.disableMenu();
			tAreaSubMenu_Split.enableMenu();
			}, &mainWindow);
	}

	tAreaMenu.addMenuItem("Area Title", [&]() {
		if (windowManager.getSelectedWindowIndex() != -1) {
			std::string t = windowManager.windows.at(windowManager.getSelectedWindowIndex()).titleEnabled ? "Disable" : "Enable";
			t += " Title";
			tAreaSubMenu_enDisTitle.addMenuItem(t, [&]() {
				if (!windowManager.windows.at(windowManager.getSelectedWindowIndex()).titleEnabled) {
					windowManager.enableWindowTitle("", windowManager.windows.at(windowManager.getSelectedWindowIndex()).id);
					std::string t1 = windowManager.windows.at(windowManager.getSelectedWindowIndex()).titleEnabled ? "Disable" : "Enable";
					t1 += " Title";
					tAreaSubMenu_enDisTitle.menuItems.at(0).tField.setText(t1);
				}
				else {
					windowManager.disableWindowTitle(windowManager.windows.at(windowManager.getSelectedWindowIndex()).id);
					std::string t1 = windowManager.windows.at(windowManager.getSelectedWindowIndex()).titleEnabled ? "Disable" : "Enable";
					t1 += " Title";
					tAreaSubMenu_enDisTitle.menuItems.at(0).tField.setText(t1);
				}
				}, & mainWindow);
		}
		else {
			tAreaSubMenu_enDisTitle.addMenuItem("No window selected.", [&]() {}, &mainWindow);
		}
		tAreaMenu.disableMenu();
		tAreaSubMenu_enDisTitle.enableMenu();
		tAreaSubMenu_enDisTitle.setEscKey([&]() {
			tAreaSubMenu_enDisTitle.disableMenu();
			tAreaSubMenu_enDisTitle.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Mode", [&]() {
		if (windowManager.getSelectedWindowIndex() != -1) {
			switch (windowManager.getSelectedWindow()->type) {
			case WindowManager::windowType::NOT_SET:
			{
				tAreaSubMenu_mode.addMenuItem("Mode not set.", []() {}, & mainWindow);
				break;
			}
			case WindowManager::windowType::SERIAL_MON:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: Serial Monitor", []() {}, & mainWindow);
				break;
			}
			case WindowManager::windowType::MIDI_MON:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: MIDI Monitor", []() {}, &mainWindow);
				break;
			}
			case WindowManager::windowType::KEYBOARD_INPUT:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: Keyboard input", []() {}, &mainWindow);
				break;
			}
			case WindowManager::windowType::AMMETER:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: Ammeter", []() {}, &mainWindow);
				break;
			}
			case WindowManager::windowType::VOLTMETER:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: Voltmeter", []() {}, &mainWindow);
				break;
			}
			case WindowManager::windowType::MULTIMETER:
			{
				tAreaSubMenu_mode.addMenuItem("Current Mode: Serial Multimter", []() {}, &mainWindow);
				break;
			}
			}
			
			tAreaSubMenu_mode.addMenuItem("Set Mode", [&]() {
				tAreaSubMenu_mode_changeMode.enableMenu();
				tAreaSubMenu_mode_changeMode.setEscKey([&]() {
					tAreaSubMenu_mode_changeMode.disableMenu();
					tAreaSubMenu_mode_changeMode.resetMenuItemList();
					tAreaSubMenu_mode.enableMenu();
					});
				}, & mainWindow);
		}
		else {
			tAreaSubMenu_mode.addMenuItem("No window selected.", []() {}, & mainWindow);
		}
		tAreaMenu.disableMenu();
		tAreaSubMenu_mode.enableMenu();
		tAreaSubMenu_mode.setEscKey([&]() {
			tAreaSubMenu_mode.disableMenu();
			tAreaSubMenu_mode.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Split", [&]() {
		tAreaSubMenu_Split.addMenuItem("Vertical", [&]() {
			if (windowManager.getSelectedWindowIndex() != -1) {
				tAreaSubMenu_Split.disableMenu();
				tAreaSubMenu_splitVert.enableMenu();
				tAreaSubMenu_splitVert.setEscKey([&]() {
					tAreaSubMenu_splitVert.disableMenu();
					tAreaSubMenu_Split.enableMenu();
					});
			}
			else {
				tAreaSubMenu_Split.menuItems.at(0).tField.setText("No window selected.");
			}
			}, & mainWindow);
		tAreaSubMenu_Split.addMenuItem("Horizontal", [&]() {
			if (windowManager.getSelectedWindowIndex() != -1) {
				tAreaSubMenu_Split.disableMenu();
				tAreaSubMenu_splitHoriz.enableMenu();
				tAreaSubMenu_splitHoriz.setEscKey([&]() {
					tAreaSubMenu_splitHoriz.disableMenu();
					tAreaSubMenu_Split.enableMenu();
					});
			}
			else {
				tAreaSubMenu_Split.menuItems.at(1).tField.setText("No window selected.");
			}
			}, & mainWindow);
		tAreaMenu.disableMenu();
		tAreaSubMenu_Split.enableMenu();
		tAreaSubMenu_Split.setEscKey([&]() {
			tAreaSubMenu_Split.disableMenu();
			tAreaSubMenu_Split.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Merge", [&]() {
		if (windowManager.firstWindow) {
			if (windowManager.getSelectedWindowIndex() == -1) {
				tAreaSubMenu_Merge.addMenuItem("No window selected.", []() {}, & mainWindow);
			}
			else {
				tAreaSubMenu_Merge.addMenuItem("Press Enter to enable second window select", [&]() {
					windowManager.selectSecondWindow();
					}, & mainWindow);
				tAreaSubMenu_Merge.addMenuItem("Then go back and select the second window", [&]() {
					windowManager.selectSecondWindow();
					}, & mainWindow);
			}
		}
		else {
			if (windowManager.getSecondSelectedWindowIndex() == -1) {
				tAreaSubMenu_Merge.addMenuItem("Second window not selected.", []() {}, & mainWindow);
			}
			else {
				tAreaSubMenu_Merge.addMenuItem("Ready to merge, press Enter", [&]() {
					windowManager.mergeWindows(windowManager.getSelectedWindow()->id, windowManager.getSecondSelectedWindow()->id);
					windowManager.resetSelectSecondWindow();
					}, & mainWindow);
			}
		}
		tAreaMenu.disableMenu();
		tAreaSubMenu_Merge.enableMenu();
		tAreaSubMenu_Merge.setEscKey([&]() {
			tAreaSubMenu_Merge.disableMenu();
			tAreaSubMenu_Merge.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Resize", [&]() {
		tAreaMenu.disableMenu();
		tAreaSubMenu_Resize.enableMenu();
		tAreaSubMenu_Resize.setEscKey([&]() {
			tAreaSubMenu_Resize.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Move", [&]() {
		tAreaMenu.disableMenu();
		tAreaSubMenu_Move.enableMenu();
		tAreaSubMenu_Move.setEscKey([&]() {
			tAreaSubMenu_Move.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Z Adjustment", [&]() {
		tAreaMenu.disableMenu();
		tAreaSubMenu_zAdjust.enableMenu();
		tAreaSubMenu_zAdjust.setEscKey([&]() {
			tAreaSubMenu_zAdjust.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Templates", [&]() {
		tAreaMenu.disableMenu();
		tAreaSubMenu_templates.enableMenu();
		tAreaSubMenu_templates.setEscKey([&]() {
			tAreaSubMenu_templates.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	
	shortcutF5.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(5), true);
			userInput.setKeyDisabled(KEY_F(11), true);
			userInput.setKeyDisabled(KEY_F(12), true);
			tAreaMenu.setEscKey([&]() {
				tAreaMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeListenerByKey(KEY_UP);
				userInput.removeListenerByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(5), false);
				userInput.setKeyDisabled(KEY_F(11), false);
				userInput.setKeyDisabled(KEY_F(12), false);
				});
			tAreaMenu.enableMenu();
			return 1;
			},
			KEY_F(5)),
		KEY_F(5));
#endif
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// End Menu Building Section
	///////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO 


	while (run) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		loop.handleAll(); // handles all the loop events that hanve been registered		
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		if (displayLoopTime) {
			int64_t loopTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
			if (loopTime > maxLoopTime)maxLoopTime = loopTime;
			avgLoopTimeAvergerArr[avgIndex++] = loopTime;
			if (avgIndex >= AVG_LOOP_TIME_NUM)avgIndex = 0;
			uint64_t avgLoopTimeTotal = 0;
			for (int i = 0; i < AVG_LOOP_TIME_NUM; i++) {
				avgLoopTimeTotal += avgLoopTimeAvergerArr[i];
			}
			avgLoopTime = avgLoopTimeTotal / AVG_LOOP_TIME_NUM;
			std::string outputString = "loopTime: " + std::to_string(loopTime);
			outputString += "\tavgLoopTime: " + std::to_string(avgLoopTime);
			outputString += "\tmaxLoopTime: " + std::to_string(maxLoopTime);
			move(0, 0);
			printw(outputString.c_str());
		}
		/*if (std::chrono::duration_cast<std::chrono::seconds>(end - programStartTime).count() > 5 && !emitOnce1) {
			userInput.emitEvent(KEY_F(1));
			emitOnce1 = true;
		}
		if (std::chrono::duration_cast<std::chrono::seconds>(end - programStartTime).count() > 6 && !emitOnce2) {
			userInput.emitEvent(KEY_DOWN);
			emitOnce2 = true;
		}
		if (std::chrono::duration_cast<std::chrono::seconds>(end - programStartTime).count() > 7 && !emitOnce3) {
			userInput.emitEvent(KEY_DOWN);
			emitOnce3 = true;
		}
		if (std::chrono::duration_cast<std::chrono::seconds>(end - programStartTime).count() > 8 && !emitOnce4) {
			userInput.emitEvent(KEY_ENTER);
			emitOnce4 = true;
		}*/

		refresh(); // push the buffer to the display.
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep for ~1ms so that the CPU isn't being hammered all the time.
	}
	mainWindow.clearScreen();
	serialPortManager.closeAllPorts();
	endwin();
	return 0;
}