#include <stdio.h>
#include <stdarg.h>
#include "main.hpp"

static const int BAR_WIDTH = 10;
static const int MSG_X = 1;
static const int MSG_HEIGHT = Gui::MESSAGE_PANEL_HEIGHT - 1;

Gui::Gui () {
	con = new TCODConsole (engine.screenWidth, MESSAGE_PANEL_HEIGHT);
	left = new TCODConsole (engine.screenWidth, engine.screenHeight);
	right = new TCODConsole (engine.screenWidth, engine.screenHeight);
}

Gui::~Gui () {
	delete con;
	delete left;
	delete right;
	clear ();
}

void Gui::clear () {
	log.clearAndDelete ();
}

void Gui::render () {
	renderLeftPanel ();
	renderRightPanel ();
	renderMessagePanel ();
}

void Gui::renderBar (TCODConsole *target, int x, int y, int width, const char *name,
					 float value, float maxValue, const TCODColor &barColor,
					 const TCODColor &backColor) {
	// fill the background
	target->setDefaultBackground (backColor);
	target->rect (x, y, width, 1, false, TCOD_BKGND_SET);

	int barWidth = (int)(value / maxValue * width);
	if (barWidth > 0) {
		// draw the bar
		target->setDefaultBackground (barColor);
		target->rect (x, y, barWidth, 1, false, TCOD_BKGND_SET);
	}
	// print text on top of the bar
	target->setDefaultForeground (TCODColor::white);
	target->printEx (x + width - 1, y, TCOD_BKGND_NONE, TCOD_RIGHT,
					 "%g/%g", value, maxValue);
}

Gui::Message::Message (const char *text, const TCODColor &col) :
text (_strdup (text)), col (col) {
}

Gui::Message::~Message () {
	free (text);
}
void Gui::renderLeftPanel () {
	// Clear console and set colors
	left->setDefaultBackground (TCODColor::black);
	left->setDefaultForeground (TCODColor::white);
	left->clear ();

	// Draw position fields
	int x = 1;
	int y = 2;

	// Name
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", engine.player->name);
	y++;

	// Level
	PlayerAi *ai = (PlayerAi *)engine.player->ai;
	char levelText[128];
	sprintf_s (levelText, "Level %d", ai->xpLevel);
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", levelText);
	y += 2;

	// draw the health bar
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", "HP");
	renderBar (left, x + strlen ("HP"), y, BAR_WIDTH, "HP", engine.player->destructible->hp,
			   engine.player->destructible->maxHp,
			   TCODColor::lightRed, TCODColor::darkerRed);
	y++;

	// draw the XP bar
	char xpTxt[128];
	sprintf_s (xpTxt, "XP");
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", xpTxt);
	renderBar (left, x + strlen (xpTxt), y, BAR_WIDTH, xpTxt, engine.player->destructible->xp,
			   ai->getNextLevelXp (),
			   TCODColor::lightViolet, TCODColor::darkerViolet);
	y += 2;

	// Stats
	// Power
	char powerTxt[128];
	sprintf_s (powerTxt, "PWR: %d", engine.player->attacker->power);
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", powerTxt);
	y++;

	// Defense
	char defTxt[128];
	sprintf_s (defTxt, "DEF: %d", engine.player->destructible->defense);	
	left->printEx (x, y, TCOD_BKGND_NONE, TCOD_LEFT, "%s", defTxt);
	y++;

	// Mouse look target
	renderMouseLook (1, engine.screenHeight - MESSAGE_PANEL_HEIGHT - 2);

	// Draw a frame
	left->setDefaultForeground (TCODColor::grey);
	left->printFrame (0, 0, LEFT_PANEL_WIDTH, engine.screenHeight - MESSAGE_PANEL_HEIGHT, false, TCOD_BKGND_DARKEN, "Char");

	// Blit the leftPanel
	TCODConsole::root->blit (left, 0, 0, LEFT_PANEL_WIDTH, engine.screenHeight - MESSAGE_PANEL_HEIGHT, TCODConsole::root, 0, 0);
}

