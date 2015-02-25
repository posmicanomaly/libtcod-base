#include <iostream>
#include "main.hpp"
const char *path = "save/game.sav";
void Map::load(TCODZip &zip) {
	std::cout << "map::load(tcodzip &zip) deprecated" << std::endl;
}

void Map::save(TCODZip &zip) {
	std::cout << "map::save(tcodzip &zip) deprecated" << std::endl;
}
void Map::load(int level, std::string mapName) {
	TCODZip zip;
	char fileName[64];
	sprintf_s(fileName, "%s/%s.%d", "save", mapName.c_str(), level);
	zip.loadFromFile(fileName);
	name = zip.getString();
	type = static_cast<Map::Type>(zip.getInt());
	seed=zip.getInt();
	heightMapMin = zip.getFloat();
	heightMapMax = zip.getFloat();
    init(false);
	for (int i=0; i < width*height; i++) {
		tiles[i].explored=zip.getInt();
		// Load the style
		tiles[i].style = zip.getInt();
		// Load the variation
		tiles[i].variation = zip.getInt();
		// Load the temperature
		tiles[i].temperature = zip.getFloat();
		// Load the weather
		tiles[i].weather = zip.getFloat();
		// Load the tile type
		tiles[i].type = static_cast<Tile::Type>(zip.getInt());
	}
	// the stairs
	stairs = new Actor(0, 0, 0, NULL, TCODColor::white);
	stairs->load(zip);
	actors.push(stairs);

	stairsUp = new Actor(0, 0, 0, NULL, TCODColor::white);
	stairsUp->load(zip);
	actors.push(stairsUp);

	// then all other actors
	int nbActors = zip.getInt();
	while (nbActors > 0) {
		Actor *actor = new Actor(0, 0, 0, NULL, TCODColor::white);
		actor->load(zip);
		actors.push(actor);
		nbActors--;
	}
}

void Map::save() {
	char fileName[64];
	sprintf_s(fileName, "%s/%s.%d", "save", name.c_str(), engine.level);
	if (engine.mapExists(engine.level, name.c_str())) {
		TCODSystem::deleteFile(fileName);
	}
	std::cout << "Map::save()" << std::endl;
	std::cout << "fileName: " << fileName << std::endl;
	TCODZip zip;

	zip.putString(name.c_str());
	zip.putInt(type);
	zip.putInt(seed);
	zip.putFloat(heightMapMin);
	zip.putFloat(heightMapMax);

	for (int i=0; i < width*height; i++) {
		zip.putInt(tiles[i].explored);
		// Save the style
		zip.putInt(tiles[i].style);
		// Save the variation
		zip.putInt(tiles[i].variation);
		// Save the temperature
		zip.putFloat(tiles[i].temperature);
		// Save the weather
		zip.putFloat(tiles[i].weather);
		// Save the tile type;
		zip.putInt(tiles[i].type);
	}
	// then the stairs
	stairs->save(zip);
	stairsUp->save(zip);

	// then all the other actors, minus unique things like player, and the stairs
	// POTENTIAL BUG:
	// Originally this was -2, to account for not including the player, or the stairs
	// However, another stairs was added, and then the stairs were included, so we need this at -3
	zip.putInt(actors.size() - 3);
	for (Actor **it = actors.begin(); it != actors.end(); it++) {
		if (*it != engine.player && *it != stairs && *it != stairsUp) {
			(*it)->save(zip);
		}
	}
	
	zip.saveToFile(fileName);
}

void Actor::load(TCODZip &zip) {
	x=zip.getInt();
	y=zip.getInt();
	ch=zip.getInt();
	col=zip.getColor();
	name=_strdup(zip.getString());
	blocks=zip.getInt();
	fovOnly=zip.getInt();
	bool hasAttacker=zip.getInt();
	bool hasDestructible=zip.getInt();
	bool hasAi=zip.getInt();
	bool hasPickable=zip.getInt();
	bool hasContainer=zip.getInt();
	if ( hasAttacker ) {
		attacker = new Attacker(0.0f);
		attacker->load(zip);
	}
	if ( hasDestructible ) {
		destructible = Destructible::create(zip);
	}
	if ( hasAi ) {
		ai = Ai::create(zip);
	}
	if ( hasPickable ) {
		pickable = Pickable::create(zip);
	}
	if ( hasContainer ) {
		container = new Container(0);
		container->load(zip);
	}
}

