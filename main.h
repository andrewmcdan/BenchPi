#pragma once
class loopUpdateHandler {
public:
	loopUpdateHandler();
	int addEvent(std::function<int()> f);
	int remove(int id);
	int call(int id);
	void handleAll();

private:
	int id;
	std::vector<std::function<int()>> funcs;
};