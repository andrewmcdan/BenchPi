#pragma once
class menu
{
public:
	menu();
	int testF();
};

class menuItem {
public:
	menuItem();
	std::function<int(int, int)>action;
};