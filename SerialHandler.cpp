#include "SerialHandler.h"

SerialHandler::SerialHandler() {

	std::string portName = "";
	int ser_port_descriptor = 0;

	for (int p = 0; p < NUMBER_OF_SERIAL_PREFIXES; p++) {
		for (int i = 0; i < 100; i++) {
			// Create a serial port name from a prefix and a number
			portName = this->portPrefixes[p] + std::to_string(i);
			// attempt to open the serial port.
			ser_port_descriptor = open(portName.c_str(), O_RDWR);
			// if the descriptor is >= 0 then the por was succesffuly opened.
			if (ser_port_descriptor >= 0) {
				// create a temporary termios struct to hold the config of the port as it was when opened
				struct termios tty_temp;
				// get the config of the serial port and print error if there was one
				if (tcgetattr(ser_port_descriptor, &tty_temp) != 0) {
					printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
				}
				// add port info to the "ports" vector
				ports.push_back({portName,-1,false,ser_port_descriptor,tty_temp});
			}
			// close the port at this point since we don't know which ports the ser wants to use
			close(ser_port_descriptor);
		}
	}
}

int SerialHandler::setPortConfig(int baud){}
int SerialHandler::getPortConfig(termios* tty_) {}

void SerialHandler::update() {

}