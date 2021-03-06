#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;

class BspListener : public ITCODBspCallback {
private:
	Map &map; // a map to dig
	int roomNum; // room number
	int lastx, lasty; // center of the last room
public:
	BspListener (Map &map) : map (map), roomNum (0) {}
	bool visitNode (TCODBsp *node, void *userData) {
		if (node->isLeaf ()) {
			int x, y, w, h;
			// dig a room
			bool withActors = (bool)userData;
			w = map.rng->getInt (ROOM_MIN_SIZE, node->w - 2);
			h = map.rng->getInt (ROOM_MIN_SIZE, node->h - 2);
			x = map.rng->getInt (node->x + 1, node->x + node->w - w - 1);
			y = map.rng->getInt (node->y + 1, node->y + node->h - h - 1);
			map.createRoom (roomNum == 0, x, y, x + w - 1, y + h - 1, withActors);
			if (roomNum != 0) {
				// dig a corridor from last room
				map.dig (lastx, lasty, x + w / 2, lasty);
				map.dig (x + w / 2, lasty, x + w / 2, y + h / 2);
			}
			lastx = x + w / 2;
			lasty = y + h / 2;
			roomNum++;
		}
		return true;
	}
};

char *Tile::typeToChar () {
	switch (type) {
		case Type::DESERT: return "desert"; break;
		case Type::FLOOR: return "floor"; break;
		case Type::FOREST: return "forest"; break;
		case Type::GLACIER: return "glacier"; break;
		case Type::GRASS: return "grass"; break;
		case Type::HILL: return "hill"; break;
		case Type::JUNGLE: return "jungle"; break;
		case Type::LAKE: return "lake"; break;
		case Type::MOUNTAIN: return "mountain"; break;
		case Type::OCEAN: return "oceean"; break;
		case Type::PLAIN: return "plain"; break;
		case Type::RIVER: return "river"; break;
		case Type::SHORE: return "shore"; break;
		case Type::SWAMP: return "swamp"; break;
		case Type::TREE: return "tree"; break;
		case Type::TUNDRA: return "tundra"; break;
		case Type::WALL: return "wall"; break;
		case Type::WATER_DEEP: return "deep water"; break;
		case Type::WATER_SHALLOW: return "shallow water"; break;
		default: return "no typeToChar"; break;
	}
}

Map::Map (int width, int height, Type type)
: width (width), height (height), type (type), name ("default map") {
	std::cout << "Map()" << std::endl;
	seed = TCODRandom::getInstance ()->getInt (0, 0x7FFFFFFF);
	// Down stairs
	stairs = new Actor (0, 0, '>', "stairs", TCODColor::white);
	stairs->blocks = false;
	stairs->fovOnly = false;
	actors.push (stairs);
	// Up stairs
	stairsUp = new Actor (0, 0, '<', "stairs", TCODColor::white);
	stairsUp->blocks = false;
	stairsUp->fovOnly = false;
	actors.push (stairsUp);
}

void Map::init (bool withActors) {
	rng = new TCODRandom (seed, TCOD_RNG_CMWC);
	tiles = new Tile[width*height];
	// Give all tiles a variation to make them appear slightly different from each other
	// (more visually appealing, no affect on gameplay)
	for (int i = 0; i < width * height; i++) {
		tiles[i].variation = rng->getInt (10, 20);
	}
	/////////////////////////////////////////////////////////////////////////////////////
	map = new TCODMap (width, height);
	// By default set the properties of all the tiles to transparent and walkable
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			map->setProperties (x, y, true, true);
		}
	}

	/*
	Not sure how I want to refactor the dungeon generation yet
	So for now, MapFactory will take care of all the new generation additions
	*/

	// Dungeon
	if (type == Type::DUNGEON) {
		TCODBsp bsp (0, 0, width, height);
		bsp.splitRecursive (rng, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
		BspListener listener (*this);
		bsp.traverseInvertedLevelOrder (&listener, (void *)withActors);
		MapFactory::makeDungeonMap (*this);
	}

	// World
	else if (type == Type::WORLD) {
		MapFactory::makeWorldMap (*this);
		name = "world";
	}

	// Town
	else if (type == Type::TOWN) {
		MapFactory::makeTownMap (*this);
	}
}

Map::~Map () {
	std::cout << " ~Map()" << std::endl;
	delete rng;
	delete[] tiles;
	delete map;
}

