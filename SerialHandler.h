#pragma once
// C library headers
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include "consoleHandler.h"

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
	SerialHandler::SerialHandler();
	// To be called as often as possible in order to read and send serial data.
	void SerialHandler::update();

	// returns 1 if successful, 0 if port isn't open, -1 if there was an error, -2 if port not found
	int SerialHandler::setPortConfig(std::string name, int baud, int bitPerBytes, bool parity = false, bool stopBits = false, bool hardwareFlow = false);
	int SerialHandler::getPortConfig(termios* tty_);
	int SerialHandler::setTextFieldForPort(std::string portName, textField* tF);
	int SerialHandler::openPort(std::string name);
	int SerialHandler::closePort(std::string name);
	void SerialHandler::closeAllPorts();

private:
	struct ports_struct {
		std::string name;
		int baud;
		bool open;
		bool available;
		int port_descriptor;
		struct termios tty;
		textField* textField_;
	};

	std::vector<ports_struct>ports;

	struct termios tty;

	struct dataOut {
		int port_descriptor;
		std::string aString;
		std::vector<char>byteVecArray;
	};

	const std::string portPrefixes[NUMBER_OF_SERIAL_PREFIXES] = {
		"/dev/ttyUSB", 
		"/dev/ttyACM", 
		"/dev/ttyS", 
		"/dev/ttyAMA"
		};
};