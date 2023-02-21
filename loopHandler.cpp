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

int inputHandler::addListener(std::function<int(int, TIMEPOINT_T)> f, int key) {
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events[i].key == key) {
			this->removeByKey(key);
		}
		if (this->events[i].key == KEY_ALL_ASCII && key > 31 && key < 127) {
			this->removeByKey(KEY_ALL_ASCII);
		}
	}
	this->events.push_back({ key,this->id_index++,f,false });
	return this->events.size();
}

int inputHandler::remove(unsigned long id) {
	auto it = this->events.begin();
	unsigned int i = 0;
	for (; i < this->events.size() && it != this->events.end(); i++, it++) {
		if (this->events.at(i).id == id) {
			this->events.erase(it);
		}
	}
	return this->events.size();
}

int inputHandler::removeByKey(int key) {
	bool found = false;
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events.at(i).key == key) {
			found = true;
			this->remove(this->events.at(i).id);
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

void inputHandler::printToTextField(textField* tF) {

	return;
}

void inputHandler::handleInput() {
	// 
	auto t = std::chrono::steady_clock::now();
	int c = getch();
	for (unsigned int i = 0; i < this->events.size(); i++) {
		if (this->events[i].key == c && !this->events[i].disabled) {
			this->events[i].func(c, t);
		}
		if (this->events[i].key == KEY_ALL_ASCII && c > 31 && c < 127 && !this->events[i].disabled) {
			this->events[i].func(c, t);
		}
	}
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