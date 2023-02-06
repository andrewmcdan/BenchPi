#pragma once
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
	int id;
	std::vector<std::function<int()>> funcs;
};

class inputHandler{
public:
	// Handle input from the keyboard. The loopUpdateHanlder object must be
	// passed to the inputhandler so that it can register handleInput()
	inputHandler(loopUpdateHandler* loop);
	
	// Add listener for "key" and register a function to be called when that key
	// is pressed. Reuturns an int id.
	int addListener(std::function<int()> f, int key);

	// Remove a listener. Returns number of registered listneres. 
	int remove(int id);

	// Call a specific listener's associated function.
	int call(int id);

	// Handle all the keyboard related input stuff. 
	void handleInput();

private:
	std::vector<std::function<int()>> funcs;
	int loopEventId;
}