#include <math.h>
#include <stdio.h>
#include "main.hpp"

static const int WORLD_FOV_RADIUS = 2;
const int MAP_WIDTH = 200;
const int MAP_HEIGHT = 200;

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
player(NULL), map(NULL), fovRadius(WORLD_FOV_RADIUS),
screenWidth(screenWidth), screenHeight(screenHeight), level(0) {
	dbglog("LIBTCOD-BASE\nRoguelike Engine\nJesse Pospisil 2015\n-----\n");
	TCODConsole::setCustomFont("terminal16x16_gs_ro.png", TCOD_FONT_LAYOUT_ASCII_INROW);
	TCODConsole::initRoot(screenWidth, screenHeight, "libtcod-base", false);
	gui = new Gui();
}

void Engine::init() {
	std::cout << "Engine::init()" << std::endl;
	// Reset level here?
	level = 0;
	fovRadius = WORLD_FOV_RADIUS;
	map = new Map(MAP_WIDTH, MAP_HEIGHT, Map::Type::WORLD);
	map->init(true);

	// Hack: player doesn't start on up stairs if its the world map, start them in center;
	int playerStartX, playerStartY;
	if (map->type == Map::Type::WORLD) {
		playerStartX = map->width / 2;
		playerStartY = map->height / 2;

	}
	else {
		playerStartX = map->stairsUp->x;
		playerStartY = map->stairsUp->y;
	}
	player = new Actor(playerStartX, playerStartY, '@', "player", TCODColor::magenta);
	////////////////////////////////////////////////////////////////////////////////////////

	player->destructible = new PlayerDestructible(30, 2, "your cadaver");
	player->attacker = new Attacker(5);
	player->ai = new PlayerAi();
	player->container = new Container(26);
	map->actors.push(player);

	gui->message(TCODColor::red,
		"Libtcod-base: Roguelike engine by Posmicanomaly2");
	gameStatus = STARTUP;
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
	
	if (gameStatus == STARTUP) map->computeFov();
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	if (lastKey.vk == TCODK_ESCAPE) {
		save();
		load(true);
	}
	else if (lastKey.vk == TCODK_PRINTSCREEN) {
		TCODSystem::saveScreenshot(NULL);
	}
	// Set out important mouse information
	translateMouseToView();
	if (mouse.lbutton_pressed) {
		std::cout << "click: " << mouse.cx << ", " << mouse.cy << std::endl;
		std::cout << "to map-> " << mouse_mapX << ", " << mouse_mapY << std::endl;
	}

	player->update();
	if (gameStatus == NEW_TURN) {
		for (Actor **iterator = map->actors.begin(); iterator != map->actors.end();
			iterator++) {
			Actor *actor = *iterator;
			if (actor != player) {
				actor->update();
			}
		}
	}
}
void Engine::translateMouseToView() {
	// Set out important mouse information
	mouse_mapX = mouse.cx + xOffset;
	mouse_mapY = mouse.cy + yOffset;
	mouse_winX = mouse.cx;
	mouse_winY = mouse.cy;
}
void Engine::render() {
	updateCount++;
	if (updateCount > 60 * 3) {
		map->shimmer();
		updateCount = 0;
	}
	TCODConsole::root->clear();

	// Update offsets for "viewport"
	// Can't call this in update because it makes the player jumpy
	xOffset = engine.player->x - VIEW_WIDTH / 2;
	yOffset = engine.player->y - VIEW_HEIGHT / 2;

	// draw the map
	map->render();

	// draw the actors
	for (Actor **iterator = map->actors.begin();
		iterator != map->actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor != player
			&& ((!actor->fovOnly && map->isExplored(actor->x, actor->y))
			|| map->isInFov(actor->x, actor->y))) {
			actor->render();
		}
	}
	player->render();

	// show the player's stats
	gui->render();

	// highlight mouse target
	TCODConsole::root->setCharBackground(mouse_winX, mouse_winY, TCODColor::red);
}

void Engine::translateToView(int &x, int &y) {
	x -= xOffset;
	y -= yOffset;
}

void Engine::sendToBack(Actor *actor) {
	map->actors.remove(actor);
	map->actors.insertBefore(actor, 0);
}