void Actor::save(TCODZip &zip) {
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(ch);
	zip.putColor(&col);
	zip.putString(name);
	zip.putInt(blocks);
	zip.putInt(fovOnly);
	zip.putInt(attacker != NULL);
	zip.putInt(destructible != NULL);
	zip.putInt(ai != NULL);
	zip.putInt(pickable != NULL);
	zip.putInt(container != NULL);
	if ( attacker ) attacker->save(zip);
	if ( destructible ) destructible->save(zip);
	if ( ai ) ai->save(zip);
	if ( pickable ) pickable->save(zip);
	if ( container ) container->save(zip);
}

void Container::load(TCODZip &zip) {
	size=zip.getInt();
	int nbActors=zip.getInt();
	while ( nbActors > 0 ) {
		Actor *actor=new Actor(0,0,0,NULL,TCODColor::white);
		actor->load(zip);
		inventory.push(actor);
		nbActors--;
	}
}

void Container::save(TCODZip &zip) {
	zip.putInt(size);
	zip.putInt(inventory.size());
	for (Actor **it=inventory.begin(); it != inventory.end(); it++) {
		(*it)->save(zip);
	}
}

void Destructible::load(TCODZip &zip) {
	maxHp=zip.getFloat();
	hp=zip.getFloat();
	defense=zip.getFloat();
	corpseName=_strdup(zip.getString());
	xp=zip.getInt();
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(defense);
	zip.putString(corpseName);
	zip.putInt(xp);
}

void PlayerDestructible::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	Destructible::save(zip);
}

Destructible *Destructible::create(TCODZip &zip) {
	DestructibleType type=(DestructibleType)zip.getInt();
	Destructible *destructible=NULL;
	switch(type) {
		case MONSTER : destructible=new MonsterDestructible(0,0,NULL,0); break;
		case PLAYER : destructible=new PlayerDestructible(0,0,NULL); break;
	}
	destructible->load(zip);
	return destructible;
}

void Attacker::load(TCODZip &zip) {
	power=zip.getFloat();
}

void Attacker::save(TCODZip &zip) {
	zip.putFloat(power);
}

void MonsterAi::load(TCODZip &zip) {
	moveCount=zip.getInt();
}

void MonsterAi::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	zip.putInt(moveCount);
}

void ConfusedMonsterAi::load(TCODZip &zip) {
	nbTurns=zip.getInt();
	oldAi=Ai::create(zip);
}

void ConfusedMonsterAi::save(TCODZip &zip) {
	zip.putInt(CONFUSED_MONSTER);
	zip.putInt(nbTurns);
	oldAi->save(zip);
}

void PlayerAi::load(TCODZip &zip) {
	xpLevel=zip.getInt();
}

void PlayerAi::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	zip.putInt(xpLevel);
}

Ai *Ai::create(TCODZip &zip) {
	AiType type=(AiType)zip.getInt();
	Ai *ai=NULL;
	switch(type) {
		case PLAYER : ai = new PlayerAi(); break;
		case MONSTER : ai = new MonsterAi(); break;
		case CONFUSED_MONSTER : ai = new ConfusedMonsterAi(0,NULL); break;
	}
	ai->load(zip);
	return ai;
}

void Healer::load(TCODZip &zip) {
	amount=zip.getFloat();
}

void Healer::save(TCODZip &zip) {
	zip.putInt(HEALER);
	zip.putFloat(amount);
}

void LightningBolt::load(TCODZip &zip) {
	range=zip.getFloat();
	damage=zip.getFloat();
}

void LightningBolt::save(TCODZip &zip) {
	zip.putInt(LIGHTNING_BOLT);
	zip.putFloat(range);
	zip.putFloat(damage);
}

void Confuser::load(TCODZip &zip) {
	nbTurns=zip.getInt();
	range=zip.getFloat();
}

void Confuser::save(TCODZip &zip) {
	zip.putInt(CONFUSER);
	zip.putInt(nbTurns);
	zip.putFloat(range);
}

