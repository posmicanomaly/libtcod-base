#include "main.hpp"
const float MapFactory::DESERT_TEMPERATURE = 80.0f;
const float MapFactory::JUNGLE_TEMPERATURE = 68.0f;
const float MapFactory::FREEZING_TEMPERATURE = 0.0f;

const float MapFactory::ARID_WEATHER = 50.0f;
const float MapFactory::HUMID_WEATHER = 170.0f;

void MapFactory::setMapTileProperties (Map &map) {
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			switch (map.tiles[x + y * map.width].type) {
				case Tile::Type::MOUNTAIN:
				case Tile::Type::WALL:
					map.map->setProperties (x, y, false, false);
			}
		}
	}
}
void MapFactory::makeDungeonMap (Map &map) {
	TCODHeightMap heightMap (map.width, map.height);
	map.heightMapMin = 256.f;
	map.heightMapMax = 512.f;
	float waterLevel = 10.0f + map.heightMapMin;
	TCODNoise* noise2d = new TCODNoise (2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);
	heightMap.addFbm (noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize (map.heightMapMin, map.heightMapMax);
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			float value = heightMap.getValue (x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;
			if (value > waterLevel + 50) {
				//tile->type = Tile::Type::FLOOR;
			} else if (value > waterLevel) {
				tile->type = Tile::Type::WATER_SHALLOW;
			} else {
				tile->type = Tile::Type::WATER_DEEP;
			}
		}
	}
	setMapTileProperties (map);
}
void MapFactory::makeTownMap (Map &map) {
	//placeBoundingWall(map, 5, 5, map.width - 5, map.height - 5);
	TCODHeightMap heightMap (map.width, map.height);
	map.heightMapMin = 256.f;
	map.heightMapMax = 512.f;
	float waterLevel = 25.f + map.heightMapMin;
	TCODNoise* noise2d = new TCODNoise (2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);
	heightMap.addFbm (noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize (map.heightMapMin, map.heightMapMax);
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			float value = heightMap.getValue (x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;
			//std::cout << value << std::endl;
			if (value > waterLevel + 125) {
				tile->type = Tile::Type::TREE;
			} else if (value > waterLevel + 50) {
				tile->type = Tile::Type::GRASS;
			} else if (value > waterLevel) {
				tile->type = Tile::Type::WATER_SHALLOW;
			} else {
				tile->type = Tile::Type::WATER_DEEP;
			}
		}
	}
	//MapFactory::addTrees(map, 200);
	generateTownBuildings (map);
	setMapTileProperties (map);
	delete noise2d;
}
void MapFactory::makeWorldMap (Map &map) {

	TCODHeightMap heightMap (map.width, map.height);
	map.heightMapMin = 0.0f;
	map.heightMapMax = 512.0f;
	TCODNoise* noise2d = new TCODNoise (2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);

	std::cout << map.heightMapMax << std::endl;
	heightMap.addFbm (noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize (map.heightMapMin, map.heightMapMax);
	delete noise2d;



	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {

			float value = heightMap.getValue (x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;

			if (value > 400) {
				tile->type = Tile::Type::MOUNTAIN;
			} else if (value > 380) {
				tile->type = Tile::Type::HILL;
			} else if (value > 290) {
				tile->type = Tile::Type::PLAIN;
			} else if (value > 270) {
				// Beach only at this time
				tile->type = Tile::Type::SHORE;
			} else if (value > 245) {
				// Shore / low lakes
				tile->type = Tile::Type::WATER_SHALLOW;
			} else
				tile->type = Tile::Type::OCEAN;
		}
	}

	// Set temperatures
	MapFactory::calcTemperatures (map);
	MapFactory::calcWeather (map);
	// Seed some deserts
	MapFactory::addDeserts (map, 64);
	// Plant some trees
	MapFactory::addTrees (map, 32);
	// Add some rivers
	MapFactory::addRivers (map);
	// Add some caves
	MapFactory::addCaves (map);
	// Add some towns
	MapFactory::addTowns (map);



	// Set all the map's TCODMAP properties based on Tile::Type
	setMapTileProperties (map);
}

void MapFactory::calcTemperatures (Map &map) {
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			Tile *t = &map.tiles[x + y * map.width];
			float height = t->variation;
			// Pole temperatures, Y axis
			float defaultTemp = 120;
			float lowestTemp = -10;
			float difference = 0;

			difference = abs (map.height / 2 - y) * 1.5f;
			float temp = defaultTemp - difference;
			if (height < 245) {
				height = 245;
			}
			height -= 245;
			temp -= height / 3;
			if (temp < lowestTemp) {
				temp = lowestTemp;
			}
			t->temperature = temp;
			if (temp <= MapFactory::FREEZING_TEMPERATURE) {
				if (t->type != Tile::Type::OCEAN)
					t->effect = Tile::Effect::FROZEN;
			}
		}
	}
}


