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
				ports.push_back({portName,-1,false,true,ser_port_descriptor,tty_temp});
			}
			// close the port at this point since we don't know which ports the ser wants to use
			close(ser_port_descriptor);
		}
	}
}

int SerialHandler::setPortConfig(std::string name, int baud, int bitPerBytes, bool parity = false, bool stopBits = false, bool hardwareFlow = false) {
	for (auto i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(name) == 0) {
			if (!this->ports.at(i).open) return 0;
			if (parity) this->ports.at(i).tty.c_cflag |= PARENB;
			else this->ports.at(i).tty.c_cflag &= ~PARENB;

			if (stopBits) this->ports.at(i).tty.c_cflag |= CSTOPB;
			else this->ports.at(i).tty.c_cflag &= ~CSTOPB;

			this->ports.at(i).tty.c_cflag &= ~CSIZE;
			switch (bitPerBytes) {
			case 5:
				this->ports.at(i).tty.c_cflag |= CS5;
				break;
			case 6:
				this->ports.at(i).tty.c_cflag |= CS6;
				break;
			case 7:
				this->ports.at(i).tty.c_cflag |= CS7;
				break;
			default:
				this->ports.at(i).tty.c_cflag |= CS8;
			}

			if (hardwareFlow)this->ports.at(i).tty.c_cflag |= CRTSCTS;
			else this->ports.at(i).tty.c_cflag &= ~CRTSCTS;

			this->ports.at(i).tty.c_cflag |= CREAD | CLOCAL;
			this->ports.at(i).tty.c_cflag &= ~ICANON;
			this->ports.at(i).tty.c_cflag &= ~ECHO;
			this->ports.at(i).tty.c_cflag &= ~ECHOE;
			this->ports.at(i).tty.c_cflag &= ~ECHONL;
			this->ports.at(i).tty.c_cflag &= ~ISIG;

			this->ports.at(i).tty.c_iflag &= ~(IXON | IXOFF | IXANY);
			this->ports.at(i).tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

			this->ports.at(i).tty.c_oflag &= ~OPOST;
			this->ports.at(i).tty.c_oflag &= ~ONLCR;

			this->ports.at(i).tty.c_cc[VTIME] = 0;
			this->ports.at(i).tty.c_cc[VMIN] = 0;

			/*this->ports.at(i).tty.c_cflag &= ~CBAUD;
			this->ports.at(i).tty.c_cflag |= CBAUDEX;*/

			cfsetspeed(&this->ports.at(i).tty, baud);
			if (tcsetattr(this->ports.at(i).port_descriptor, TCSANOW, &tty) != 0) {
				return -1;
			}
			return 1;
		}
	}
	return -2;
	
}
int SerialHandler::getPortConfig(termios* tty_) { return 1; }

void SerialHandler::update() {
	for (auto i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).available && this->ports.at(i).open) {
			char read_buf[256];
			int n = read(this->ports.at(i).port_descriptor, &read_buf, sizeof(read_buf));
			this->ports.at(i).textField_->setText(read_buf, n);
		}
	}
}

int SerialHandler::setTextFieldForPort(std::string portName, textField* tF) {
	for (auto i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(portName) == 0) {
			this->ports.at(i).textField_ = tF;
			return 1;
		}
	}
	return -2;
}

int SerialHandler::openPort(std::string name) {
	int ser_port_descriptor = open(name.c_str(), O_RDWR);
	// if the descriptor is >= 0 then the por was succesffuly opened.
	if (ser_port_descriptor >= 0) {
		for (auto i = 0; i < this->ports.size(); i++) {
			if (this->ports.at(i).name.compare(name) == 0) {
				this->ports.at(i).port_descriptor = ser_port_descriptor;
				this->ports.at(i).open = true;
			}
		}
	}
	return 1;
}

int SerialHandler::closePort(std::string name) {
	return 1;
}

void SerialHandler::closeAllPorts() {

}