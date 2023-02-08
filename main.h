#pragma once
#define BORDER_ENABLED 1
#define BORDER_DISABLED 0
#define TIMEPOINT_T std::chrono::time_point<std::chrono::steady_clock>
#define KEY_ALL_ASCII 999999

class loopUpdateHandler {
public:
	// LoopUpdateHandler handles all the functions that are registered to run 
	// during the loop. Use [loopUpdateHanler].addEvent(std::function<int()) to
	// add events to be called every time the loop happens. 
	loopUpdateHandler();

	// Add an event to the loop. Whatever function is added will be called every time
	// the loop happens. 
	int addEvent(std::function<int()> f);

	// Remove an event from the loop. "id" comes from the returned value from addEvent()
	// Returns numbered of registered loop events. 
	int remove(int id);
	
	// Call an individual event registered with the loop.
	int call(int id);

	// Handles all the registered evnets. 
	void handleAll();

private:
	unsigned long id;
	std::vector<std::function<int()>> funcs;
	std::vector<unsigned long> id_s;
	struct event_struct {
		unsigned long id;
		std::function<int()> func;
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
	int addListener(std::function<int(int, std::chrono::_V2::steady_clock::time_point)> f, int key);

	// Remove a listener. Returns number of registered listneres. 
	int remove(unsigned long id);

	// Call a specific listener's associated function.
	int call(unsigned long id, int a);

	// Handle all the keyboard related input stuff. 
	void handleInput();

	// pass a textField object in and normal ASCII characters will be printed in the field
	void printToTextField(textField* tF);

private:
	//std::vector<std::function<int(int, int)>> funcs;
	int loopEventId;
	unsigned long id_index;
	struct events_struct {
		int key;
		unsigned long id;
		std::function<int(int, std::chrono::_V2::steady_clock::time_point) > func;
	};
	std::vector<events_struct> events;
	textField* printToScreen;
	bool printToScreenEn;
};