void MapFactory::calcWeather (Map &map) {
	TCODHeightMap weatherMap (map.width, map.height);
	TCODNoise* noise2d = new TCODNoise (2, 1.0f, 20.0f, map.rng, TCOD_NOISE_PERLIN);
	weatherMap.addFbm (noise2d, map.width / 24, map.height / 24, 0, 0, 8, 0.0f, 1.0f);

	// 0cm to 400cm rainfall per year
	weatherMap.normalize (0, 400);
	delete noise2d;
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			Tile *t = &map.tiles[x + y * map.width];
			// The elevation
			float height = t->variation;

			// Init adjustment
			float adjustment = t->variation;

			// Provides for different scaling based on range
			// less than sea level or greater than some range in the mountain range
			if (t->variation < 245 || t->variation > 400)
				adjustment = -(abs (adjustment / 8));
			// otherwise
			else {
				adjustment = -(abs (adjustment / 8));
			}

			// Set the "weather" which is actually just "rainfall" and should probably be refactored
			t->weather = weatherMap.getValue (x, y) + adjustment;
		}
	}
}

void MapFactory::addDeserts (Map &map, int divisor) {
	Tile::Type baseType = Tile::Type::PLAIN;
	int tries;
	int maxTries = map.width * map.height;
	int failed = 0;
	int amount = map.width / divisor * map.height / divisor;
	for (int i = 0; i < amount; i++) {
		tries = 0;
		int x, y;
		Tile *t;
		bool valid = false;
		do {
			if (tries > maxTries) {
				valid = false;
				break;
			}
			x = map.rng->getInt (1, map.width - 1);
			y = map.rng->getInt (1, map.height - 1);
			t = &map.tiles[x + y * map.width];
			if (t->type == baseType) {
				if (t->temperature >= MapFactory::DESERT_TEMPERATURE) {
					if (t->weather <= MapFactory::ARID_WEATHER)
						valid = true;
				}
			}
			tries++;
		} while (!valid);
		if (valid)
			MapFactory::addFeatureSeed (map, x, y, Tile::Type::DESERT, 1, 1000);
		else {
			failed++;
		}

	}
	std::cout << "Desert seeds: " << amount << "(" << failed << " failed)" << std::endl;
}

void MapFactory::addTrees (Map &map, int divisor) {
	Tile::Type baseType;
	Tile::Type treeType;
	switch (map.type) {
		case Map::Type::WORLD:
			baseType = Tile::Type::PLAIN; treeType = Tile::Type::FOREST; break;
		case Map::Type::TOWN:
			baseType = Tile::Type::GRASS; treeType = Tile::Type::TREE; break;
		default:				baseType = Tile::Type::PLAIN; break;
	}
	int jungles = 0;
	int forests = 0;

	// Jungles
	for (int i = 0; i < map.width / divisor * map.height / divisor; i++) {
		int x, y;
		Tile *t = NULL;
		bool valid = false;
		int tries = 0;
		int fails = 0;
		int maxTries = map.width * map.height;
		do {
			if (tries > maxTries) {
				valid = false;
				break;
			}
			x = map.rng->getInt (1, map.width - 1);
			y = map.rng->getInt (1, map.height - 1);
			t = &map.tiles[x + y * map.width];
			if (t->type == baseType) {
				if (t->temperature >= MapFactory::JUNGLE_TEMPERATURE && t->weather >= MapFactory::HUMID_WEATHER)
					valid = true;
			}
			tries++;
		} while (!valid);
		if (valid) {
			if (t->temperature >= MapFactory::JUNGLE_TEMPERATURE && t->weather >= MapFactory::HUMID_WEATHER) {
				treeType = Tile::Type::JUNGLE;
				jungles++;
			}
		} else {
			fails++;
		}
		MapFactory::addFeatureSeed (map, x, y, Tile::Type::JUNGLE, 1, 1000);
	}

	// Temperate forests
	for (int i = 0; i < map.width / divisor * map.height / divisor; i++) {
		int x, y;
		do {
			x = map.rng->getInt (1, map.width - 1);
			y = map.rng->getInt (1, map.height - 1);
		} while (map.tiles[x + y * map.width].type != baseType);
		Tile *t = &map.tiles[x + y * map.width];
		if (map.type == Map::Type::WORLD) {
			treeType = Tile::Type::FOREST;
			forests++;

		} else {
			treeType = Tile::Type::TREE;
		}

		MapFactory::addFeatureSeed (map, x, y, treeType, 1, 1000);

	}
	std::cout << "Forests: " << forests << " Jungles: " << jungles << std::endl;
}