Actor *Engine::getActor(int x, int y) const {
	for (Actor **iterator = map->actors.begin();
		iterator != map->actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor->x == x && actor->y == y && actor->destructible
			&& !actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
	Actor *closest = NULL;
	float bestDistance = 1E6f;
	for (Actor **iterator = map->actors.begin();
		iterator != map->actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor != player && actor->destructible
			&& !actor->destructible->isDead()) {
			float distance = actor->getDistance(x, y);
			if (distance < bestDistance && (distance <= range || range == 0.0f)) {
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange) {
	while (!TCODConsole::isWindowClosed()) {
		render();

		// highlight the possible range
		for (int cx = 0; cx < map->width; cx++) {
			for (int cy = 0; cy < map->height; cy++) {
				if (map->isInFov(cx, cy)
					&& (maxRange == 0 || player->getDistance(cx, cy) <= maxRange)) {
					int skewX = cx;
					int skewY = cy;
					engine.translateToView(skewX, skewY);
					TCODColor col = TCODConsole::root->getCharBackground(skewX, skewY);
					col = col * 1.2f;
					TCODConsole::root->setCharBackground(skewX, skewY, col);
				}
			}
		}
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
		// Set mouse info
		translateMouseToView();

		if (map->isInFov(mouse_mapX, mouse_mapY)
			&& (maxRange == 0 || player->getDistance(mouse_mapX, mouse_mapY) <= maxRange)) {
			TCODConsole::root->setCharBackground(mouse_winX, mouse_winY, TCODColor::white);
			if (mouse.lbutton_pressed) {
				std::cout << "clicked: " << mouse_mapX << ", " << mouse_mapY << std::endl;
				std::cout << "player: " << player->x << ", " << player->y << std::endl;
				*x = mouse_mapX;
				*y = mouse_mapY;
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
	//int maxLevel = 1;
	//while (mapExists(maxLevel)) {
	//	char fileName[16];
	//	sprintf_s(fileName, "%s.%d", "save/map", maxLevel);
	//	TCODSystem::deleteFile(fileName);
	//	maxLevel++;
	//}
	system("exec rm -r save/*");
}
/*
TODO:
Map needs to go back to world map if its going to be level 1, level 0?
The name might need to be tracked by engine for current map, so we don't name it "stairs"
*/
void Engine::changeLevel(signed int direction, Actor *actor) {
	// Let's support only -1 and +1 at this point
	if (direction < -1 || direction > 1 || direction == 0) {
		dbglog("Engine::changeLevel() direction incorrect. Use -1 or 1");
		return;
	}
	// Do some bounds checking
	if (level + direction < 0) {
		dbglog("At the highest level already");
		return;
	}
	std::string nextMapName;

	// Store the old map name in case we end up going back to the world map, so we can place the player
	// on the entrance to the cave.
	std::string oldMapName = map->name;

	// What type to create?
	// Hack: look at the Actor' glyph: * - cave O - town
	Map::Type nextMapType;
	switch (actor->ch) {
	case '*': nextMapType = Map::Type::DUNGEON; break;
	case 'O': nextMapType = Map::Type::TOWN; break;
	default: nextMapType = Map::Type::DUNGEON; break;
	}

	//Store the name of the actor passed in, to determine the correct map file to load

	// If the current map is named "world", then we are going into a cave
	// So the actorName(mapName) must be set to the cave's name.
	if (map->name == "world") {
		nextMapName = actor->name;
	}
	// Otherwise, we are already inside a cave, and are moving around through stairs most likely
	// So keep the same name for the next map.
	else {
		nextMapName = map->name;
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

	// Check if we're going to the world map
	if (level == 0) {
		// Hack, set name to "world"
		nextMapName = "world";
	}

	// Create a new map

	map = new Map(MAP_WIDTH, MAP_HEIGHT, nextMapType);

	// If the map doesn't exist at the next level, create a new one
	if (!mapExists(level, nextMapName)) {
		map->init(true);
		map->name = nextMapName;
	}
	// Otherwise load the map from file
	else {
		map->load(level, nextMapName);
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
		// If we're going to the world, then we need to put the player on the cave they came from
		// This is where oldMapName comes in.
		if (map->name == "world") {
			Actor *caveEntrance;
			for (Actor **iterator = map->actors.begin();
				iterator != map->actors.end();
				iterator++) {
				Actor *actor = *iterator;
				if (actor->name == oldMapName) {
					player->x = actor->x;
					player->y = actor->y;
				}
			}
		}
		else {
			// Player on the down stair of the previous level
			player->x = map->stairs->x;
			player->y = map->stairs->y;
		}
	}

	// Last step, set the engine FoV in case its world
	if (map->type == Map::Type::WORLD) {
		engine.fovRadius = WORLD_FOV_RADIUS;
	}
	if (map->type == Map::Type::TOWN) {
		engine.fovRadius = 0;
	}
	else {
		engine.fovRadius = 10;
	}
	std::cout << "fovRadius = " << engine.fovRadius << std::endl;
	gui->message(TCODColor::green, "You entered %s", map->name.c_str());
}

bool Engine::mapExists(int level, std::string mapName) {
	char fileName[64];
	sprintf_s(fileName, "%s/%s.%d", "save", mapName.c_str(), level);
	return TCODSystem::fileExists(fileName);
}

