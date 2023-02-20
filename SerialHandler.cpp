#include "SerialHandler.h"

SerialHandler::SerialHandler() {

	std::string portName = "";
	int ser_port_descriptor = 0;
	textField* temp_tF;

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
				ports.push_back({portName,"",-1,false,false,ser_port_descriptor,tty_temp,temp_tF,BIN});
			}
			// close the port at this point since we don't know which ports the ser wants to use
			close(ser_port_descriptor);
		}
	}
}

int SerialHandler::setPortConfig(std::string name, int baud) {
	return this->setPortConfig(name, baud, 8, false, false, false);
}

int SerialHandler::setPortConfig(std::string name, int baud, int bitPerByte, bool parity, bool stopBits, bool hardwareFlow) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(name) == 0) {
			if (!this->ports.at(i).open) return 0;

			if (parity) this->ports.at(i).tty.c_cflag |= PARENB;
			else this->ports.at(i).tty.c_cflag &= ~PARENB;

			if (stopBits) this->ports.at(i).tty.c_cflag |= CSTOPB;
			else this->ports.at(i).tty.c_cflag &= ~CSTOPB;

			this->ports.at(i).tty.c_cflag &= ~CSIZE;
			switch (bitPerByte) {
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
			this->ports.at(i).baud = baud;
			return 1;
		}
	}
	return -2;
}
int SerialHandler::getPortConfig(int portDescriptor, termios* tty_) {
	if (tcgetattr(portDescriptor, tty_) != 0) {
		return -1;
	}
	return 1; 
}

void SerialHandler::update() {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).available && this->ports.at(i).open && (this->ports.at(i).textField_->getEnabled() || this->ports.at(i).printMode_ == ADDON_CONTROLLER || this->ports.at(i).printMode_ == MULTIMETER)) {
			char read_buf[256];
			int n = read(this->ports.at(i).port_descriptor, &read_buf, sizeof(read_buf));
			switch (this->ports.at(i).printMode_) {
			case ASCII:
				this->ports.at(i).textField_->setText(read_buf, n);
				break;
			case HEX:
				for (int itr = 0; itr < n; itr++) {
					std::ostringstream ss;
					ss << std::hex << (int)read_buf[itr];
					std::string s = "0x" + ss.str() + " ";
					this->ports.at(i).textField_->setText(s);
				}
				break;
			case BIN:
				for (int itr = 0; itr < n; itr++) {
					std::ostringstream ss;
					ss << std::bitset<8>((int)read_buf[itr]);
					std::string s = "0b" + ss.str() + " ";
					this->ports.at(i).textField_->setText(s);
				}
				break;
			case ADDON_CONTROLLER:
				addonCtrlr_->update(read_buf, n);
				break;
			case MULTIMETER:
				// find the meter tha corresponds to the current port and send the data to that multiMeter class object
				for (size_t j = 0; j < this->multiMeters_v.size(); j++) {
					if (this->ports.at(i).name.compare(this->multiMeters_v.at(j).portName) == 0) {
						this->multiMeters_v.at(j).multiMeter_->update(read_buf, n);
					}
				}
				break;
			}
			
		}
	}
	for (unsigned int i = 0; i < this->dataToBeWritten.size(); i++) {
		if (this->dataToBeWritten.at(i).port_descriptor != -1) {
			unsigned char charArray[this->dataToBeWritten.at(i).byteVecArray.size()];
			for(int it = 0; it < this->dataToBeWritten.at(i).byteVecArray.size(); it++){
				charArray[it] = this->dataToBeWritten.at(i).byteVecArray.at(it);
			}
			write(this->dataToBeWritten.at(i).port_descriptor, charArray, this->dataToBeWritten.at(i).byteVecArray.size());
		}
	}
}

int SerialHandler::setTextFieldForPort(std::string portName, textField* tF) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(portName) == 0) {
			this->ports.at(i).textField_ = tF;
			this->ports.at(i).textField_->setClearOnPrint(false);
			return 1;
		}
	}
	return -2;
}

bool SerialHandler::setAddonControllerForData(std::string portName, AddonController* ctrl) {
	this->addonCtrlr_ = ctrl;
	for (int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(portName) == 0) {
			this->ports.at(i).printMode_ = ADDON_CONTROLLER;
			this->ports.at(i).alias = "Addon Controller";
			if (!this->openPort(portName))return false;
			this->setPortConfig(portName, 115200);
			return true;
		}
		else if (this->ports.at(i).printMode_ == ADDON_CONTROLLER) {
			this->ports.at(i).printMode_ = ASCII;
		}
	}
}

