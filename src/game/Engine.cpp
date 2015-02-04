#include <math.h>
#include "main.hpp"

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
	player(NULL),map(NULL),fovRadius(10),
	screenWidth(screenWidth),screenHeight(screenHeight),level(1) {
	TCODConsole::setCustomFont("terminal16x16_gs_ro.png", TCOD_FONT_LAYOUT_ASCII_INROW);
    TCODConsole::initRoot(screenWidth,screenHeight,"libtcod C++ tutorial",false);
	// Have to put a dummy map here so when term is called it doesn't crash when trying to access map->actors
	// It will get deleted anyway in term, and we'll get a new one.
	map = new Map(80, 43);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
    gui = new Gui();
}

void Engine::init() {    
    map = new Map(80,43);
    map->init(true);

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
	term();
    delete gui;
}

void Engine::term() {
    map->actors.clearAndDelete();
    if ( map ) delete map;
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

void Engine::nextLevel() {
	level++;
	gui->message(TCODColor::lightViolet,"You take a moment to rest, and recover your strength.");
	player->destructible->heal(player->destructible->maxHp/2);
	gui->message(TCODColor::red,"After a rare moment of peace, you descend\ndeeper into the heart of the dungeon...");
	// TODO: don't delete previous map, save it so we can go back to it
	// delete all actors but player and stairs
	for (Actor **it = map->actors.begin(); it != map->actors.end(); it++) {
		if (*it != player) {
			delete *it;
			it = map->actors.remove(it);
		}
	}
    delete map;
    
    // create a new map
    map = new Map(80,43);
    map->init(true);
	gameStatus=STARTUP;   

	// move player to the up stairs, which lead to the previous level
	player->x = map->stairsUp->x;
	player->y = map->stairsUp->y;
}

void Engine::previousLevel() {
	if (level <= 1) {
		gui->message(TCODColor::red, "Ascension not available yet!");
		return;
	}
	level--;
	gui->message(TCODColor::red, "You ascend the stairs, but the dungeon has shifted!\nNothing is familiar!");
	
	for (Actor **it = map->actors.begin(); it != map->actors.end(); it++) {
		if (*it != player) {
			delete *it;
			it = map->actors.remove(it);
		}
	}
	delete map;
	

	map = new Map(80, 43);
	map->init(true);
	gameStatus = STARTUP;
	// Player is put on the up stairs by the map, since we're going backwards, we want to put them on the down stair of the previous level
	player->x = map->stairs->x;
	player->y = map->stairs->y;
	//// Swap the stairs
	//gui->message(TCODColor::violet, "prev, stairs %d %d, stairsUp %d %d", map->stairs->x, map->stairs->y, map->stairsUp->x, map->stairsUp->y);
	//int tmpx = map->stairs->x;
	//int tmpy = map->stairs->y;
	//map->stairs->x = map->stairsUp->x;
	//map->stairs->y = map->stairsUp->y;
	//map->stairsUp->x = tmpx;
	//map->stairsUp->y = tmpy;
	//gui-> message(TCODColor::violet, "swap, stairs %d %d, stairsUp %d %d", map->stairs->x, map->stairs->y, map->stairsUp->x, map->stairsUp->y);
}