void Gui::renderRightPanel () {
	// Clear console and set colors
	right->setDefaultBackground (TCODColor::black);
	right->setDefaultForeground (TCODColor::grey);
	right->clear ();

	// Draw position fields
	int x = 2;
	int y = 2;

	int playerX = engine.player->x;
	int playerY = engine.player->y;
	Tile *t = engine.map->getTile (playerX, playerY);
	char coordText[32];
	sprintf_s (coordText, "X:%d Y:%d", playerX, playerY);
	char ms_winText[32];
	sprintf_s (ms_winText, "X:%d Y:%d", engine.mouse_winX, engine.mouse_winY);
	char ms_mapText[32];
	sprintf_s (ms_mapText, "X:%d Y:%d", engine.mouse_mapX, engine.mouse_mapY);
	char typeText[32];
	sprintf_s (typeText, "%s", t->typeToChar());
	char tempText[32];
	sprintf_s (tempText, "%f", t->temperature);
	char rainText[32];
	sprintf_s (rainText, "%f", t->weather);
	char elvText[32];
	sprintf_s (elvText, "%f", t->variation);

	// Init field to hold how many lines this "well" has
	int well_coord_lines = 0;
	
	right->setDefaultForeground (TCODColor::lightGrey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", "Player");
	y++;

	right->setDefaultForeground (TCODColor::grey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", coordText);
	y++; y++;

	right->setDefaultForeground (TCODColor::lightGrey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", "MS_WIN");
	y++;

	right->setDefaultForeground (TCODColor::grey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", ms_winText);
	y++; y++;

	right->setDefaultForeground (TCODColor::lightGrey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", "MS_MAP");
	y++;

	right->setDefaultForeground (TCODColor::grey);
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", ms_mapText);
	y++;
	// Set field to how many times y was incremented to represent how many lines we wrote
	well_coord_lines = y;
	right->setDefaultForeground (TCODColor::lightGrey);
	right->printFrame (1, 1, RIGHT_PANEL_WIDTH - 2, well_coord_lines, false, TCOD_BKGND_DARKEN);
	
	// Spacer
	y += 2;

	// Init field for "tile" lines
	int well_tile_lines = 0;

	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", typeText);
	y++;
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "TMP: %s", tempText);
	y++;
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "RAIN:%s", rainText);
	y++;
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "ELV: %s", elvText);
	y++;
	// Set field to how many lines this was, minus the previous well.
	well_tile_lines = y - well_coord_lines;

	right->printFrame (1, well_coord_lines + 1, RIGHT_PANEL_WIDTH - 2, well_tile_lines, false, TCOD_BKGND_DARKEN);

	y += 2;


	char offsetText[32];
	sprintf_s (offsetText, "X:%d Y:%d", engine.xOffset, engine.yOffset);
	// Init field for "engine" lines
	int well_engine_lines = 0;

	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", "OFFSET");
	y++;
	right->printEx (x, y, TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", offsetText);
	y++;
	// Set field to how many lines this was, minus the previous well.
	well_engine_lines = y - well_coord_lines - well_tile_lines;

	right->printFrame (1, well_coord_lines + well_tile_lines + 1, RIGHT_PANEL_WIDTH - 2, well_engine_lines, false, TCOD_BKGND_DARKEN);

	// Draw a frame
	right->setDefaultForeground (TCODColor::lightGrey);
	right->printFrame (0, 0, RIGHT_PANEL_WIDTH, engine.screenHeight - MESSAGE_PANEL_HEIGHT, false, TCOD_BKGND_DARKEN, "Info");

	// Blit the rightPanel
	TCODConsole::root->blit (right, 0, 0, 
							 RIGHT_PANEL_WIDTH, 
							 engine.screenHeight - MESSAGE_PANEL_HEIGHT, 
							 TCODConsole::root, 
							 engine.screenWidth - RIGHT_PANEL_WIDTH, 
							 0);
}

void Gui::renderMessagePanel () {
	// clear the GUI console
	con->setDefaultBackground (TCODColor::black);
	con->clear ();

	// draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (Message **it = log.begin (); it != log.end (); it++) {
		Message *message = *it;
		con->setDefaultForeground (message->col * colorCoef);
		con->print (MSG_X, y, message->text);
		y++;
		if (colorCoef < 1.0f) {
			colorCoef += 0.3f;
		}
	}

	con->setDefaultForeground (TCODColor::grey);
	con->printFrame (0, 0, 
					 engine.screenWidth, 
					 MESSAGE_PANEL_HEIGHT, 
					 false, TCOD_BKGND_DARKEN, "Message Log");
	// blit the GUI console on the root console
	TCODConsole::blit (con, 0, 0,
					   engine.screenWidth, 
					   MESSAGE_PANEL_HEIGHT,
					   TCODConsole::root, 
					   0, 
					   engine.screenHeight - MESSAGE_PANEL_HEIGHT);
}
void Gui::renderMouseLook (int x, int y) {

	if (!engine.map->isInFov (engine.mouse_mapX, engine.mouse_mapY)) {
		// if mouse is out of fov, nothing to render
		return;
	}
	char buf[128] = "";
	bool first = true;
	for (Actor **it = engine.map->actors.begin (); it != engine.map->actors.end (); it++) {
		Actor *actor = *it;
		// Translate actor x y?
		int skewX = actor->x;
		int skewY = actor->y;
		//engine.translateToView(skewX, skewY);
		// find actors under the mouse cursor
		if (actor->x == engine.mouse_mapX && actor->y == engine.mouse_mapY) {
			if (!first) {
				strcat_s (buf, ", ");
			} else {
				first = false;
			}
			strcat_s (buf, actor->name);
		}
	}
	// display the list of actors under the mouse cursor
	left->setDefaultForeground (TCODColor::lightGrey);
	left->print (x, y, buf);
}

void Gui::message (const TCODColor &col, const char *text, ...) {
	// build the text
	va_list ap;
	char buf[128];
	va_start (ap, text);
	vsprintf_s (buf, text, ap);
	va_end (ap);

	char *lineBegin = buf;
	char *lineEnd;
	do {
		// make room for the new message
		if (log.size () == MSG_HEIGHT) {
			Message *toRemove = log.get (0);
			log.remove (toRemove);
			delete toRemove;
		}

		// detect end of the line
		lineEnd = strchr (lineBegin, '\n');
		if (lineEnd) {
			*lineEnd = '\0';
		}

		// add a new message to the log
		Message *msg = new Message (lineBegin, col);
		log.push (msg);

		// go to next line
		lineBegin = lineEnd + 1;
	} while (lineEnd);
}


Menu::~Menu () {
	clear ();
}

void Menu::clear () {
	items.clearAndDelete ();
}

void Menu::addItem (MenuItemCode code, const char *label) {
	MenuItem *item = new MenuItem ();
	item->code = code;
	item->label = label;
	items.push (item);
}

const int PAUSE_MENU_WIDTH = 30;
const int PAUSE_MENU_HEIGHT = 15;
Menu::MenuItemCode Menu::pick (DisplayMode mode) {
	int selectedItem = 0;
	int menux, menuy;
	if (mode == PAUSE) {
		menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground (TCODColor (200, 180, 50));
		TCODConsole::root->printFrame (menux, menuy, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT, true,
									   TCOD_BKGND_ALPHA (70), "menu");
		menux += 2;
		menuy += 3;
	} else {
		static TCODImage img ("menu_background1.png");
		img.blit2x (TCODConsole::root, 0, 0);
		menux = 10;
		menuy = TCODConsole::root->getHeight () / 3;
	}

	while (!TCODConsole::isWindowClosed ()) {
		int currentItem = 0;
		for (MenuItem **it = items.begin (); it != items.end (); it++) {
			if (currentItem == selectedItem) {
				TCODConsole::root->setDefaultForeground (TCODColor::lighterOrange);
			} else {
				TCODConsole::root->setDefaultForeground (TCODColor::lightGrey);
			}
			TCODConsole::root->print (menux, menuy + currentItem * 3, (*it)->label);
			currentItem++;
		}
		TCODConsole::flush ();

		// check key presses
		TCOD_key_t key;
		TCODSystem::checkForEvent (TCOD_EVENT_KEY_PRESS, &key, NULL);
		switch (key.vk) {
			case TCODK_UP:
				selectedItem--;
				if (selectedItem < 0) {
					selectedItem = items.size () - 1;
				}
				break;
			case TCODK_DOWN:
				selectedItem = (selectedItem + 1) % items.size ();
				break;
			case TCODK_ENTER:
				return items.get (selectedItem)->code;
			default: break;
		}
	}
	return NONE;
}
