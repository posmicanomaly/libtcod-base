#include "vld.h"
#include "main.hpp"
int testAdjustWidth = 10;
int testAdjustHeight = 6;
const int SCREEN_WIDTH = 80 + 14 + testAdjustWidth;
const int SCREEN_HEIGHT = 50 + 9 + testAdjustHeight;


void dbglog (std::string s) {
	std::cout << s;
}

Engine engine (SCREEN_WIDTH, SCREEN_HEIGHT);

int main () {
	engine.load ();
	while (!TCODConsole::isWindowClosed ()) {
		engine.update ();
		engine.render ();
		TCODConsole::flush ();
	}
	engine.save ();
	return 0;
}
