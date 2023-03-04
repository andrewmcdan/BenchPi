#include "loopHandler.h"

loopUpdateHandler::loopUpdateHandler() {
	id = 0;
}

unsigned long loopUpdateHandler::addEvent(std::function<int()> f) {
	this->events.push_back({ ++this->id,f ,0});
	return this->id;
}

unsigned long loopUpdateHandler::addEvent(std::function<int()> f, int priority) {
	this->events.push_back({ ++this->id,f ,priority });
	return this->id;
}

int loopUpdateHandler::remove(unsigned long id) {
	auto itr = events.begin();
	bool found = false;
	for (size_t i = 0; i < this->events.size() && itr != events.end(); i++, itr++) {
		if (this->events.at(i).id == id) {
			this->events.erase(itr);
			found = true;
		}
	}
	return found ? this->events.size() : -1;
}

void loopUpdateHandler::handleAll() {
	int highestPriority = -100;
	int lowestPriority = 100;
	for (size_t i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).priority > highestPriority) highestPriority = this->events.at(i).priority;
		if (this->events.at(i).priority < lowestPriority) lowestPriority = this->events.at(i).priority;
	}
	for (int p = highestPriority; p >= lowestPriority; p--) {
		for (size_t i = 0; i < this->events.size(); i++) {
			if (this->events.at(i).priority == p) {
				this->events.at(i).func();
			}
		}
	}
}





inputHandler::inputHandler(loopUpdateHandler* loop) {
	this->loopEventId = loop->addEvent([this]() {
		this->handleInput();
	return 0;
		});

	this->id_index = 0;
	this->printToScreenEn = false;
}

unsigned long inputHandler::addListener(std::function<int(int, TIMEPOINT_T)> f, int key) {
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events[i].key == key) {
			this->removeListenerByKey(key);
		}
		if (this->events[i].key == KEY_ALL_ASCII && key > 31 && key < 127) {
			this->removeListenerByKey(KEY_ALL_ASCII);
		}
	}
	this->events.push_back({ key,this->id_index,f,false });
	unsigned long temp = this->id_index;
	this->id_index++;
	return temp;
}

int inputHandler::removeListener(unsigned long id) {
	/*auto it = this->events.begin();
	unsigned int i = 0;
	for (; i < this->events.size() && it != this->events.end(); i++, it++) {
		if (this->events.at(i).id == id) {
			this->events.erase(it);
		}
	}*/
	this->idsToRemove.push_back(id);
	return this->events.size() - this->idsToRemove.size();
}

void inputHandler::removeQueuedIds() {
	for (unsigned int k = 0; k < this->idsToRemove.size(); k++) {
		auto it = this->events.begin();
		unsigned int i = 0;
		for (; i < this->events.size() && it != this->events.end(); i++, it++) {
			if (this->events.at(i).id == this->idsToRemove.at(k)) {
				this->events.erase(it);
				i = 0; 
				it = this->events.begin();
			}
		}
	}
	this->idsToRemove.clear();
	return;
}

int inputHandler::removeListenerByKey(int key) {
	bool found = false;
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).key == key) {
			found = true;
			this->removeListener(this->events.at(i).id);
		}
	}
	return found ? this->events.size() : -1;
}

int inputHandler::call(unsigned long id, int a) {
	TIMEPOINT_T t = std::chrono::steady_clock::now();
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).id == id) {
			return this->events.at(i).func(a, t);
		}
	}
	return -1;
}

int inputHandler::emitEvent(int key) {
	this->eventsEmitted.push_back(key);
	return this->events.size();
}

void inputHandler::printToTextField(textField* tF) {

	return;
}

void inputHandler::handleInput() {
	// first, call removeQueuedIds() to get rid of listeners that are queued for removal
	this->removeQueuedIds();
	auto t = std::chrono::steady_clock::now();
	int c = getch();
	size_t eventsSize = this->events.size();
	for (size_t i = 0; i < this->events.size(); i++) {
		// iterate though the emitted events and check against all the registered events. If there's a match, call the function.
		for (size_t i2 = 0; i2 < this->eventsEmitted.size(); i2++) if (this->eventsEmitted.at(i2) == this->events.at(i).key) {
			this->events.at(i).func(this->eventsEmitted.at(i2), t);
			if (eventsSize != this->events.size()) break; // number of event listeners changed during above function call.
		}
	}
	eventsSize = this->events.size();
	this->removeQueuedIds();
	for (size_t i = 0; i < this->events.size(); i++) {
		// check for a match with the key that was pressed
		if (this->events.at(i).key == c && !this->events.at(i).disabled) {
			this->events.at(i).func(c, t);
			if (eventsSize != this->events.size()) break; // number of event listeners changed during above function call.
		}
	}
	eventsSize = this->events.size();
	this->removeQueuedIds();
	for (size_t i = 0; i < this->events.size(); i++) {
		// Check for "all ascii" registered eventss
		if (this->events.at(i).key == KEY_ALL_ASCII && c > 31 && c < 127 && !this->events.at(i).disabled) {
			this->events.at(i).func(c, t);
			if (eventsSize != this->events.size()) break; // number of event listeners changed during above function call.
		}
	}
	// clear the eventsEmitted vector since they will all have been handled. 
	this->eventsEmitted.clear();
	return;
}

void inputHandler::setKeyDisabled(int key, bool en) {
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).key == key) this->events.at(i).disabled = en;
	}
}

void inputHandler::resetEvents() {
	this->events.clear();
}