void MapFactory::addTowns (Map &map) {
	int townsToAdd = 15;
	bool placeTowns = true;
	if (placeTowns) {
		for (int i = 0; i < townsToAdd; i++) {
			Tile *tile;
			int x, y;
			bool valid = false;
			do {
				x = map.rng->getInt (1, map.width - 1);
				y = map.rng->getInt (1, map.height - 1);
				tile = &map.tiles[x + y * map.width];
				switch (tile->type) {
					case Tile::Type::PLAIN:
					case Tile::Type::FOREST:
					case Tile::Type::DESERT:
					case Tile::Type::SHORE:
					case Tile::Type::HILL:
						valid = true;
				}
				if (valid) {
					if (map.hasFeatureAt (x, y, '*'))
						valid = false;
				}
			} while (!valid);
			std::string name = "The ";
			if (tile->temperature <= FREEZING_TEMPERATURE) {
				name += "Frozen ";
			}
			switch (tile->type) {
				case Tile::Type::JUNGLE:
					name += "Jungle Town of ";
					break;
				case Tile::Type::FOREST:
					name += "Forest Town of ";
					break;
				case Tile::Type::DESERT:
					name += "Desert Town of ";
					break;
				case Tile::Type::SHORE:
					name += "Coastal Town of ";
					break;
				case Tile::Type::HILL:
					name += "Outlook Town of ";
					break;
				default:
					name += "Town of ";
					break;
			}
			name += std::to_string (i);
			

			Actor *town = new Actor (x, y, 'O', name.c_str (), TCODColor::red);
			town->fovOnly = false;
			town->blocks = false;
			map.actors.push (town);
			// Make the tile walkable
			map.tiles[x + y * map.width].type = Tile::Type::PLAIN;
			map.map->setProperties (x, y, true, true);
		}
	}
}

void MapFactory::addCaves (Map &map) {
	std::vector<std::string> names;
	names.push_back ("Dungeon 1");
	names.push_back ("Dungeon 2");
	names.push_back ("Dungeon 3");
	names.push_back ("Dungeon 4");
	names.push_back ("Dungeon 5");
	names.push_back ("Dungeon 6");
	for (int i = 0; i < names.size (); i++) {
		Tile *tile;
		int x, y;
		bool valid = false;
		do {
			// Make the target area smaller so I don't have to bounds check
			x = map.rng->getInt (5, map.width - 5);
			y = map.rng->getInt (5, map.height - 5);
			tile = &map.tiles[x + y * map.width];
			if (tile->type == Tile::Type::MOUNTAIN) {
				Tile *up = &map.tiles[x + (y - 1) * map.width];
				Tile *down = &map.tiles[x + (y + 1) * map.width];
				Tile *left = &map.tiles[(x - 1) + y * map.width];
				Tile *right = &map.tiles[(x + 1) + y * map.width];
				std::vector<Tile*> adjacentTiles;
				adjacentTiles.push_back (up);
				adjacentTiles.push_back (down);
				adjacentTiles.push_back (left);
				adjacentTiles.push_back (right);
				for (int i = 0; i < adjacentTiles.size (); i++) {
					Tile *cur = adjacentTiles[i];
					switch (cur->type) {
						case Tile::Type::PLAIN:
						case Tile::Type::FOREST:
						case Tile::Type::LAKE:
						case Tile::Type::JUNGLE:
						case Tile::Type::HILL:
						case Tile::Type::DESERT:
							valid = true; break;
					}
					if (valid) {
						break;
					}
				}
			}
		} while (!valid);
		std::string name = names[i];

		Actor *cave = new Actor (x, y, '*', name.c_str (), TCODColor::red);
		cave->fovOnly = false;
		cave->blocks = false;
		map.actors.push (cave);
		// Make the tile walkable
		map.tiles[x + y * map.width].type = Tile::Type::PLAIN;
		map.map->setProperties (x, y, true, true);
	}
}

