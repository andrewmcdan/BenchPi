#include "gpio.h"
gpio::gpio() {
	wiringPiSetup();
	pinMode(0, OUTPUT);
	pinMode(1, INPUT);
}