void Fireball::save(TCODZip &zip) {
	zip.putInt(FIREBALL);
	zip.putFloat(range);
	zip.putFloat(damage);	
}

Pickable *Pickable::create(TCODZip &zip) {
	PickableType type=(PickableType)zip.getInt();
	Pickable *pickable=NULL;
	switch(type) {
		case HEALER : pickable=new Healer(0); break;
		case LIGHTNING_BOLT : pickable=new LightningBolt(0,0); break;
		case CONFUSER : pickable=new Confuser(0,0); break;
		case FIREBALL : pickable=new Fireball(0,0); break;
	}
	pickable->load(zip);
	return pickable;
}

void Gui::load(TCODZip &zip) {
	int nbMessages=zip.getInt();
	while (nbMessages > 0) {
		const char *text=zip.getString();
		std::cout << "Load: Gui: Text: " << text << std::endl;
		TCODColor col=zip.getColor();
		message(col,text);
		nbMessages--;
	}
}

void Gui::save(TCODZip &zip) {
	zip.putInt(log.size());
	for (Message **it=log.begin(); it != log.end(); it++) {
		std::cout << "Save: Gui: Text: " << (*it)->text << std::endl;
		zip.putString((*it)->text);
		zip.putColor(&(*it)->col);
	}
}

const int SAVEGAME_VERSION=0x1100;
void Engine::load(bool pause) {
	std::cout << "Engine::load()" << std::endl;
	TCODZip zip;
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::NEW_GAME,"New game");
	if ( TCODSystem::fileExists(path)) {
		zip.loadFromFile(path);
		int version = zip.getInt();
		if ( version == SAVEGAME_VERSION ) {
			engine.gui->menu.addItem(Menu::CONTINUE,"Continue");
		}
	}
	engine.gui->menu.addItem(Menu::EXIT,"Exit");

	Menu::MenuItemCode menuItem=engine.gui->menu.pick(
		pause ? Menu::PAUSE : Menu::MAIN);
	if ( menuItem == Menu::EXIT || menuItem == Menu::NONE ) {
		std::cout << "\tMenu::EXIT" << std::endl;
		// Exit or window closed
		exit(0);
	} else if ( menuItem == Menu::NEW_GAME ) {
		// New game
		std::cout << "\tMenu::NEW_GAME" << std::endl;
		clearMapFiles();
		engine.term();
		engine.init();
		
	} else {
		dbglog("\tMenu::CONTINUE\n");
		loadContinueHelper();
	}	
}

void Engine::loadContinueHelper() {
	TCODZip zip;
	if (TCODSystem::fileExists(path)) {
		zip.loadFromFile(path);
		int version = zip.getInt();
		if (version != SAVEGAME_VERSION) {
			dbglog("Engine::loadContinueHelper() SAVEGAME_VERSION mismatch\n");
		}
	}
	// continue a saved game
	engine.term();
	// load the map
	std::string mapName = zip.getString();
	level = zip.getInt();
	int width = zip.getInt();
	int height = zip.getInt();
	map = new Map(width, height, Map::Type::LOADING);
	//TCODZip mapZip;
	//mapZip.loadFromFile("save/map." + level);
	map->load(level, mapName);
	// then the player
	player = new Actor(0, 0, 0, NULL, TCODColor::white);
	engine.map->actors.push(player);
	player->load(zip);


	// finally the message log
	gui->load(zip);
	// to force FOV recomputation
	gameStatus = STARTUP;
}

void Engine::save() {
	dbglog("Engine::save()\n");
	if ( player->destructible->isDead() ) {
		TCODSystem::deleteFile(path);
		clearMapFiles();
	} else {
		TCODZip zip;
		zip.putInt(SAVEGAME_VERSION);
		zip.putString(map->name.c_str());
		zip.putInt(level);
		// save the map first
		zip.putInt(map->width);
		zip.putInt(map->height);
		// Map->save now saves its own actors
		map->save();
		// then the player
		player->save(zip);
		
		
		// finally the message log
		// TODO: Check and create directory if needed
		gui->save(zip);
		zip.saveToFile(path);
	}
}


