#include <iostream>
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#include <time.h>
#include <functional>
#include <cstring>

timespec time_;

class console {
public:
	console() {
		struct winsize size;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
		this->width = size.ws_col;
		this->height = size.ws_row;

		clock_gettime(CLOCK_REALTIME, &time_);

		clearScreen();
		return;
	}
	void clearScreen() {
		printf("%c[0;7m", 0x1B); // set black text on white background
		printf("%c[0;0H", 0x1B); // cursor to top left corner
		for (int i = 0; i < this->height; i++) {
			for (int u = 0; u < this->width; u++) {
				printf(" ");
			}
		}
		printf("%c[0;0H", 0x1B); // return cursor to top left corner
	}

	bool setCursorPos(int x, int y) {
		if (x > this->width || y > this->height) { return false; }
		printf("%c[%d;%dH", 0x1B, y, x);
		return true;
	}
	int width;
	int height;
};

class textField {
public:
	textField(int x_coord, int y_coord, int width, int height, int textColor, int bgColor, bool borderEn, console* con) {
		this->border = borderEn;
		this->x = x_coord;
		this->y = y_coord;
		this->fgColor = textColor;
		this->bgColor = bgColor;
		this->mainConsole = con;
		this->scrolling = false;
		this->invert = false;
		this->needsRedraw = true;
		this->border = true;
		this->width = width;
		this->height = height;
		for (int i = 0; i < 256; i++) {
			this->theString[i] = '\0';
		}
		return;
	}

	bool setText(char* s, int len) {
		if (len > 256)return false;
		this->textLength = len;
		for (int i = 0; i < len; i++) {
			this->theString[i] = s[i];
		}
	}

	void draw() {
		this->mainConsole->setCursorPos(this->x, this->y);
		if (this->border) {
			// top left corner
			printf(u8"\u2554");
			for (int i = 0; i < this->width; i++) {
				// crossbar
				printf(u8"\u2550");
			}
			//top right corner
			printf(u8"\u2557");
			this->mainConsole->setCursorPos(this->x, this->y+1);
			// left edge
			printf(u8"\u2551");
			for (int i = 0; i < this->width; i++) {
				// crossbar
				if (this->theString[i] != '\0') printf("%c", this->theString[i]);
				else printf(" ");
			}
			// right edge
			printf(u8"\u2551");
			this->mainConsole->setCursorPos(this->x, this->y + 2);
			// top left corner
			printf(u8"\u255A");
			for (int i = 0; i < this->width; i++) {
				// crossbar
				printf(u8"\u2550");
			}
			//top right corner
			printf(u8"\u255D");
		}
		else {
			for (int i = 0; i < this->width; i++) {
				// crossbar
				if (this->theString[i] != '\0') printf("%c", this->theString[i]);
				else printf(" ");
			}
		}
	}

	int scrollSpeed, width, height, textLength, x, y, fgColor, bgColor;
	bool scrolling, invert, needsRedraw, border;
	char theString[256];
	console* mainConsole;
};

class menu {
public:
	menu() {

	}
};

class menuItem {
public:
	menuItem() {

	}

	std::function<int(int, int)>action;
};

int main() {

	console mainWindow = console();
	mainWindow.setCursorPos(5, 20);
	printf("5,20");
	mainWindow.setCursorPos(20, 5);
	printf("20,5 ");
	printf(u8"\u2550");
	printf(u8"\u2551");
	printf(u8"\u2552");
	printf(u8"\u2553");
	//mainWindow.clearScreen();

	textField testTextField(1, 1, 30, 1, 0, 7, true, &mainWindow);
	std::string s = "test Text abcdefghijklmnopqrsdtuvwxyz";
	char* char_array = new char[s.length() + 1];
	strcpy(char_array, s.c_str());
	testTextField.setText(char_array, s.length());
	testTextField.draw();
	
	menuItem testMenuItem;
	testMenuItem.action = [](int a, int b) {
		return a;
	};

	int x = testMenuItem.action(1, 2);
	printf("%c[%d;%df  width: %d  height: %d", 0x1B, mainWindow.height, mainWindow.width - 23, mainWindow.width, mainWindow.height);

	// hold up the program here until the user inputs something
	//char c;
	//std::cin >> c;
	//printf("%c[37;40m", 0x1B);
	//printf("%c[2J", 0x1B);
	return 0;
}