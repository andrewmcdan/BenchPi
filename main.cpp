#include "main.h"

int main() {
	std::chrono::steady_clock::time_point programStartTime = std::chrono::steady_clock::now();
	bool emitOnce = false;
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


	// this variable is used in the loop below to exit the program
	bool run = true;
	
	// Init all the handler onjects
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
	

	// Build the quit confirm screen.
	// Init a textfield for confirming quit. 
	textField quitConfirm(0, 0, mainWindow.width - 2, mainWindow.height - 1, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::center);
	// Connect the F4 shortcut with its key listener and instantiate all the events to happen when user responds to confirm
	shortcutF5.setInputListenerIdAndKey(
		userInput.addListener(
			[&userInput,&mainWindow,&loop,&quitConfirm,&run](int a, TIMEPOINT_T t) {
				// when the user pressed F4, clear the screen...
				mainWindow.clearScreen();
				userInput.setKeyDisabled(KEY_F(1), true);
				userInput.setKeyDisabled(KEY_F(2), true);
				userInput.setKeyDisabled(KEY_F(3), true);
				userInput.setKeyDisabled(KEY_F(4), true);
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
					userInput.setKeyDisabled(KEY_F(1), false);
					userInput.setKeyDisabled(KEY_F(2), false);
					userInput.setKeyDisabled(KEY_F(3), false);
					userInput.setKeyDisabled(KEY_F(4), false);
					loop.remove(loopEvent); 
					mainWindow.clearScreen(); 
					return 1; }, 'n');
				userInput.addListener([&, loopEvent](int c2, TIMEPOINT_T t2) { 
					quitConfirm.setEnabled(false); 
					userInput.removeByKey('Y'); 
					userInput.removeByKey('y'); 
					userInput.removeByKey('n'); 
					userInput.removeByKey('N');
					userInput.setKeyDisabled(KEY_F(1), false);
					userInput.setKeyDisabled(KEY_F(2), false);
					userInput.setKeyDisabled(KEY_F(3), false);
					userInput.setKeyDisabled(KEY_F(4), false);
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
	

	Menu loadConfigMenu = Menu(&loop, &mainWindow, &userInput, "Load Configuration From Disk");
	Menu saveConfigMenu = Menu(&loop, &mainWindow, &userInput, "Save Configuration To Disk");
	Menu enDisSerialDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable Serial Devices");
	Menu enDisMidiInDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Input Devices");
	Menu enDisMidiOutDevsMenu = Menu(&loop, &mainWindow, &userInput, "Enable / Disable MIDI Output Devices");
	Menu configAddonControllerMenu = Menu(&loop, &mainWindow, &userInput, "Configure Addon Controller (Teensy)");
	Menu configMultiMeterMenu = Menu(&loop, &mainWindow, &userInput, "Configure Serial MultiMeter");

	Menu addonControllerSubMenu_selectSerialPort = Menu(&loop, &mainWindow, &userInput, "Select Serial Port");
	Menu serialMultiMeterSubMenu_SelectSerialPort = Menu(&loop, &mainWindow, &userInput, "Select Serial Port(s)");

	// build the main menu, F1
	Menu mainMenu = Menu(&loop, &mainWindow, &userInput, "Main Menu");
	Menu serialConfigMenu = Menu(&loop, &mainWindow, &userInput, "Serial Config");
	Menu midiConfigMenu = Menu(&loop, &mainWindow, &userInput, "MIDI Config");
	Menu tAreaMenu = Menu(&loop, &mainWindow, &userInput, "Text Area Options");
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
	mainMenu.addMenuItem("Load Config", []() { /* @TODO: */ }, &mainWindow);
	mainMenu.addMenuItem("Save Config", []() { /* @TODO: */ }, &mainWindow);
	mainMenu.addMenuItem("En/Disable Serial Devices", [&]() {
		enDisSerialDevsMenu.enableMenu();
		enDisSerialDevsMenu.setEscKey([&]() {
			enDisSerialDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, &mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Input Devices", [&]() {
		enDisMidiInDevsMenu.enableMenu();
		enDisMidiInDevsMenu.setEscKey([&]() {
			enDisMidiInDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, &mainWindow);
	mainMenu.addMenuItem("En/Disable MIDI Output Devices", [&]() {
		enDisMidiOutDevsMenu.enableMenu();
		enDisMidiOutDevsMenu.setEscKey([&]() {
			enDisMidiOutDevsMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, &mainWindow);
	mainMenu.addMenuItem("Configure Addon Controller (Teensy)", [&]() {
		configAddonControllerMenu.enableMenu();
		configAddonControllerMenu.setEscKey([&]() {
			configAddonControllerMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, & mainWindow);
	mainMenu.addMenuItem("Configure Serial MultiMeter", [&]() {
		configMultiMeterMenu.enableMenu();
		configMultiMeterMenu.setEscKey([&]() {
			configMultiMeterMenu.disableMenu();
			mainMenu.enableMenu();
			});
		}, &mainWindow);
	configAddonControllerMenu.addMenuItem("Select Serial Port", [&]() {
		addonControllerSubMenu_selectSerialPort.resetMenuItemList();
		for (int i = 0, j = 0; i < serialPortManager.getNumberOfPorts(); i++) {
			std::string name;
			serialPortManager.getPortName(i, name);
			std::string alias;
			serialPortManager.getPortAlias(i, alias);
			bool available = serialPortManager.getPortAvailable(i);
			if (available) {
				addonControllerSubMenu_selectSerialPort.addMenuItem((alias.length() > 0) ? alias : name, [&, j, name]() {
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
		addonControllerSubMenu_selectSerialPort.setEscKey([&]() {
			addonControllerSubMenu_selectSerialPort.disableMenu();
			configAddonControllerMenu.enableMenu();
			});
		}, &mainWindow);
	configMultiMeterMenu.addMenuItem("Select Serial Port(s)", [&]() {
		serialMultiMeterSubMenu_SelectSerialPort.enableMenu();
		serialMultiMeterSubMenu_SelectSerialPort.setEscKey([&]() {
			serialMultiMeterSubMenu_SelectSerialPort.disableMenu();
			configMultiMeterMenu.enableMenu();
			});
		}, &mainWindow);
	std::vector<Menu> serialConfigMenuItems;
	std::vector<std::vector<Menu>> serialConfigSubMenuItems;

	Menu serialPortConfig_openClosePort(&loop, &mainWindow, &userInput, "Open / Close Port");

	Menu tAreaSubMenu_enDisTitle(&loop, &mainWindow, &userInput, "Area Title");
	Menu tAreaSubMenu_mode(&loop, &mainWindow, &userInput, "Mode");
	Menu tAreaSubMenu_Split(&loop, &mainWindow, &userInput, "Split");
	Menu tAreaSubMenu_Move(&loop, &mainWindow, &userInput, "Move");
	Menu tAreaSubMenu_zAdjust(&loop, &mainWindow, &userInput, "Z Adjustment");
	Menu tAreaSubMenu_templates(&loop, &mainWindow, &userInput, "Templates");

	Menu tAreaSubMenu_mode_changeMode(&loop, &mainWindow, &userInput, "Select Mode");
	Menu tAreaSubMenu_mode_changeMode_selectSource(&loop, &mainWindow, &userInput, "Select Source");

	tAreaSubMenu_mode_changeMode.addMenuItem("Serial Monitor", [&]() {
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
		tAreaSubMenu_enDisTitle.enableMenu();
		tAreaSubMenu_enDisTitle.setEscKey([&]() {
			tAreaSubMenu_enDisTitle.disableMenu();
			tAreaSubMenu_enDisTitle.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Mode", [&]() {
		if (windowManager.getSelectedWindowIndex() != -1) {
			//@TODO:
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
		tAreaSubMenu_mode.enableMenu();
		tAreaSubMenu_mode.setEscKey([&]() {
			tAreaSubMenu_mode.disableMenu();
			tAreaSubMenu_mode.resetMenuItemList();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Split", [&]() {
		tAreaSubMenu_Split.enableMenu();
		tAreaSubMenu_Split.setEscKey([&]() {
			tAreaSubMenu_Split.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Move", [&]() {
		tAreaSubMenu_Move.enableMenu();
		tAreaSubMenu_Move.setEscKey([&]() {
			tAreaSubMenu_Move.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Z Adjustment", [&]() {
		tAreaSubMenu_zAdjust.enableMenu();
		tAreaSubMenu_zAdjust.setEscKey([&]() {
			tAreaSubMenu_zAdjust.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	tAreaMenu.addMenuItem("Templates", [&]() {
		tAreaSubMenu_templates.enableMenu();
		tAreaSubMenu_templates.setEscKey([&]() {
			tAreaSubMenu_templates.disableMenu();
			tAreaMenu.enableMenu();
			});
		}, & mainWindow);
	
	

	shortcutF1.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			mainMenu.setEscKey([&]() {
				mainMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeByKey(KEY_UP);
				userInput.removeByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				});
			mainMenu.enableMenu();
			return 1;
		},
		KEY_F(1)),
	KEY_F(1));
	shortcutF2.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			for (int i = 0; i < serialPortManager.getNumberOfPorts(); i++) {
				if (serialPortManager.getPortAvailable(i)) {
					std::string tempString;
					serialPortManager.getPortName(i, tempString);
					std::string portName = tempString;
					std::string tempString2;
					serialPortManager.getPortAlias(i, tempString2);
					if(tempString2.length() > 0)tempString += " - " + tempString2;
					serialConfigMenuItems.push_back(Menu(&loop, &mainWindow, &userInput, tempString));
					serialConfigMenu.addMenuItem(tempString, [&, i, portName]() {
						serialConfigSubMenuItems.push_back(std::vector<Menu>());
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Open / Close Port"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Set / Change Alias"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Baud"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Bits Per Byte"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Parity"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Stop bits"));
						serialConfigSubMenuItems.at(i).push_back(Menu(&loop, &mainWindow, &userInput, "Hardware flow control"));

						serialConfigMenuItems.at(i).addMenuItem("Open / Close Port", [&, i, portName]() {
							if (serialPortManager.isPortOpen(portName)) {
								serialPortConfig_openClosePort.addMenuItem("Port is open", [&]() {}, & mainWindow);
								serialPortConfig_openClosePort.addMenuItem("Close port", [&, portName]() {
									if (!serialPortManager.closePort(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									if (!serialPortManager.isPortOpen(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Port closed");
									}
									else {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									}, & mainWindow);
							}
							else {
								serialPortConfig_openClosePort.addMenuItem("Port is closed (or unavailable)", [&]() {}, & mainWindow);
								serialPortConfig_openClosePort.addMenuItem("Attempt to open port", [&, portName]() {
									if (!serialPortManager.openPort(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}
									if (serialPortManager.isPortOpen(portName)) {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Port opened");
									}
									else {
										serialPortConfig_openClosePort.menuItems.at(1).tField.setText("Error.");
									}

									
									}, & mainWindow);
							}
							serialPortConfig_openClosePort.enableMenu();
							serialPortConfig_openClosePort.setEscKey([&]() {
								serialPortConfig_openClosePort.disableMenu();
								serialPortConfig_openClosePort.resetMenuItemList();
								serialConfigMenuItems.at(i).enableMenu();
								});
							}, & mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Set / Change Alias", [&, i]() {/* @TODO: */return; }, & mainWindow);
						serialConfigMenuItems.at(i).addMenuItem("Baud", [&, i]() {
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("600", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 600) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(0).tField.setText("Successfully set to 600 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(0).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1200", [&, i, portName]() {
								if (serialPortManager.setPortConfig(portName, 1200) == 1) {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(1).tField.setText("Successfully set to 1200 baud");
								}
								else {
									serialConfigSubMenuItems.at(i).at(2).menuItems.at(1).tField.setText("Error setting baud. Is the port open?");
								}
								return;
								}, &mainWindow);
							serialConfigSubMenuItems.at(i).at(2).addMenuItem("1800", [&, i, portName]() {
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
								}, & mainWindow);
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
						serialConfigMenuItems.at(i).setEscKey([&,i]() {
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
					serialConfigMenuItems.at(i).addMenuItem("Set / Change Alias", [&, i]() {/* @TODO: */return; }, & mainWindow);
					serialConfigMenuItems.at(i).addMenuItem("Baud", [&, i]() {
						serialConfigSubMenuItems.at(i).at(2).addMenuItem("600", [&, i, portName]() {
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
						}, & mainWindow);
					serialConfigMenuItems.at(i).setEscKey([&, i]() {
						serialConfigMenuItems.at(i).disableMenu();
						while (serialConfigSubMenuItems.at(i).size() > 0) {
							serialConfigSubMenuItems.at(i).pop_back();
						}
						serialConfigMenu.enableMenu();
						serialConfigMenuItems.at(i).resetMenuItemList();
						});
					serialConfigMenuItems.at(i).enableMenu();
					},&mainWindow);
			}
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			serialConfigMenu.setEscKey([&]() {
				userInput.removeByKey(KEY_UP);
				userInput.removeByKey(KEY_DOWN);
				serialConfigMenu.disableMenu();
				serialConfigMenu.resetMenuItemList();
				while (serialConfigMenuItems.size() > 0) {
					serialConfigMenuItems.pop_back();
				}
				while (serialConfigSubMenuItems.size() > 0){
					serialConfigSubMenuItems.pop_back();
				}
				mainWindow.clearScreen();
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				});
			serialConfigMenu.enableMenu();
			return 1;
			},KEY_F(2)
		),KEY_F(2)
	);
	shortcutF3.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			midiConfigMenu.setEscKey([&]() {
				midiConfigMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeByKey(KEY_UP);
				userInput.removeByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				});
			midiConfigMenu.enableMenu();
			return 1;
			},
			KEY_F(3)),
		KEY_F(3));
	shortcutF4.setInputListenerIdAndKey(
		userInput.addListener([&](int c, TIMEPOINT_T time) {
			userInput.setKeyDisabled(KEY_F(1), true);
			userInput.setKeyDisabled(KEY_F(2), true);
			userInput.setKeyDisabled(KEY_F(3), true);
			userInput.setKeyDisabled(KEY_F(4), true);
			userInput.setKeyDisabled(KEY_F(11), true);
			userInput.setKeyDisabled(KEY_F(12), true);
			tAreaMenu.setEscKey([&]() {
				tAreaMenu.disableMenu();
				mainWindow.clearScreen();
				userInput.removeByKey(KEY_UP);
				userInput.removeByKey(KEY_DOWN);
				userInput.setKeyDisabled(KEY_F(1), false);
				userInput.setKeyDisabled(KEY_F(2), false);
				userInput.setKeyDisabled(KEY_F(3), false);
				userInput.setKeyDisabled(KEY_F(4), false);
				userInput.setKeyDisabled(KEY_F(11), false);
				userInput.setKeyDisabled(KEY_F(12), false);
				});
			tAreaMenu.enableMenu();
			return 1;
			},
			KEY_F(4)),
		KEY_F(4));

	//textField testTextField(0, 0, mainWindow.width / 2, 5, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::textAlignment::left);
	//testTextField.setClearOnPrint(false);
	//for (int i = 0; i < midi.midiInDevices.size(); i++) {
	//	testTextField.setText(midi.midiInDevices.at(i).name + "\r");
	//}
	//testTextField.draw();

	//textField anotherTF(0, 7, mainWindow.width / 3, mainWindow.height - 11, COLOR_WHITE, COLOR_BLACK, BORDER_ENABLED, &mainWindow, textField::textAlignment::left);
	//tf_global = &anotherTF;
	////anotherTF.draw();
	//anotherTF.setClearOnPrint(false);
	//midi.openInPort(2, false, false, false, &anotherTF);
	//try {
	//	if (midi.midiInDevices.size() > 1) midi.midiInDevices.at(2).enabled = true;
	//}
	//catch(std::out_of_range& oor){}
	//loop.addEvent([&midi]() {
	//	midi.update();
	//return 1;
	//	});
	//anotherTF.setBorderColor(COLOR_CYAN);
	//anotherTF.setTitle("test text area");

	// testing printing text from keyboard into textField
	/*testTextField.setClearOnPrint(false);
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
	serialPortManager.setPortConfig(portName, 9600);
	serialPortManager.setTextFieldForPort("/dev/ttyUSB0", &anotherTF);
	

	*/

	while (run) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		loop.handleAll(); // handles all the loop events that hanve been registered
		// last thing to do in the loop is push the buffer to the dispaly.
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		move(0, 0);
		printw(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()).c_str());
		if (std::chrono::duration_cast<std::chrono::seconds>(end - programStartTime).count() > 5 && !emitOnce) {
			userInput.emitEvent(KEY_F(1));
			emitOnce = true;
		}
		refresh();
		// sleep for ~1ms so that the CPU isn't being hammered all the time.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	mainWindow.clearScreen();
	serialPortManager.closeAllPorts();
	endwin();
	return 0;
}


