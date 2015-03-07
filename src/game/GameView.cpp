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
	/*static TCODImage img ("menu_background1.png");
	img.blit2x (con, 15, 0);*/
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
	char title[32];
	if (engine.level == 0) {
		strcpy_s (title, "World");
	} else {
		char integer_level[8];
		strcpy_s (title, engine.map->name.c_str ());
		strcat_s (title, " level: ");
		sprintf_s (integer_level, "%d", engine.level);
		strcat_s (title, integer_level);
	}
	con->printFrame (Gui::LEFT_PANEL_WIDTH, 0, engine.screenWidth - Gui::LEFT_PANEL_WIDTH - Gui::RIGHT_PANEL_WIDTH, engine.screenHeight- 16, false, TCOD_BKGND_DARKEN, title);
	// blit the GameView console on the root console
	TCODConsole::blit (con, 0, 0, engine.screenWidth, engine.screenHeight,
					   TCODConsole::root, 0, 0);
}