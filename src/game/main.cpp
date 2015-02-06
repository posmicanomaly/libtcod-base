#include "vld.h"
#include "main.hpp"
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 50;

Engine engine(SCREEN_WIDTH,SCREEN_HEIGHT);

int main() {
	engine.load();
    while ( !TCODConsole::isWindowClosed() ) {
    	engine.update();
    	engine.render();
		TCODConsole::flush();    
    }
    engine.save();
    return 0;
}
