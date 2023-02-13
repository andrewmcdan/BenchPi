#pragma once
#include "RtMidi.h"
#include "consoleHandler.h"
#include <sstream>
#include <bitset>
class MidiHandler
{
public:
	// Midi In / Out handler for all connected midi devices.
	MidiHandler();
	// Open a Midi in port. 
	// Params: port number, sysex filter enabled, timing message filter enabled, activesesing filter enabled, 
	// textField to print to on data in
	// return: 1 if successful, -1 if not. 
	int openInPort(int portNum, bool sysex, bool timing, bool activeSense, textField* tF);
	// Open a Midi out port. 
	// Params: port number
	// Return: 1 if successful, -1 if not
	int openOutPort(int portNum);
	// Update() reads data from all enabled midi in ports, write pending data out all enabled Midi out ports,
	// send Midi timing messages out all pors that have it enabled, prints incoming data to text fields. 
	int update();

	enum printStyle {
		RAW_BYTES, // HEX formatted + timestamp
		PRETTY_1, // String formatted
		PRETTY_2,
	};

	// 
	struct midiMes {
		double t; // timestamp of when the message was recevied.
		std::vector<unsigned char> m; // the bytes that were received
	};

	struct midiIn {
		RtMidiIn* RtMidi_device;
		std::string name; // Name of the connected midi device
		bool enabled;
		bool open; // gets set to true once the port is opened.
		std::vector<midiMes>message; // messages with timestamps
		textField* textField_; // The textField that the data should get printed to
		// How the incoming Midi messages shoudl printed.
		// RAW_BYTES: just hex representation of each byte
		// PRETTY: Formatted with strings and stuff
		printStyle printStyle_;
	};

	struct midiOut {
		RtMidiOut* RtMidi_device;
		std::string name;
		bool enabled;
		bool open;
		bool clockEn;
		std::vector<midiMes>message;
	};

	std::vector<midiIn> midiInDevices;
	std::vector<midiOut> midiOutDevices;
};

