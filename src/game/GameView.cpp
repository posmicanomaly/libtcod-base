#include "main.hpp"

GameView::GameView () {
	con = new TCODConsole (engine.screenWidth, engine.screenHeight);
}

GameView::~GameView () {
	delete con;
}

void GameView::render () {
	// clear the console
	con->setDefaultBackground (TCODColor::black);
	con->clear ();
	// draw the map
	Map *map = engine.map;
	Actor *player = engine.player;
	map->render (con);

	// draw the actors
	for (Actor **iterator = map->actors.begin ();
		 iterator != map->actors.end (); iterator++) {
		Actor *actor = *iterator;
		if (actor != player
			&& ((!actor->fovOnly && map->isExplored (actor->x, actor->y))
			|| map->isInFov (actor->x, actor->y))) {
			actor->render (con);
		}
	}
	player->render (con);
	// dungeon level
	con->setDefaultForeground (TCODColor::grey);
	const char *title = "Game View";
	if (engine.level == 0) {
		title = "World";
	} else {
		title = engine.map->name.c_str();
	}
	con->printFrame (14, 0, engine.screenWidth - 14, engine.screenHeight- 16, false, TCOD_BKGND_DARKEN, title);
	// blit the GameView console on the root console
	TCODConsole::blit (con, 0, 0, engine.screenWidth, engine.screenHeight,
					   TCODConsole::root, 0, 0);
}