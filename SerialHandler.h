// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <asm/termbits.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h>
//#include <termios.h>
#include <linux/serial.h>

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
#include <chrono>



#define NUMBER_OF_SERIAL_PREFIXES 4

class AddonController;
class MultiMeter;

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
	int getPortConfig(int portDescriptor, termios2* tty_);
	int setTextFieldForPort(std::string portName, textField* tF);
	bool isPortOpen(std::string name);
	bool openPort(std::string name);
	bool closePort(std::string name);
	bool closePort(int portDescriptor);
	void closeAllPorts();
	int getNumberOfPorts();
	void getPortName(int index, std::string& name);
	void getPortAlias(int index, std::string& alias);
	int getPortBaud(int index);
	bool getPortAvailable(int index);
	void setPortAvaialble(int i, bool b);
	bool writeDataToPort(int index, char* data, int len);
	bool writeDataToPort(int index, std::string s);
	bool setAddonControllerForData(std::string portName, AddonController* ctrl);
	bool setMultiMeterForData(std::string portName, MultiMeter* mtr);
	bool setDTR(std::string portName, bool dtrOn);

	enum printMode { ASCII, HEX, BIN, ADDON_CONTROLLER, MULTIMETER };

private:
	struct ports_struct {
		std::string name;
		std::string alias;
		int baud;
		bool open;
		bool available;
		int port_descriptor;
		struct termios2 tty;
		textField* textField_;
		printMode printMode_;
		bool tFieldReady;
	};
	AddonController* addonCtrlr_;
	struct m_meters {
		MultiMeter* multiMeter_;
		std::string portName;
	};
	std::vector<m_meters> multiMeters_v;
	std::vector<ports_struct>ports;
	//struct termios2 tty;
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
	AddonController(SerialHandler* serial);
	void update(char* data, int len);
	bool setPort(std::string n);
	bool setSerialBaud(int portNum, int baud);
	std::vector<unsigned char>unprocessedData;
	std::string portName;
	struct termios2 tty;
	SerialHandler* serial_;

	struct ammeter {
		std::string name;
		textField* tField;
		bool tFready;
		long milliampsMax;
		long milliampsAvg;
		long milliampsInst;
	};
	std::vector<ammeter> ammeters_v;

	struct voltmeter {
		std::string name;
		textField* tField;
		bool tFready;
		long millivoltsMax;
		long millivoltsAvg;
		long millivoltsInst;
	};
	std::vector<voltmeter> voltmeters_v;

	struct serialPort {
		int baud;
		textField* tField;
		bool tFready;
		std::string alias;
		std::string name;
		std::vector<unsigned char>dataOut;
		std::vector<unsigned char>dataIn;
		SerialHandler::printMode printMode_;
	};
	std::vector<serialPort>serialPorts_v;

};

class MultiMeter {
public:
	MultiMeter(SerialHandler* serial, textField* tF);
	~MultiMeter();
	void update(char* data, int len);
	bool setPort(std::string n);
	SerialHandler* serial_;
	std::string portName;
	textField* tField;
};