/**
Alternates style of certain water tiles to provide a movement effect
**/
void Map::shimmer () {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Tile *t = &tiles[x + y * width];
			if (t->type == Tile::Type::OCEAN || t->type == Tile::Type::WATER_SHALLOW) {
				int chance = rng->getInt (0, 100);
				if (chance < 10) {
					if (t->style == 0) {
						t->style++;
					} else {
						t->style--;
					}
				}
			}
		}
	}
}

void Map::dig (int x1, int y1, int x2, int y2) {
	if (x2 < x1) {
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1) {
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	for (int tilex = x1; tilex <= x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {
			map->setProperties (tilex, tiley, true, true);
			// Set the tile type
			tiles[tilex + tiley * width].type = Tile::Type::FLOOR;
		}
	}
}

void Map::addMonster (int x, int y) {
	TCODRandom *rng = TCODRandom::getInstance ();
	if (rng->getInt (0, 100) < 80) {
		// create an orc
		Actor *orc = new Actor (x, y, 'o', "orc",
								TCODColor::desaturatedGreen);
		orc->destructible = new MonsterDestructible (10, 0, "dead orc", 35);
		orc->attacker = new Attacker (3);
		orc->ai = new MonsterAi ();
		actors.push (orc);
	} else {
		// create a troll
		Actor *troll = new Actor (x, y, 'T', "troll",
								  TCODColor::darkerGreen);
		troll->destructible = new MonsterDestructible (16, 1, "troll carcass", 100);
		troll->attacker = new Attacker (4);
		troll->ai = new MonsterAi ();
		actors.push (troll);
	}
}

void Map::addItem (int x, int y) {
	TCODRandom *rng = TCODRandom::getInstance ();
	int dice = rng->getInt (0, 100);
	if (dice < 10) {
		// create a health potion
		Actor *healthPotion = new Actor (x, y, '!', "health potion",
										 TCODColor::violet);
		healthPotion->blocks = false;
		healthPotion->pickable = new Healer (4);
		actors.push (healthPotion);
	} else if (dice < 10 + 10) {
		// create a scroll of lightning bolt 
		Actor *scrollOfLightningBolt = new Actor (x, y, '#', "scroll of lightning bolt",
												  TCODColor::lightYellow);
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->pickable = new LightningBolt (5, 20);
		actors.push (scrollOfLightningBolt);
	} else if (dice < 70 + 10 + 10) {
		// create a scroll of fireball
		Actor *scrollOfFireball = new Actor (x, y, '#', "scroll of fireball",
											 TCODColor::lightYellow);
		scrollOfFireball->blocks = false;
		scrollOfFireball->pickable = new Fireball (3, 12);
		actors.push (scrollOfFireball);
	} else {
		// create a scroll of confusion
		Actor *scrollOfConfusion = new Actor (x, y, '#', "scroll of confusion",
											  TCODColor::lightYellow);
		scrollOfConfusion->blocks = false;
		scrollOfConfusion->pickable = new Confuser (10, 8);
		actors.push (scrollOfConfusion);
	}
}

void Map::createRoom (bool first, int x1, int y1, int x2, int y2, bool withActors) {
	dig (x1, y1, x2, y2);
	if (!withActors) {
		return;
	}
	if (first) {
		// We will insert the player manually
		//// put the player in the first room
		//engine.player->x=(x1+x2)/2;
		//engine.player->y=(y1+y2)/2;
		// Add stairs going up in first room
		stairsUp->x = (x1 + x2) / 2;
		stairsUp->y = (y1 + y2) / 2;
		///////////////////////////////////////
	} else {
		TCODRandom *rng = TCODRandom::getInstance ();
		// add monsters
		int nbMonsters = rng->getInt (0, MAX_ROOM_MONSTERS);
		while (nbMonsters > 0) {
			int x = rng->getInt (x1, x2);
			int y = rng->getInt (y1, y2);
			if (canWalk (x, y)) {
				addMonster (x, y);
			}
			nbMonsters--;
		}
		// add items
		int nbItems = rng->getInt (0, MAX_ROOM_ITEMS);
		while (nbItems > 0) {
			int x = rng->getInt (x1, x2);
			int y = rng->getInt (y1, y2);
			if (canWalk (x, y)) {
				addItem (x, y);
			}
			nbItems--;
		}
		// set stairs position
		stairs->x = (x1 + x2) / 2;
		stairs->y = (y1 + y2) / 2;
	}
}

bool Map::hasFeatureAt (Actor *owner, const char featureGlyph) const {
	for (Actor **iterator = actors.begin ();
		 iterator != actors.end ();
		 iterator++) {
		Actor *actor = *iterator;
		if (actor->x == owner->x && actor->y == owner->y) {
			// Hack
			if (actor->ch == featureGlyph) {
				//std::cout << featureGlyph << ": " << actor->x << ", " << actor->y << " owner: " << owner->x << ", " << owner->y << std::endl;
				return true;
			}
		}
	}
	return false;
}

bool Map::hasFeatureAt (int x, int y, const char featureGlyph) const {
	for (Actor **iterator = actors.begin ();
		 iterator != actors.end ();
		 iterator++) {
		Actor *actor = *iterator;
		if (actor->x == x && actor->y == y && actor->ch == featureGlyph) {
			return true;
		}
	}
	return false;
}

Actor *Map::getFeatureAt (Actor *owner, const char featureGlyph) {
	for (Actor **iterator = actors.begin ();
		 iterator != actors.end ();
		 iterator++) {
		Actor *actor = *iterator;
		if (actor->x == owner->x && actor->y == owner->y) {
			// Hack
			if (actor->ch == featureGlyph) {
				//std::cout << featureGlyph << ": " << actor->x << ", " << actor->y << " owner: " << owner->x << ", " << owner->y << std::endl;
				return actor;
			}
		}
	}
	return NULL;
}

bool Map::isWall (int x, int y) const {
	//return !map->isWalkable(x,y);
	return tiles[x + y * width].type == Tile::Type::WALL;
}

bool Map::isMountain (int x, int y) const {
	return tiles[x + y * width].type == Tile::Type::MOUNTAIN;
}

bool Map::canWalk (int x, int y) const {
	/*if (x < 0 || x >= width) {
		return false;
		}
		if (y < 0 || y >= height) {
		return false;
		}*/
	if (isWall (x, y)) {
		// this is a wall
		return false;
	}
	for (Actor **iterator = actors.begin ();
		 iterator != actors.end (); iterator++) {
		Actor *actor = *iterator;
		if (actor->blocks && actor->x == x && actor->y == y) {
			// there is a blocking actor here. cannot walk
			return false;
		}
	}
	return true;
}

bool Map::isExplored (int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	return tiles[x + y*width].explored;
}

void Map::setFullyExplored () {
	for (int i = 0; i < width * height; i++) {
		tiles[i].explored = true;
	}
}

bool Map::isInFov (int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	if (map->isInFov (x, y)) {
		tiles[x + y*width].explored = true;
		return true;
	}
	return false;
}

void Map::computeFov () {
	map->computeFov (engine.player->x, engine.player->y,
					 engine.fovRadius);
}

void Map::setTileEffect (int x, int y, Tile::Effect effect) {
	tiles[x + y * width].effect = effect;
}

void Map::getRandomCoords (int *x, int *y) {
	*x = rng->getInt (0, width - 1);
	*y = rng->getInt (0, height - 1);
}
Tile *Map::getTile (int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return NULL;
	}
	return &tiles[x + y * width];
}

