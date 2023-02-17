#pragma once
// C library headers
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include "consoleHandler.h"
#include <iostream>
#include <sstream>
#include <bitset>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()


#define NUMBER_OF_SERIAL_PREFIXES 4

// Handles all things related to the serial ports.
class SerialHandler
{
public:
	// When instantiated, this object will check all serial ports to see if they are available for use
	SerialHandler();
	// To be called as often as possible in order to read and send serial data.
	void update();

	// returns 1 if successful, 0 if port isn't open, -1 if there was an error, -2 if port not found
	int setPortConfig(std::string name, int baud, int bitPerByte, bool parity, bool stopBits, bool hardwareFlow);
	int setPortConfig(std::string name, int baud);
	void setPortAlias(std::string s, std::string port);
	int getPortConfig(int portDescriptor, termios* tty_);
	int setTextFieldForPort(std::string portName, textField* tF);
	int openPort(std::string name);
	int closePort(std::string name);
	int closePort(int portDescriptor);
	void closeAllPorts();
	int getNumberOfPorts();
	void getPortName(int index, std::string& name);
	void getPortAlias(int index, std::string& alias);
	int getPortBaud(int index);
	bool getPortAvailable(int index);
	void setPortAvaialble(int i, bool b);
	bool writeDataToPort(int index, char* data, int len);
	bool writeDataToPort(int index, std::string s);

private:
	enum printMode { ASCII, HEX, BIN };

	struct ports_struct {
		std::string name;
		std::string alias;
		int baud;
		bool open;
		bool available;
		int port_descriptor;
		struct termios tty;
		textField* textField_;
		printMode printMode_;
	};

	

	std::vector<ports_struct>ports;

	struct termios tty;

	struct dataOut {
		int port_descriptor;
		std::vector<char>byteVecArray;
	};

	std::vector<dataOut>dataToBeWritten;

	const std::string portPrefixes[NUMBER_OF_SERIAL_PREFIXES] = {
		"/dev/ttyUSB", 
		"/dev/ttyACM", 
		"/dev/ttyS", 
		"/dev/ttyAMA"
		};
};

class AddonController {
public:
	AddonController();
	void update();
	int portFileDescriptor;
	struct termios tty;
};