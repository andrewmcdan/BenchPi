#include "MidiHandler.h"

MidiHandler::MidiHandler() {
    // first we init the RtMidi objects just to get port counts and names
    RtMidiIn* inDevs;
    RtMidiOut* outDevs;
    try {
        inDevs = new RtMidiIn();
        outDevs = new RtMidiOut();
    }
    catch (RtMidiError& error) {
        error.printMessage();
        return;
    }

    unsigned int nMidiInPorts = inDevs->getPortCount();
    unsigned int nMidiOutPorts = outDevs->getPortCount();
    std::string portName;

    // iterate through the all the midi in and out devices and save them in the objects
    for (unsigned int i = 0; i < nMidiInPorts; i++) {
        this->midiInDevices.push_back({new RtMidiIn(), inDevs->getPortName(i), false, false});
    }
    for (unsigned int i = 0; i < nMidiOutPorts; i++) {
        this->midiOutDevices.push_back({new RtMidiOut(), outDevs->getPortName(i), false, false, false });
    }
}

int MidiHandler::openInPort(int portNum, bool sysex, bool timing, bool activeSense, textField* tF) {
    try {
        this->midiInDevices.at(portNum).RtMidi_device->openPort(portNum);
        this->midiInDevices.at(portNum).RtMidi_device->ignoreTypes(sysex, timing, activeSense);
    }
    catch (RtMidiError& error) {
        error.printMessage();
        return -1;
    }
    this->midiInDevices.at(portNum).open = true;
    this->midiInDevices.at(portNum).textField_ = tF;
    //this->midiInDevices.at(portNum).printStyle_ = RAW_BYTES; // Default to printing raw bytes
    this->midiInDevices.at(portNum).printStyle_ = PRETTY_2;
    return 1;
}

int MidiHandler::openOutPort(int portNum) {
    try {
        this->midiOutDevices.at(portNum).RtMidi_device->openPort(portNum);
    }
    catch (RtMidiError& error) {
        error.printMessage();
        return -1;
    }
    this->midiOutDevices.at(portNum).open = true;
    return 1;
}

int MidiHandler::update() {
    for (unsigned int i = 0; i < this->midiInDevices.size(); i++) {
        if (this->midiInDevices.at(i).RtMidi_device->isPortOpen()) {
            int size = 0;
            do {
                std::vector<unsigned char> tempMes;
                double stamp = this->midiInDevices.at(i).RtMidi_device->getMessage(&tempMes);
                size = tempMes.size();
                if (size == 0)break;
                midiMes tM;
                tM.t = stamp;
                for (unsigned int it = 0; it < tempMes.size(); it++) {
                    tM.m.push_back(tempMes.at(it));
                }
                if (tempMes.size() > 0) this->midiInDevices.at(i).message.push_back(tM);
            } while (size > 0);
        }
    }
    for (unsigned int i = 0; i < this->midiInDevices.size(); i++) {
        if (this->midiInDevices.at(i).enabled && this->midiInDevices.at(i).open) {
            unsigned int u = 0;
            while (this->midiInDevices.at(i).message.size() > 0)
            {
                unsigned char k = this->midiInDevices.at(i).message.at(u).m[0] & 0xf0;
                switch (k) {
                case 0x80: // note off
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Note Off Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Note Value: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += " - Velocity: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case PRETTY_2:
                    {
                        std::string s = "Note Off Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += "\rNote Value: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += "\rVelocity: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += "\r \r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(2);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0x90: // note on
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Note On Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Note Value: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += " - Velocity: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case PRETTY_2:
                    {
                        std::string s = "Note On Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += "\rNote Value: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += "\rVelocity: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += "\r \r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(2);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0xa0: // key pressure
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Key Pressure Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Key: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += " - Pressure: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(2);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0xb0: // controller change
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Controller Change - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Controller: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += " - Value: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(2);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0xc0: // controller change
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Program Change - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Program Number: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0xd0: // Channel pressure
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Channel Pressure - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Pressure: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                case 0xe0: // pitch bend
                {
                    switch (this->midiInDevices.at(i).printStyle_) {
                    case PRETTY_1:
                    {
                        std::string s = "Pitch Bend Event - Ch: ";
                        unsigned char ch = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                        s += std::to_string(ch);
                        s += " - Bend LSB: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(1));
                        s += " - Bend MSB: ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).m.at(2));
                        s += '\r';
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    case RAW_BYTES:
                    {
                        std::ostringstream ss;
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(0);
                        std::string s = "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(1);
                        s += "0x" + ss.str() + " ";
                        ss.str("");
                        ss.clear();
                        ss << std::hex << (int)this->midiInDevices.at(i).message.at(u).m.at(2);
                        s += "0x" + ss.str() + " ";
                        s += std::to_string(this->midiInDevices.at(i).message.at(u).t) + "\r";
                        this->midiInDevices.at(i).textField_->setText(s);
                        break;
                    }
                    }
                    break;
                }
                    default:
                        break;
                }
                this->midiInDevices.at(i).message.erase(this->midiInDevices.at(i).message.begin());
            }
        }
    }
    return 1;
}