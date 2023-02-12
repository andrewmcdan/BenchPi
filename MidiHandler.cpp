#include "MidiHandler.h"

MidiHandler::MidiHandler() {
    try {
        this->inDevs = new RtMidiIn();
        this->outDevs = new RtMidiOut();
    }
    catch (RtMidiError& error) {
        error.printMessage();
        return;
    }

    unsigned int nMidiInPorts = inDevs->getPortCount();
    unsigned int nMidiOutPorts = outDevs->getPortCount();

    std::string portName;

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
    this->midiInDevices.at(portNum).printStyle_ = RAW_BYTES;
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
                switch (this->midiInDevices.at(i).message.at(u).m.size()) {
                case 1: // 1 byte messages
                    this->midiInDevices.at(i).textField_->setText("no1");
                    break;
                case 2: // 2 byte messages
                    this->midiInDevices.at(i).textField_->setText("no2");
                    break;
                case 3: // 3 byte messages
                {
                    unsigned char k = this->midiInDevices.at(i).message.at(u).m[0] & 0xf0;
                    switch (k) {
                    case 0x80: // note off
                        this->midiInDevices.at(i).textField_->setText(std::to_string(this->midiInDevices.at(i).message.at(u).m[0]));
                        break;
                    case 0x90: // note on
                        this->midiInDevices.at(i).textField_->setText(std::to_string(this->midiInDevices.at(i).message.at(u).m[0]));
                        this->midiInDevices.at(i).textField_->setText(" " + std::to_string(this->midiInDevices.at(i).message.at(u).m[1]));
                        this->midiInDevices.at(i).textField_->setText(" " + std::to_string(this->midiInDevices.at(i).message.at(u).m[2]));
                        break;
                    default:
                        this->midiInDevices.at(i).textField_->setText("no " + std::to_string(k) + " \r");
                        break;
                    }
                    break;
                }
                default: // all other length messages
                    this->midiInDevices.at(i).textField_->setText("no3");
                    break;
                }
                this->midiInDevices.at(i).message.erase(this->midiInDevices.at(i).message.begin());
            }
        }
    }
    return 1;
}