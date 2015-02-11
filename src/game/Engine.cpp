#include <math.h>
#include <stdio.h>
#include "main.hpp"

// 2 is the default radius for WORLD
Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
	player(NULL),map(NULL),fovRadius(2),
	screenWidth(screenWidth),screenHeight(screenHeight),level(1) {
	TCODConsole::setCustomFont("terminal16x16_gs_ro.png", TCOD_FONT_LAYOUT_ASCII_INROW);
    TCODConsole::initRoot(screenWidth,screenHeight,"libtcod C++ tutorial",false);
	// Have to put a dummy map here so when term is called it doesn't crash when trying to access map->actors
	// It will get deleted anyway in term, and we'll get a new one.
	//map = new Map(80, 43);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
    gui = new Gui();
}

void Engine::init() {  
	std::cout << "Engine::init()" << std::endl;
	// Reset level here?
	level = 1;
    map = new Map(80,43, Map::Type::WORLD);
    map->init(true);

	// Does the player alraedy point to something?
	// These are leaking

	player = new Actor(map->stairsUp->x, map->stairsUp->y, '@', "player", TCODColor::white);
	player->destructible = new PlayerDestructible(30, 2, "your cadaver");
	player->attacker = new Attacker(5);
	player->ai = new PlayerAi();
	player->container = new Container(26);
	map->actors.push(player);

    gui->message(TCODColor::red, 
    	"Welcome stranger!\nPrepare to perish in the Tombs of the Ancient Kings.");
    gameStatus=STARTUP;
}

Engine::~Engine() {
	std::cout << "Engine::~Engine()" << std::endl;
	term();
    delete gui;
}

void Engine::term() {
	std::cout << "Engine::term()" << std::endl;
	if (map) {
		map->actors.clearAndDelete();
		if (map) delete map;
	}	
    gui->clear();
}

void Engine::update() {
	if ( gameStatus == STARTUP ) map->computeFov();
   	gameStatus=IDLE;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
    if ( lastKey.vk == TCODK_ESCAPE ) {
    	save();
    	load(true);
    }
    player->update();
    if ( gameStatus == NEW_TURN ) {
	    for (Actor **iterator=map->actors.begin(); iterator != map->actors.end();
	        iterator++) {
	        Actor *actor=*iterator;
	        if ( actor != player ) {
	            actor->update();
	        }
	    }
	}
}

void Engine::render() {
	TCODConsole::root->clear();
	// draw the map
	map->render();
	// draw the actors
	for (Actor **iterator=map->actors.begin();
	    iterator != map->actors.end(); iterator++) {
		Actor *actor=*iterator;
		if ( actor != player 
			&& ((!actor->fovOnly && map->isExplored(actor->x,actor->y))
				|| map->isInFov(actor->x,actor->y)) ) {
	        actor->render();
	    }
	}
	player->render();
	// show the player's stats
	gui->render();
}

void Engine::sendToBack(Actor *actor) {
	map->actors.remove(actor);
	map->actors.insertBefore(actor,0);
}

Actor *Engine::getActor(int x, int y) const {
	for (Actor **iterator=map->actors.begin();
	    iterator != map->actors.end(); iterator++) {
		Actor *actor=*iterator;
		if ( actor->x == x && actor->y ==y && actor->destructible
			&& ! actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
	Actor *closest=NULL;
	float bestDistance=1E6f;
	for (Actor **iterator=map->actors.begin();
	    iterator != map->actors.end(); iterator++) {
		Actor *actor=*iterator;
		if ( actor != player && actor->destructible 
			&& !actor->destructible->isDead() ) {
			float distance=actor->getDistance(x,y);
			if ( distance < bestDistance && ( distance <= range || range == 0.0f ) ) {
				bestDistance=distance;
				closest=actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange) {
	while ( !TCODConsole::isWindowClosed() ) {
		render();
		// highlight the possible range
		for (int cx=0; cx < map->width; cx++) {
			for (int cy=0; cy < map->height; cy++) {
				if ( map->isInFov(cx,cy)
					&& ( maxRange == 0 || player->getDistance(cx,cy) <= maxRange) ) {
					TCODColor col=TCODConsole::root->getCharBackground(cx,cy);
					col = col * 1.2f;
					TCODConsole::root->setCharBackground(cx,cy,col);
				}
			}
		}
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
		if ( map->isInFov(mouse.cx,mouse.cy)
			&& ( maxRange == 0 || player->getDistance(mouse.cx,mouse.cy) <= maxRange )) {
			TCODConsole::root->setCharBackground(mouse.cx,mouse.cy,TCODColor::white);
			if ( mouse.lbutton_pressed ) {
				*x=mouse.cx;
				*y=mouse.cy;
				return true;
			}
		} 
		if (mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) {
			return false;
		}
		TCODConsole::flush();
	}
	return false;
}

void Engine::setFullyExplored() {
	map->setFullyExplored();
}

void Engine::clearMapFiles() {
	int maxLevel = 1;
	while (mapExists(maxLevel)) {
		char fileName[16];
		sprintf_s(fileName, "%s.%d", "save/map", maxLevel);
		TCODSystem::deleteFile(fileName);
		maxLevel++;
	}
}
void Engine::changeLevel(signed int direction) {
	// Let's support only -1 and +1 at this point
	if (direction < -1) {
		std::cout << "Engine::changeLevel() direction too low. Use -1 or 1" << std::endl;
		return;
	}
	else if (direction > 1) {
		std::cout << "Engine::changeLevel() direction too high, use -1, or 1" << std::endl;
		return;
	}
	else if (direction == 0) {
		std::cout << "Engine::changeLevel() direction is zero, use -1 or 1" << std::endl;
		return;
	}

	// Do some bounds checking
	if (level + direction < 1) {
		std::cout << "At the highest level already" << std::endl;
		return;
	}

	// First save the current game, because we're using the engine load fnction,
	// which will unload everything.
	save();
	// Delete all the actors except the player
	for (Actor ** it = map->actors.begin(); it != map->actors.end(); it++) {
		if (*it != player) {
			delete *it;
			it = map->actors.remove(it);
		}
	}
	// Delete the map, this should remove the actors list,
	// but our player will still be accessible through engine->player
	delete map;

	// Increment/Decrement level based on direction
	level += direction;

	// Create a new map
	map = new Map(80, 43, Map::Type::DUNGEON);
	// If the map doesn't exist at the next level, create a new one
	if (!mapExists(level)) {
		map->init(true);
	}
	// Otherwise load the map from file
	else {
		map->load(level);
	}
	// Add the player, so it can be deleted and not leak memory
	map->actors.push(player);
	// Save again, because we'll have to load
	save();
	// Finally load the engine again
	// UPDATE: We can use Engine::loadContinueHelper() instead, it will do what continue does
	// just minus the menu.
	//load(true);
	loadContinueHelper();
	// Determine which stairs to place the player on
	// - Going down, place on up stairs
	// - Going up, place on down stairs
	if (direction > 0) {
		// move player to the up stairs, which lead to the previous level
		player->x = map->stairsUp->x;
		player->y = map->stairsUp->y;
	}
	else {
		// Player on the down stair of the previous level
		player->x = map->stairs->x;
		player->y = map->stairs->y;
	}	

	// Last step, set the engine FoV in case its world
	if (map->type == Map::Type::WORLD) {
		engine.fovRadius = 2;
	}
	else {
		engine.fovRadius = 10;
	}
}

bool Engine::mapExists(int level) {
	char fileName[16];
	sprintf_s(fileName, "%s.%d", "save/map", level);
	return TCODSystem::fileExists(fileName);
}