bool SerialHandler::setMultiMeterForData(std::string portName, MultiMeter* mtr) {
	for (int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(portName) == 0) {
			this->ports.at(i).printMode_ = MULTIMETER;
			this->ports.at(i).alias = "Multimeter";
			this->multiMeters_v.push_back({ mtr, portName});
			if (!this->openPort(portName)) return false;
			this->setPortConfig(portName, 4800);
			this->setDTR(portName, true);
		}
	}
}

bool SerialHandler::openPort(std::string name) {
	int ser_port_descriptor = open(name.c_str(), O_RDWR);
	// if the descriptor is >= 0 then the port was succesffuly opened.
	if (ser_port_descriptor >= 0) {
		for (unsigned int i = 0; i < this->ports.size(); i++) {
			if (this->ports.at(i).name.compare(name) == 0) {
				this->ports.at(i).port_descriptor = ser_port_descriptor;
				this->ports.at(i).open = true;
				return true;
			}
		}
	}
	else return false;
}

bool SerialHandler::closePort(std::string name) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(name) == 0) {
			close(this->ports.at(i).port_descriptor);
			this->ports.at(i).open = false;
			return true;
		}
	}
	return false;
}

bool SerialHandler::closePort(int portDescriptor) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).port_descriptor == portDescriptor && this->ports.at(i).open) {
			close(this->ports.at(i).port_descriptor);
			this->ports.at(i).open = false;
			return true;
		}
	}
	return false;
}

void SerialHandler::closeAllPorts() {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).open) {
			close(this->ports.at(i).port_descriptor);
			this->ports.at(i).open = false;
		}
	}
}

void SerialHandler::setPortAlias(std::string s, std::string port) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(port) == 0) {
			this->ports.at(i).alias = s;
		}
	}
}

void SerialHandler::setPortAvaialble(int i,bool b) {
	if (i < 0 || i > this->ports.size()) return;
	this->ports.at(i).available = b;
}
int SerialHandler::getNumberOfPorts(){
	return this->ports.size();
}
void SerialHandler::getPortName(int index, std::string& name){
	if (index < 0 || index >= this->ports.size()) {
		name = "";
		return;
	}
	name = this->ports.at(index).name;
}
void SerialHandler::getPortAlias(int index, std::string& alias){
	if (index < 0 || index >= this->ports.size()) {
		alias = "";
		return;
	}
	alias = this->ports.at(index).alias;
}
int SerialHandler::getPortBaud(int index){
	if (index < 0 || index >= this->ports.size()) {
		return -1;
	}
	return this->ports.at(index).baud;
}
bool SerialHandler::getPortAvailable(int index) {
	if (index < 0 || index >= this->ports.size()) {
		return false;
	}
	return this->ports.at(index).available;
}

bool SerialHandler::writeDataToPort(int index, char* data, int len) {
	if (index < 0 || index > this->ports.size())return false;
	dataOut temp;
	temp.port_descriptor = this->ports.at(index).port_descriptor;
	if (!this->ports.at(index).open)temp.port_descriptor = -1;
	for (int i = 0; i < len; i++) {
		temp.byteVecArray.push_back(data[i]);
	}
	this->dataToBeWritten.push_back(temp);
	return true;
}

bool SerialHandler::writeDataToPort(int index, std::string s) {
	if (index < 0 || index > this->ports.size())return false;
	dataOut temp;
	temp.port_descriptor = this->ports.at(index).port_descriptor;
	if (!this->ports.at(index).open)temp.port_descriptor = -1;
	for (int i = 0; i < s.length(); i++) {
		temp.byteVecArray.push_back(s[i]);
	}
	this->dataToBeWritten.push_back(temp);
}

bool SerialHandler::setDTR(std::string portName, bool dtrOn) {
	for (unsigned int i = 0; i < this->ports.size(); i++) {
		if (this->ports.at(i).name.compare(portName) == 0) {
			int DTR_flag = TIOCM_DTR;
			ioctl(this->ports.at(i).port_descriptor,dtrOn?TIOCMBIS:TIOCMBIC,&DTR_flag);
			return true;
		}
	}
	return false;
}

AddonController::AddonController(SerialHandler* serial) {
	this->serial_ = serial;
}

