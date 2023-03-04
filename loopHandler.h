#pragma once
#include <ncurses.h>
#include <vector>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include <ctime>
#include "consoleHandler.fwd.h"


#define TIMEPOINT_T std::chrono::time_point<std::chrono::steady_clock>
#define KEY_ALL_ASCII 999999 
#define KEY_ESC 27
#define KEY_ENTER 10
#define DEBUGKEY
#ifdef DEBUGKEYS
#define KEY_ESC '`'
#define KEY_F(n)  48 + n
#define KEY_DOWN 'd'
#define KEY_UP 'u'
#define KEY_ENTER 'e'
#endif


class loopUpdateHandler {
public:
	// LoopUpdateHandler handles all the functions that are registered to run 
	// during the loop. Use [loopUpdateHanler].addEvent(std::function<int()) to
	// add events to be called every time the loop happens. 
	loopUpdateHandler();

	// Add an event to the loop. Whatever function is added will be called every time
	// the loop happens. 
	unsigned long addEvent(std::function<int()> f);

	unsigned long addEvent(std::function<int()> f, int priority);

	// Remove an event from the loop. "id" comes from the returned value from addEvent()
	// Returns numbered of registered loop events. 
	int remove(unsigned long id);

	// Handles all the registered evnets. 
	void handleAll();

private:
	unsigned long id;
	//std::vector<std::function<int()>> funcs;
	//std::vector<unsigned long> id_s;
	struct event_struct {
		unsigned long id;
		std::function<int()> func;
		int priority;
	};
	std::vector<event_struct> events;
};

class inputHandler {
public:
	// Handle input from the keyboard. The loopUpdateHanlder object must be
	// passed to the inputhandler so that it can register handleInput()
	inputHandler(loopUpdateHandler* loop);

	// Add listener for "key" and register a function to be called when that key
	// is pressed. Reuturns an int id.
	unsigned long addListener(std::function<int(int, std::chrono::_V2::steady_clock::time_point)> f, int key);

	// Remove a listener. Returns number of registered listneres. 
	int removeListener(unsigned long id);

	// Remove any / all listeners for a given key
	int removeListenerByKey(int key);

	// Remove all listeners.
	void resetEvents();

	// Call a specific listener's associated function.
	int call(unsigned long id, int a);

	// Handle all the keyboard related input stuff. 
	void handleInput();

	// pass a textField object in and normal ASCII characters will be printed in the field
	void printToTextField(textField* tF);

	void setKeyDisabled(int key, bool en);

	int emitEvent(int key);

private:
	void removeQueuedIds();
	//std::vector<std::function<int(int, int)>> funcs;
	int loopEventId;
	unsigned long id_index;
	struct events_struct {
		int key;
		unsigned long id;
		std::function<int(int, std::chrono::_V2::steady_clock::time_point) > func;
		bool disabled;
	};
	std::vector<events_struct> events;
	textField* printToScreen;
	bool printToScreenEn;
	std::vector<int>eventsEmitted;
	std::vector<unsigned long>idsToRemove;
};