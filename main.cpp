
#include <time.h>
#include <functional>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include <sys/poll.h>
#include <termios.h>
#include "menu.h"
#include "consoleHandler.h"
#include "main.h"

class loopUpdateHandler {
public:
	loopUpdateHandler() {
		id = 0;
	}

	int add(std::function<int()> f) {
		this->funcs.push_back(f);
		int t = this->id;
		this->id++;
		return t;
	}

	int remove(int id) {
		this->funcs.erase(funcs.begin() + id);
		this->id--;
		return this->funcs.size();
	}

	int call(int id) {
		if (id >= this->id) return 0;
		return this->funcs[id]();
	}

private:
	int id;
	std::vector<std::function<int()>> funcs;
};

int main() {

	struct pollfd fds;
	int ret;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;

	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~ICANON;
	// Setting key, TCSANOW, and term
	tcsetattr(1, TCSANOW, &term);

	consoleHandler mainWindow = consoleHandler();

	loopUpdateHandler loop = loopUpdateHandler();
	loop.add([]() {return 321; });
	int i = loop.add([]() {return 654; });
	loop.add([]() {return 876; });
	loop.add([]() {return 951; });
	
	int t = loop.call(0);
	mainWindow.setCursorPos(5, 20);
	printf("%d", t);
	t = loop.call(1);
	mainWindow.setCursorPos(5, 21);
	printf("%d", t);

	loop.remove(i);
	t = loop.call(0);
	mainWindow.setCursorPos(5, 23);
	printf("%d", t);
	t = loop.call(1);
	mainWindow.setCursorPos(5, 24);
	printf("%d", t);

	t = loop.call(0);
	mainWindow.setCursorPos(5, 26);
	printf("%d", t);
	t = loop.call(1);
	mainWindow.setCursorPos(5, 27);
	printf("%d", t);

	t = loop.call(2);
	mainWindow.setCursorPos(5, 29);
	printf("%d", t);
	t = loop.call(3);
	mainWindow.setCursorPos(5, 30);
	printf("%d", t);

	menu m = menu();
	int p = m.testF();

	/*
	* Setup items:
	* 1. get list of all serial ports and query them to see what sort of device is attached.
	* 2. for all ammeters attached, set them up along with ammeter objects
	* 3.
	* 
	*/

	//mainWindow.setCursorPos(5, 20);
	//printf("5,20");
	//mainWindow.setCursorPos(20, 5);
	//printf("20,5 ");
	//printf(u8"\u2550");
	//printf(u8"\u2551");
	//printf(u8"\u2552");
	//printf(u8"\u2553");
	//mainWindow.clearScreen();

	textField testTextField(1, 1, 30, 1, 0, 7, 2, &mainWindow);
	std::string s = "test Text abcdefghijklmnopqrsdtuvwxyz";
	char* char_array = new char[s.length() + 1];
	strcpy(char_array, s.c_str());
	testTextField.setText(char_array, s.length());
	testTextField.draw();
	//testTextField.toggleBorder();
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//testTextField.draw();
	bool run = true;
	while (run) {
		/*
		* Stuff that needs to go in main loop
		* 1. call draw methods of everything on screen
		* 2. monitor open serial ports for new data
		* 3. monitor stdin for user input
		* 4. call update on methods that register a loop update method
		*/


		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//testTextField.draw();
		ret = poll(&fds, 1, 0);
		if (ret == 1) {
			run = false;
			char a, b, c;
			std::cin >> a;
			std::cin >> b;
			std::cin >> c;
			if (a == 27) {
				std::cout << "a: " << (int)a << "  b: " << (int)b << "  c: " << (int)c;
			}
			else {
				std::string s;
				std::getline(std::cin, s);
				std::cout << "Entered: " << s;
			}
		}
	}
	
	
	//menuItem testMenuItem;
	//testMenuItem.action = [](int a, int b) {
	//	return a;
	//};

	//int x = testMenuItem.action(1, 2);

	// Move cursor to bottom right corner and print...
	printf("%c[%d;%df  width: %d  height: %d", 0x1B, mainWindow.height, mainWindow.width - 23, mainWindow.width, mainWindow.height);
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	//printf("%c[37;40m", 0x1B);
	//printf("%c[2J", 0x1B);
	return 0;
}