void AddonController::update(char* data, int len) {
	// first put the incoming data into the unprocessed vector
	unsigned char t = 0;
	for (int i = 0; i < len; i++) {
		t = data[i];
		this->unprocessedData.push_back(t);
	}

	/*
	* This section will process incoming data and stor it in the relavent object
	*/

	// check to make sure we at least have a header's worth of data in the vector
	//if (this->unprocessedData.size() > 4) {
	if (0 > 4) {
		unsigned int numIncomingBytes = 0;
		unsigned int startSequence = 0;
		// get the header data out of the vector
		startSequence = (unsigned int)this->unprocessedData.at(0) << 8 | (unsigned int)this->unprocessedData.at(1);
		numIncomingBytes = (unsigned int)this->unprocessedData.at(2) << 8 | (unsigned int)this->unprocessedData.at(3);
		do {
			unsigned char classByte = this->unprocessedData.at(4);
			unsigned char idByte = this->unprocessedData.at(5);
			switch (classByte) {
			case 0xa0: // Ammeter
			{
				// if the id is larger than the sizeof the ammeter vector, add to the vector
				if (idByte > this->ammeters_v.size()) {
					for (unsigned char t_ = this->ammeters_v.size(); t_ <= idByte; t_++) {
						this->ammeters_v.push_back({
							"Ammeter " + t_,
							NULL, // Null becuase the tField needs to be created by the window manager
							false,
							0,0,0
							});
					}
				}

				if (idByte > this->ammeters_v.size() || numIncomingBytes != 14) {
					break;
				}

				this->ammeters_v.at(idByte - 1).milliampsInst = this->unprocessedData.at(6) << (8 * 3) | this->unprocessedData.at(7) << (8 * 2) | this->unprocessedData.at(8) << 8 | this->unprocessedData.at(9);
				this->ammeters_v.at(idByte - 1).milliampsAvg = this->unprocessedData.at(10) << (8 * 3) | this->unprocessedData.at(11) << (8 * 2) | this->unprocessedData.at(12) << 8 | this->unprocessedData.at(13);
				this->ammeters_v.at(idByte - 1).milliampsMax = this->unprocessedData.at(14) << (8 * 3) | this->unprocessedData.at(15) << (8 * 2) | this->unprocessedData.at(16) << 8 | this->unprocessedData.at(17);
				break;
			}
			case 0xa1:
			{
				// if the id is larger than the sizeof the voltmeter vector, add to the vector
				if (idByte > this->voltmeters_v.size()) {
					for (unsigned char t_ = this->voltmeters_v.size(); t_ <= idByte; t_++) {
						this->voltmeters_v.push_back({
							"Voltmeter " + t_,
							NULL, // Null becuase the tField needs to be created by the window manager
							false,
							0,0,0
							});
					}
				}

				if (idByte > this->voltmeters_v.size() || numIncomingBytes != 14) {
					break;
				}

				this->voltmeters_v.at(idByte - 1).millivoltsInst = this->unprocessedData.at(6) << (8 * 3) | this->unprocessedData.at(7) << (8 * 2) | this->unprocessedData.at(8) << 8 | this->unprocessedData.at(9);
				this->voltmeters_v.at(idByte - 1).millivoltsAvg = this->unprocessedData.at(10) << (8 * 3) | this->unprocessedData.at(11) << (8 * 2) | this->unprocessedData.at(12) << 8 | this->unprocessedData.at(13);
				this->voltmeters_v.at(idByte - 1).millivoltsMax = this->unprocessedData.at(14) << (8 * 3) | this->unprocessedData.at(15) << (8 * 2) | this->unprocessedData.at(16) << 8 | this->unprocessedData.at(17);
				break;
			}
			case 0xa2:
			{
				// if the id is larger than the sizeof the serialports vector, add to the vector
				if (idByte > this->serialPorts_v.size()) {
					for (unsigned char t_ = this->serialPorts_v.size(); t_ <= idByte; t_++) {
						this->serialPorts_v.push_back({
							9600,
							NULL, // Null becuase the tField needs to be created by the window manager
							false
							});
					}
				}
				// for all the incoming bytes, add to the datain vector
				for (size_t i = 0; i < numIncomingBytes - 2; i++) {
					this->serialPorts_v.at(idByte - 1).dataIn.push_back(this->unprocessedData.at(5 + i));
				}
				break;
			}
			}
			if (this->unprocessedData.size() > 4) {
				startSequence = (unsigned int)this->unprocessedData.at(0) << 8 | (unsigned int)this->unprocessedData.at(1);
				numIncomingBytes = (unsigned int)this->unprocessedData.at(2) << 8 | (unsigned int)this->unprocessedData.at(3);
			}
			else {
				numIncomingBytes = 0;
				startSequence = 0;
			}
		} while (this->unprocessedData.size() >= numIncomingBytes && startSequence == 0b1010101011001100);
	}

	/*
	* This section updates each textfield using the relavent object's data
	*/

	// ammeters
	for (size_t i = 0; i < this->ammeters_v.size(); i++){
		if (this->ammeters_v.at(i).tFready) {
			std::string temp1 = "Instant: ";
			std::string temp2 = "Maximum: ";
			std::string temp3 = "Average: ";
			temp1 += std::to_string(this->ammeters_v.at(i).milliampsInst);
			temp2 += std::to_string(this->ammeters_v.at(i).milliampsMax);
			temp3 += std::to_string(this->ammeters_v.at(i).milliampsAvg);
			this->ammeters_v.at(i).tField->setTitle(this->ammeters_v.at(i).name);
			this->ammeters_v.at(i).tField->setText(temp1 + "mA\r" + temp2 + "mA\r" + temp3 + "mA");
		}
	}

	// Voltmeters
	for (size_t i = 0; i < this->voltmeters_v.size(); i++) {
		if (this->voltmeters_v.at(i).tFready) {
			std::string temp1 = "Instant: ";
			std::string temp2 = "Maximum: ";
			std::string temp3 = "Average: ";

			if (this->voltmeters_v.at(i).millivoltsInst > 1000)temp1 += std::to_string(this->voltmeters_v.at(i).millivoltsInst / 1000.0) + "V";
			else temp1 += std::to_string(this->voltmeters_v.at(i).millivoltsInst) + "mV";
			if (this->voltmeters_v.at(i).millivoltsMax > 1000)temp2 += std::to_string(this->voltmeters_v.at(i).millivoltsMax / 1000.0) + "V";
			else temp2 += std::to_string(this->voltmeters_v.at(i).millivoltsMax) + "mV";
			if (this->voltmeters_v.at(i).millivoltsAvg > 1000) temp3 += std::to_string(this->voltmeters_v.at(i).millivoltsAvg / 1000.0) + "V";
			else temp3 += std::to_string(this->voltmeters_v.at(i).millivoltsAvg) + "mV";

			this->voltmeters_v.at(i).tField->setTitle(this->voltmeters_v.at(i).name);
			this->voltmeters_v.at(i).tField->setText(temp1 + "\r" + temp2 + "\r" + temp3);
		}
	}

	// Serial ports
	for (size_t i = 0; i < this->serialPorts_v.size(); i++) {
		if (this->serialPorts_v.at(i).tFready) {
			char read_buf[this->serialPorts_v.at(i).dataIn.size() + 1];

			for (int itr = 0; itr < this->serialPorts_v.at(i).dataIn.size(); itr++) {
				read_buf[itr] = this->serialPorts_v.at(i).dataIn.at(itr);
			}

			switch (this->serialPorts_v.at(i).printMode_) {
			case SerialHandler::printMode::ASCII:
				this->serialPorts_v.at(i).tField->setText(read_buf, this->serialPorts_v.at(i).dataIn.size());
				break;
			case SerialHandler::printMode::HEX:
				for (int itr = 0; itr < this->serialPorts_v.at(i).dataIn.size(); itr++) {
					std::ostringstream ss;
					ss << std::hex << (int)read_buf[itr];
					std::string s = "0x" + ss.str() + " ";
					this->serialPorts_v.at(i).tField->setText(s);
				}
				break;
			case SerialHandler::printMode::BIN:
				for (int itr = 0; itr < this->serialPorts_v.at(i).dataIn.size(); itr++) {
					std::ostringstream ss;
					ss << std::bitset<8>((int)read_buf[itr]);
					std::string s = "0b" + ss.str() + " ";
					this->serialPorts_v.at(i).tField->setText(s);
				}
				break;
			}
		}
	}
}

bool AddonController::setPort(std::string n) {
	bool found = false;
	for (int i = 0; i < this->serial_->getNumberOfPorts(); i++) {
		std::string temp;
		this->serial_->getPortName(i, temp);
		if (temp.compare(n) == 0) found = true;
	}
	if (!found) return false;
	this->portName = n;
	if(!this->serial_->setAddonControllerForData(n, this))return false;
	return true;
}

MultiMeter::MultiMeter(SerialHandler* serial, textField* tF) {
	this->tField = tF;
}

MultiMeter::~MultiMeter() {}

void MultiMeter::update(char* data, int len) {

}

bool MultiMeter::setPort(std::string n) {
	bool found = false;
	for (int i = 0; i < this->serial_->getNumberOfPorts(); i++) {
		std::string temp;
		this->serial_->getPortName(i, temp);
		if (temp.compare(n) == 0) found = true;
	}
	if (!found) return false;
	this->portName = n;
	if(!this->serial_->setMultiMeterForData(n, this))return false;
	return true;
}