void MapFactory::addRivers (Map &map) {
	for (int i = 0; i < 7; i++) {
		std::cout << "creating river " << i << std::endl;
		int x, y;
		Tile *tile;
		do {
			x = map.rng->getInt (5, map.width - 5);
			y = map.rng->getInt (5, map.height - 5);
			tile = &map.tiles[x + y * map.width];
		} while (tile->type != Tile::Type::MOUNTAIN);
		int nextx, nexty;
		int xd, yd;
		Tile *lowest = tile;
		bool riverDone = false;
		std::cout << "\tflowing" << std::endl;

		static const enum DIR {
			up, down, left, right
		};
		int curDirection = DIR::up;
		int prevDirection = DIR::up;
		while (!riverDone) {
			if (lowest->type == Tile::Type::WATER_SHALLOW || lowest->type == Tile::Type::OCEAN || lowest->type == Tile::Type::LAKE) {
				riverDone = true;
				break;
			}
			std::cout << ".";
			lowest->type = Tile::Type::RIVER;
			//			// up and down
			//		case 0: glyph = 186; break;
			//			// left and right
			//		case 1:	glyph = 205; break;
			//			// upper left
			//		case 2: glyph = 201; break;
			//			// upper right
			//		case 3: glyph = 187; break;
			//			// lower left
			//		case 4: glyph = 200; break;
			//			// lower right
			//		case 5: glyph = 188; break;
			int style;
			switch (curDirection) {
				case DIR::up:
					if (prevDirection == DIR::left) {
						style = 4;
					} else if (prevDirection == DIR::right) {
						style = 5;
					} else {
						style = 0;
					}
					break;
				case DIR::down:
					if (prevDirection == DIR::left) {
						style = 2;
					} else if (prevDirection == DIR::right) {
						style = 3;
					} else {
						style = 0;
					}
					break;
				case DIR::left:
					if (prevDirection == DIR::up) {
						style = 5;
					} else if (prevDirection == DIR::down) {
						style = 2;
					} else {
						style = 1;
					}
					break;
				case DIR::right:
					if (prevDirection == DIR::up) {
						style = 5;
					} else if (prevDirection == DIR::down) {
						style = 3;
					} else {
						style = 1;
					}
					break;
			}
			// My river cornering isn't working right now, use 0 for default waves glyph
			style = 0;
			lowest->style = style;
			prevDirection = curDirection;
			Tile *up = NULL;
			Tile *down = NULL;
			Tile *left = NULL;
			Tile *right = NULL;
			if (y - 1 >= 0)
				up = &map.tiles[x + (y - 1) * map.width];
			if (y + 1 < map.height)
				down = &map.tiles[x + (y + 1) * map.width];
			if (x - 1 >= 0)
				left = &map.tiles[(x - 1) + y * map.width];
			if (x + 1 < map.width)
				right = &map.tiles[(x + 1) + y * map.width];
			std::vector<Tile*> adjacentTiles;
			adjacentTiles.push_back (up);
			adjacentTiles.push_back (down);
			adjacentTiles.push_back (left);
			adjacentTiles.push_back (right);
			int lowestIndex = -1;
			float lowestValue = 999;
			for (int i = 0; i < adjacentTiles.size (); i++) {
				Tile *cur = adjacentTiles[i];
				if (cur == NULL)
					continue;
				if (cur->type == Tile::Type::RIVER)
					continue;
				if (cur->variation < lowestValue) {
					lowestValue = cur->variation;
					lowest = cur;
					lowestIndex = i;
				}

			}

			if (lowestValue == 999 && lowestIndex == -1)
				break;
			switch (lowestIndex) {
				case -1: riverDone = true; break;
				case 0: y--; curDirection = DIR::up; break;
				case 1:	y++; curDirection = DIR::down; break;
				case 2:	x--; curDirection = DIR::left; break;
				case 3:	x++; curDirection = DIR::right; break;
			}
			//style = lowestIndex;
		}
		std::cout << std::endl;
	}
}

