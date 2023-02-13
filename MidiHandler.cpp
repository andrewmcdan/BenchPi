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
    if (this->midiInDevices.size() <= portNum) return -1;
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
    this->midiInDevices.at(portNum).printStyle_ = PRETTY_1;
    return 1;
}

int MidiHandler::openOutPort(int portNum) {
    if (this->midiOutDevices.size() <= portNum) return -1;
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
    // this loop iterates through all the midiIn devices and checks for messages from the RtMidi library
    for (unsigned int i = 0; i < this->midiInDevices.size(); i++) {
        // check if the device is open
        if (this->midiInDevices.at(i).RtMidi_device->isPortOpen()) {
            int size = 0;
            do {
                // get the message into tempMes and store the timestamp
                std::vector<unsigned char> tempMes;
                double stamp = this->midiInDevices.at(i).RtMidi_device->getMessage(&tempMes);
                size = tempMes.size();
                if (size == 0)break;
                // create a temporary message object
                midiMes tM;
                tM.t = stamp;
                // add the data bytes to the object
                for (unsigned int it = 0; it < tempMes.size(); it++) {
                    tM.m.push_back(tempMes.at(it));
                }
                // add the message to the vector inside the midiIn device object
                if (tempMes.size() > 0) this->midiInDevices.at(i).message.push_back(tM);
            } while (size > 0);
        }
    }

    // iterate through each of the midi In devices.
    for (unsigned int i = 0; i < this->midiInDevices.size(); i++) {
        // check for the device being enabled and open
        if (this->midiInDevices.at(i).enabled && this->midiInDevices.at(i).open) {
            unsigned int u = 0;

            // the message vector object gets shrunk with each iteratation of this while loop,
            // so it should loop until the vector size is 0
            while (this->midiInDevices.at(i).message.size() > 0)
            {
                // switch on the upper nibble of the first byte
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
                case 0xf0:
                {
                    unsigned char type_ = this->midiInDevices.at(i).message.at(u).m.at(0) & 0x0f;
                    switch (type_) {
                    case 0x0: // SysEx
                    {
                        this->midiInDevices.at(i).textField_->setText("SysEx");
                        break;
                    }
                    case 0x2: // song position
                    {
                        this->midiInDevices.at(i).textField_->setText("Song Posistion");
                        break;
                    }
                    case 0x3: // song select
                    {
                        this->midiInDevices.at(i).textField_->setText("Song Select");
                        break;
                    }
                    case 0x5: // bus select
                    {
                        this->midiInDevices.at(i).textField_->setText("Bus Select");
                        break;
                    }
                    case 0x6: // tune request
                    {
                        this->midiInDevices.at(i).textField_->setText("Tune Request");
                        break;
                    }
                    case 0x7: // SysEx end
                    {
                        this->midiInDevices.at(i).textField_->setText("SysEx End");
                        break;
                    }
                    case 0x8: // Timing Tick
                    {
                        this->midiInDevices.at(i).textField_->setText("Timing tick");
                        break;
                    }
                    case 0xa: // start song
                    {
                        this->midiInDevices.at(i).textField_->setText("Start Song");
                        break;
                    }
                    case 0xb: // Continue song
                    {
                        this->midiInDevices.at(i).textField_->setText("Continue Song");
                        break;
                    }
                    case 0xc: // stop song
                    {
                        this->midiInDevices.at(i).textField_->setText("Stop Song");
                        break;
                    }
                    case 0xe: // active sensing
                    {
                        this->midiInDevices.at(i).textField_->setText("Active Sensing");
                        break;
                    }
                    case 0xf: // System reset
                    {
                        this->midiInDevices.at(i).textField_->setText("System Reset");
                        break;
                    }
                    }
                    break;
                }
                    default:
                        this->midiInDevices.at(i).textField_->setText("Unknown Midi Message.");
                    break;
                }
                this->midiInDevices.at(i).message.erase(this->midiInDevices.at(i).message.begin());
            }
        }
    }


    return 1;
}

void MidiHandler::sendNoteOn(int portNum, int channel, int noteValue, int velocity) {

}