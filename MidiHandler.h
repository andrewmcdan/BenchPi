#pragma once
#include "RtMidi.h"
#include "consoleHandler.h"
class MidiHandler
{
public:
	MidiHandler();
	int openInPort(int portNum, bool sysex, bool timing, bool activeSense, textField* tF);
	int openOutPort(int portNum);
	int update();

	RtMidiIn* inDevs;
	RtMidiOut* outDevs;

	enum printStyle {
		RAW_BYTES,
		PRETTY,
	};

	struct midiMes {
		double t;
		std::vector<unsigned char> m;
	};

	struct midiIn {
		RtMidiIn* RtMidi_device;
		std::string name;
		bool enabled;
		bool open;
		std::vector<midiMes>message;
		textField* textField_;
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