/*
Adds <amount> of <type> to <map>
for each amount, use a random strength to spread it around
*/
void MapFactory::addFeatureSeed (Map &map, int x, int y, Tile::Type type, int minStrength, int maxStrength) {

	// set tile to type
	map.tiles[x + y * map.width].type = type;

	// strength is how many times it will try and continue to spread
	int strength = map.rng->getInt (minStrength, maxStrength);

	// init nextx and nexty to the base seed
	int nextx = x;
	int nexty = y;

	// init directional modifiers
	int xd;
	int yd;

	// Grow seed
	int maxTries = strength * 10;
	int tries = 0;
	for (int s = 0; s < strength; s++) {
		// Get a random direction while the nextx, nexty is out of range
		bool valid = true;
		do {
			if (tries > maxTries) {
				return;
			}
			xd = map.rng->getInt (-1, 1);
			yd = map.rng->getInt (-1, 1);
			if ((nextx + xd < 0 || nextx + xd >= map.width) || (nexty + yd < 0 || nexty + yd >= map.height)) {
				valid = false;
				//std::cout << "out of range" << std::endl;
			}
			tries++;
		} while (!valid);

		if (valid) {
			// nextx, nexty will be in range, add the directional modifiers to nextx, nexty
			nextx += xd;
			nexty += yd;

			// set the nextx, nexty tile to type
			Tile *tile = &map.tiles[nextx + nexty * map.width];
			// Check for a blocking tile
			bool changeType = true;
			switch (tile->type) {
				case Tile::Type::MOUNTAIN:
				case Tile::Type::OCEAN:
				case Tile::Type::WATER_SHALLOW:
				case Tile::Type::SHORE:
				case Tile::Type::DESERT:
				case Tile::Type::LAKE: changeType = false; break;
			}
			// Check if tile is suitable
			if (changeType) {
				switch (type) {
					case Tile::Type::JUNGLE:
						if (tile->type == Tile::Type::FOREST) {
							changeType = false;
						}
						break;
					case Tile::Type::FOREST:
						if (tile->type == Tile::Type::JUNGLE) {
							changeType = false;
						}
						break;
				}
			}
			if (changeType) {
				tile->type = type;
			}
		}
	}
}

void MapFactory::fillWithType (Map &map, Tile::Type type) {
	bool transparent = true;
	bool walkable = true;
	switch (type) {
		case Tile::Type::MOUNTAIN: walkable = false; break;
		case Tile::Type::WALL: walkable = false; transparent = false; break;
		default: break;
	}
	// Fill with type
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			map.tiles[x + y * map.width].type = type;
			// If we don't do this, fov can't compute
			map.map->setProperties (x, y, transparent, walkable);
		}
	}
}

void MapFactory::generateTownBuildings (Map &map) {
	int buildings = 25;
	for (int i = 0; i < buildings; i++) {
		int x, y;
		do {
			x = map.rng->getInt (6, map.width - 7);
			y = map.rng->getInt (6, map.height - 7);
		} while (!checkBuildingPlacement (map, x, y, 5, 5));
		placeBuilding (map, x, y, 5, 5);
	}
}

bool MapFactory::checkBuildingPlacement (Map &map, int x, int y, int width, int height) {
	if (x - 1 < 0 || x + width + 1 >= map.width || y - 1 < 0 || y + height + 1 >= map.height)
		return false;
	for (int curX = x - 1; curX <= x + width + 1; curX++) {
		for (int curY = y - 1; curY <= y + height + 1; curY++) {
			Tile *t = &map.tiles[curX + curY * map.width];
			switch (t->type) {
				case Tile::Type::WALL:
				case Tile::Type::FLOOR:
					return false;
			}
		}
	}
	return true;
}

void MapFactory::placeBuilding (Map &map, int x, int y, int width, int height) {
	if (!checkBuildingPlacement (map, x, y, width, height))
		return;
	bool doorPlaced = false;
	for (int curX = x; curX <= x + width; curX++) {
		for (int curY = y; curY <= y + height; curY++) {
			Tile *t = &map.tiles[curX + curY * map.width];
			Tile::Type type = Tile::Type::WALL;
			bool transparent = false;
			bool walkable = false;
			// If this is on the edge
			if (curX == x || curX == x + width || curY == y || curY == y + height) {
				// If we still need a door, maybe this could be a door
				// TODO: real door placement, and not in a corner
				if (!doorPlaced) {
					int chance = map.rng->getInt (0, 100);
					if (chance < 10) {
						type = Tile::Type::FLOOR;
						transparent = true;
						walkable = true;
						doorPlaced = true;
					}
				}
				// Otherwise, just a wall
				else {
					type = Tile::Type::WALL;
					transparent = false;
					walkable = false;
				}
			}
			// Not on edge, a floor then
			else {
				type = Tile::Type::FLOOR;
				transparent = true;
				walkable = true;
			}
			// Set variables
			t->type = type;
			map.map->setProperties (curX, curY, transparent, walkable);
		}
	}
}

void MapFactory::placeBoundingWall (Map &map, int x1, int y1, int x2, int y2) {
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			if (x == x1 || x == x2) {
				if (y != (y1 + y2) / 2)
					map.tiles[x + y * map.width].type = Tile::Type::WALL;
			}
			if (y == y1 || y == y2) {
				if (x != (x1 + x2) / 2)
					map.tiles[x + y * map.width].type = Tile::Type::WALL;
			}
		}
	}
}