void Map::render (TCODConsole *target) const {
	/*
	Color constants

	Reference:
	// World map
	PLAIN, FOREST, MOUNTAIN, JUNGLE, DESERT, GLACIER, TUNDRA, OCEAN, LAKE, SWAMP,
	// Common
	WATER_SHALLOW
	// Area maps
	FLOOR, WALL, GRASS, TREE, WATER_DEEP
	*/

	// World
	static const TCODColor PLAIN (TCODColor::darkestGreen);
	static const TCODColor FOREST (TCODColor::darkerGreen);
	static const TCODColor MOUNTAIN (TCODColor::darkerGrey);
	static const TCODColor HILL (TCODColor::darkestOrange);
	static const TCODColor JUNGLE (TCODColor::darkerGreen);
	static const TCODColor SHORE (TCODColor::lightestOrange);
	static const TCODColor DESERT (TCODColor::lighterOrange);
	static const TCODColor GLACIER (TCODColor::blue);
	static const TCODColor TUNDRA (TCODColor::green);
	static const TCODColor OCEAN (TCODColor::blue);
	static const TCODColor LAKE (TCODColor::lighterBlue);
	static const TCODColor SWAMP (TCODColor::lighterBlue);
	static const TCODColor RIVER (TCODColor::lighterBlue);

	// Common
	static const TCODColor WATER_SHALLOW (TCODColor::lightBlue);

	// Area
	static const TCODColor WALL (TCODColor::lightGrey);
	static const TCODColor FLOOR (TCODColor::darkGrey);
	static const TCODColor GRASS (TCODColor::darkestGreen);
	static const TCODColor TREE (TCODColor::darkestGreen);
	static const TCODColor WATER_DEEP (TCODColor::blue);

	// Constant glyphs
	static const int ALMOST_EQUAL_TO = 247;
	static const int JUNLE_J = 20;// 244;
	static const int INTERSECTION = 239;
	static const int UP_TRIANGLE = 30;
	static const int SPADE = 6;
	static const int CLOVER = 5;

	// Get the drawing offsets
	int xOffset = engine.xOffset;
	int yOffset = engine.yOffset;

	// Init skew coordinates
	int skewX;
	int skewY;

	// For x within view width and y within view height
	// DEBUG: engine.VIEW_WIDTH instead of VIEW_WIDTH / 2 and same for height to draw when map scrolls to edge
	//			until I can fix.
	Actor *p = engine.player;
	for (int x = p->x - engine.VIEW_WIDTH / 2; x <= p->x + engine.VIEW_WIDTH / 2; x++) {
		for (int y = p->y - engine.VIEW_HEIGHT / 2; y <= p->y + engine.VIEW_HEIGHT / 2; y++) {



			// Out of range
			if (x < 0 || y < 0 || x >= width || y >= height) {
				continue;
			}

			// Defaults
			int glyph = '?';
			TCODColor backColor = TCODColor::black;
			TCODColor foreColor = TCODColor::azure;

			// Current tile pointer
			Tile *tile = &tiles[x + y * width];
			// Set colors if the type is WORLD
			if (engine.map->type == Map::Type::WORLD) {
				switch (tile->type) {
					case Tile::Type::PLAIN:
						glyph = ALMOST_EQUAL_TO; backColor = backColor; foreColor = PLAIN; break;
					case Tile::Type::FOREST:
						glyph = SPADE; backColor = backColor; foreColor = FOREST; break;
					case Tile::Type::MOUNTAIN:
						glyph = UP_TRIANGLE; backColor = backColor; foreColor = MOUNTAIN; break;
					case Tile::Type::HILL:
						glyph = INTERSECTION; backColor = backColor; foreColor = HILL; break;
					case Tile::Type::JUNGLE:
						glyph = JUNLE_J; backColor = backColor; foreColor = JUNGLE; break;
					case Tile::Type::DESERT:
						glyph = ALMOST_EQUAL_TO; backColor = backColor; foreColor = DESERT; break;
					case Tile::Type::SHORE:
						glyph = 240; backColor = backColor; foreColor = SHORE; break;
					case Tile::Type::GLACIER:
						glyph = '='; backColor = backColor; foreColor = GLACIER; break;
					case Tile::Type::TUNDRA:
						glyph = '='; backColor = backColor; foreColor = TUNDRA; break;
					case Tile::Type::OCEAN:
						if (tile->style == 0) {
							glyph = ALMOST_EQUAL_TO;
						} else {
							glyph = '=';
						}
						backColor = backColor; foreColor = OCEAN; break;
					case Tile::Type::LAKE:
						glyph = '='; backColor = backColor; foreColor = LAKE; break;
					case Tile::Type::WATER_SHALLOW:
						if (tile->style == 0) {
							glyph = ALMOST_EQUAL_TO;
						} else {
							glyph = '=';
						}
						backColor = backColor; foreColor = WATER_SHALLOW; break;
					case Tile::Type::RIVER:
						switch (tile->style) {
							case 0: glyph = ALMOST_EQUAL_TO; break;
							case 1: glyph = '='; break;
								// cornering broken, use this
								//case -1: glyph = ALMOST_EQUAL_TO; break;
								// up and down
								//case 0: glyph = 186; break;
								// left and right
								//case 1:	glyph = 205; break;
								// upper left
							case 2: glyph = 201; break;
								// upper right
							case 3: glyph = 187; break;
								// lower left
							case 4: glyph = 200; break;
								// lower right
							case 5: glyph = 188; break;
						}
						backColor = backColor; foreColor = RIVER; break;
					case Tile::Type::SWAMP:
						glyph = ALMOST_EQUAL_TO; backColor = backColor; foreColor = SWAMP; break;
					default:
						glyph = '?'; backColor = backColor; foreColor = PLAIN; break;
				}
			}

			// Set colors if type is NOT WORLD
			else {
				switch (tile->type) {
					case Tile::Type::FLOOR:
						glyph = '.'; backColor = backColor; foreColor = FLOOR; break;
					case Tile::Type::WALL:
						glyph = '#'; backColor = backColor; foreColor = WALL; break;
					case Tile::Type::GRASS:
						glyph = ALMOST_EQUAL_TO; backColor = backColor; foreColor = GRASS; break;
					case Tile::Type::TREE:
						glyph = SPADE; backColor = backColor; foreColor = TREE; break;
					case Tile::Type::WATER_SHALLOW:
						if (tile->style == 0) {
							glyph = ALMOST_EQUAL_TO;
						} else {
							glyph = '=';
						}
						backColor = backColor; foreColor = WATER_SHALLOW; break;
					case Tile::Type::WATER_DEEP:
						glyph = ALMOST_EQUAL_TO; backColor = backColor; foreColor = WATER_DEEP; break;
					default:
						glyph = '!'; backColor = backColor; foreColor = FLOOR; break;
				}
			}

			/*
			This sets the color brightness/contrast/gradient,
			based on the height(variation) of each tile.
			*/

			// The calcuations result in a particually dim color, so these will brighten it up
			static const float BRIGHTNESS_WORLD = 2.5f;
			static const float BRIGHTNESS_TOWN = 2.5f;
			float brightness = 2.0f;
			switch (type) {
				case Map::Type::WORLD: brightness = BRIGHTNESS_WORLD; break;
				case Map::Type::TOWN: brightness = BRIGHTNESS_TOWN; break;
			}

			// This is the multiplier to use against the foreColor.
			// A variation that is the maximum height of the map will get the full color * brightness
			float heightColorMult = (float)(tile->variation / heightMapMax);

			// Squaring the heightColorMult gives a more pronounced gradient, but leaves it dim,
			// so we use the brightness to jack it back up.
			foreColor = foreColor * (heightColorMult * heightColorMult) * brightness;

			// Set the backcolor to 1/4 of the foreColor for better visual appeal
			backColor = foreColor * 0.25f;

			// TEMPERATURE TEST
			if (engine.showTemperature) {
				backColor = TCODColor::red * (tile->temperature / 120);
			} else if (engine.showWeather) {
				// Weather test
				if (tile->weather > MapFactory::HUMID_WEATHER) {
					backColor = TCODColor::green * (tile->weather / (400));
				} else if (tile->weather > 100) {
					backColor = TCODColor::blue * ((tile->weather + 100) / 400);
				}
				//else if (tile->weather > 50) {
				//	backColor = TCODColor::yellow * ((tile->weather + 50) / 400);
				//}
				else {
					backColor = TCODColor::darkRed * ((tile->weather + 400) / 400);
				}
				backColor = backColor * .5;
				/////////////////////////////////////
			} else {
				if (type == Type::WORLD || type == Type::TOWN) {
					switch (tile->effect) {
						case Tile::Effect::FROZEN:
							if (tile->type != Tile::Type::OCEAN && tile->type != Tile::Type::WATER_SHALLOW) {
								backColor = TCODColor::white * (abs (tile->temperature) / (10 / 2));
							} else if (tile->type == Tile::Type::OCEAN) {
								backColor = TCODColor::lightBlue * (abs (tile->temperature) / (40 / 2));
							} else if (tile->type == Tile::Type::WATER_SHALLOW) {
								backColor = TCODColor::lighterBlue * (abs (tile->temperature) / (40 / 2));
							}
							break;
					}
				}
			}


			//Skew X and Y based on the offset for drawing
			skewX = x;
			skewY = y;
			engine.translateToView (skewX, skewY);



			if (isInFov (x, y)) {
				//TCODConsole::root->setChar (skewX, skewY, glyph);
				target->setChar (skewX, skewY, glyph);
				//TCODConsole::root->setCharBackground (skewX, skewY,
				//									  backColor
				target->setCharBackground (skewX, skewY, backColor);
				//TCODConsole::root->setCharForeground (skewX, skewY, foreColor);
				target->setCharForeground (skewX, skewY, foreColor);
			} else if (isExplored (x, y)) {
				if (type != Map::Type::WORLD) {
					backColor = backColor * 0.5f;
					foreColor = foreColor * 0.5f;
				}
				//TCODConsole::root->setChar (skewX, skewY, glyph);
				target->setChar (skewX, skewY, glyph);
				//TCODConsole::root->setCharBackground (skewX, skewY,
				//									  backColor);
				target->setCharBackground (skewX, skewY, backColor);
				//TCODConsole::root->setCharForeground (skewX, skewY, foreColor);
				target->setCharForeground (skewX, skewY, foreColor);
			}
		}
	}
}
