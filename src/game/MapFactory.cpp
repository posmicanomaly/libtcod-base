#include "main.hpp"
const float MapFactory::DESERT_TEMPERATURE = 90.0f;
const float MapFactory::JUNGLE_TEMPERATURE = 75.0f;
const float MapFactory::FREEZING_TEMPERATURE = 0.0f;

void MapFactory::setMapTileProperties(Map &map) {
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			switch (map.tiles[x + y * map.width].type){
			case Tile::Type::MOUNTAIN:
			case Tile::Type::WALL:
				map.map->setProperties(x, y, false, false);
			}
		}
	}
}
void MapFactory::makeDungeonMap(Map &map) {
	TCODHeightMap heightMap(map.width, map.height);
	map.heightMapMin = 256.f;
	map.heightMapMax = 512.f;
	float waterLevel = 10.0f + map.heightMapMin;
	TCODNoise* noise2d = new TCODNoise(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);
	heightMap.addFbm(noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize(map.heightMapMin, map.heightMapMax);
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			float value = heightMap.getValue(x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;
			//std::cout << value << std::endl;
			if (value > waterLevel + 50) {
				//tile->type = Tile::Type::FLOOR;
			}
			else if (value > waterLevel) {
				tile->type = Tile::Type::WATER_SHALLOW;
			}
			else {
				tile->type = Tile::Type::WATER_DEEP;
			}
		}
	}
	setMapTileProperties(map);
}
void MapFactory::makeTownMap(Map &map) {

	placeBoundingWall(map, 5, 5, map.width - 5, map.height - 5);
	TCODHeightMap heightMap(map.width, map.height);
	map.heightMapMin = 256.f;
	map.heightMapMax = 512.f;
	float waterLevel = 25.f + map.heightMapMin;
	TCODNoise* noise2d = new TCODNoise(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);
	heightMap.addFbm(noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize(map.heightMapMin, map.heightMapMax);
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			float value = heightMap.getValue(x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;
			//std::cout << value << std::endl;
			if (value > waterLevel + 125) {
				tile->type = Tile::Type::TREE;
			}
			else if (value > waterLevel + 50) {
				tile->type = Tile::Type::GRASS;
			}
			else if (value > waterLevel) {
				tile->type = Tile::Type::WATER_SHALLOW;
			}
			else {
				tile->type = Tile::Type::WATER_DEEP;
			}
		}
	}
	//MapFactory::addTrees(map, 200);
	generateTownBuildings(map);
	setMapTileProperties(map);
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			if (map.tiles[x + y * map.width].type == Tile::Type::WALL) {
				map.map->setProperties(x, y, true, false);
			}
		}
	}
	delete noise2d;
}
void MapFactory::makeWorldMap(Map &map) {

	TCODHeightMap heightMap(map.width, map.height);
	map.heightMapMin = 0.0f;
	map.heightMapMax = 512.0f;
	TCODNoise* noise2d = new TCODNoise(2, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, map.rng, TCOD_NOISE_PERLIN);

	std::cout << map.heightMapMax << std::endl;
	heightMap.addFbm(noise2d, map.width / 32, map.height / 32, 0, 0, 8, 0.0f, 1.0f);
	heightMap.normalize(map.heightMapMin, map.heightMapMax);
	delete noise2d;


	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {

			float value = heightMap.getValue(x, y);
			Tile *tile = &map.tiles[x + y * map.width];
			tile->variation = value;

			if (value > 400) {
				tile->type = Tile::Type::MOUNTAIN;
			}
			else if (value > 380) {
				tile->type = Tile::Type::HILL;
			}
			else if (value > 290) {
				tile->type = Tile::Type::PLAIN;
			}
			else if (value > 270){
				// Beach only at this time
				tile->type = Tile::Type::SHORE;
			}
			else if (value > 245) {
				// Shore / low lakes
				tile->type = Tile::Type::WATER_SHALLOW;
			}
			else
				tile->type = Tile::Type::OCEAN;
		}
	}

	// Set temperatures
	MapFactory::calcTemperatures(map);
	// Seed some deserts
	MapFactory::addDeserts(map, 64);
	// Plant some trees
	MapFactory::addTrees(map, 16);
	// Add some rivers
	MapFactory::addRivers(map);
	// Add some caves
	MapFactory::addCaves(map);
	// Add some towns
	MapFactory::addTowns(map);

	

	// Set all the map's TCODMAP properties based on Tile::Type
	setMapTileProperties(map);
}

void MapFactory::calcTemperatures(Map &map) {
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			Tile *t = &map.tiles[x + y * map.width];
			float height = t->variation;
			// Pole temperatures, Y axis
			float defaultTemp = 110;
			float lowestTemp = -40;
			float difference = 0;

			difference = abs(map.height / 2 - y) * 1.5f;
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
				t->effect = Tile::Effect::FROZEN;
			}
		}
	}
}

void MapFactory::addDeserts(Map &map, int divisor) {
	Tile::Type baseType = Tile::Type::PLAIN;
	for (int i = 0; i < map.width / divisor * map.height / divisor; i++) {
		int x, y;
		Tile *t;
		bool valid = false;
		do {
			x = map.rng->getInt(1, map.width - 1);
			y = map.rng->getInt(1, map.height - 1);
			t = &map.tiles[x + y * map.width];
			if (t->type == baseType) {
				if (t->temperature >= MapFactory::DESERT_TEMPERATURE) {
					valid = true;
				}
			}
		} while (!valid);
		MapFactory::addFeatureSeed(map, x, y, Tile::Type::DESERT, 1, 1000);

	}
}

void MapFactory::addTrees(Map &map, int divisor) {
	Tile::Type baseType;
	Tile::Type treeType;
	switch (map.type) {
	case Map::Type::WORLD:
		baseType = Tile::Type::PLAIN; treeType = Tile::Type::FOREST; break;
	case Map::Type::TOWN:
		baseType = Tile::Type::GRASS; treeType = Tile::Type::TREE; break;
	default:				baseType = Tile::Type::PLAIN; break;
	}
	for (int i = 0; i < map.width / divisor * map.height / divisor; i++) {
		bool jungle = false;
		int x, y;
		do {
			x = map.rng->getInt(1, map.width - 1);
			y = map.rng->getInt(1, map.height - 1);
		} while (map.tiles[x + y * map.width].type != baseType);
		Tile *t = &map.tiles[x + y * map.width];
		if (map.type == Map::Type::WORLD) {
			if (t->temperature >= MapFactory::JUNGLE_TEMPERATURE) {
				treeType = Tile::Type::JUNGLE;
			}
			else {
				treeType = Tile::Type::FOREST;
			}
		}
		MapFactory::addFeatureSeed(map, x, y, treeType, 1, 1000);
		
	}
}

void MapFactory::addTowns(Map &map) {
	std::vector<std::string> names;
	names.push_back("Town 1");
	names.push_back("Town 2");
	names.push_back("Town 3");
	names.push_back("town 4");
	names.push_back("Town 5");
	names.push_back("Town 6");
	names.push_back("Town 7");
	bool placeTowns = true;
	if (placeTowns) {
		for (int i = 0; i < names.size(); i++) {
			Tile *tile;
			int x, y;
			bool valid = false;
			do {
				x = map.rng->getInt(1, map.width - 1);
				y = map.rng->getInt(1, map.height - 1);
				tile = &map.tiles[x + y * map.width];
				if (tile->type == Tile::Type::PLAIN || tile->type == Tile::Type::FOREST || tile->type == Tile::Type::JUNGLE)
					valid = true;
				if (valid) {
					if (map.hasFeatureAt(x, y, '*'))
						valid = false;
				}
			} while (!valid);
			std::string name = names[i];

			Actor *town = new Actor(x, y, 'O', name.c_str(), TCODColor::white);
			town->fovOnly = false;
			town->blocks = false;
			map.actors.push(town);
			// Make the tile walkable
			map.tiles[x + y * map.width].type = Tile::Type::PLAIN;
			map.map->setProperties(x, y, true, true);
		}
	}
}

void MapFactory::addCaves(Map &map) {
	std::vector<std::string> names;
	names.push_back("Dungeon 1");
	names.push_back("Dungeon 2");
	names.push_back("Dungeon 3");
	names.push_back("Dungeon 4");
	names.push_back("Dungeon 5");
	names.push_back("Dungeon 6");
	for (int i = 0; i < names.size(); i++) {
		Tile *tile;
		int x, y;
		bool valid = false;
		do {
			// Make the target area smaller so I don't have to bounds check
			x = map.rng->getInt(5, map.width - 5);
			y = map.rng->getInt(5, map.height - 5);
			tile = &map.tiles[x + y * map.width];
			if (tile->type == Tile::Type::MOUNTAIN) {
				Tile *up = &map.tiles[x + (y - 1) * map.width];
				Tile *down = &map.tiles[x + (y + 1) * map.width];
				Tile *left = &map.tiles[(x - 1) + y * map.width];
				Tile *right = &map.tiles[(x + 1) + y * map.width];
				std::vector<Tile*> adjacentTiles;
				adjacentTiles.push_back(up);
				adjacentTiles.push_back(down);
				adjacentTiles.push_back(left);
				adjacentTiles.push_back(right);
				for (int i = 0; i < adjacentTiles.size(); i++) {
					Tile *cur = adjacentTiles[i];
					switch (cur->type) {
					case Tile::Type::PLAIN:
					case Tile::Type::FOREST:
					case Tile::Type::LAKE:
					case Tile::Type::DESERT:
					case Tile::Type::JUNGLE:
					case Tile::Type::HILL:
						valid = true; break;
					}
					if (valid) {
						break;
					}
				}
			}
		} while (!valid);
		std::string name = names[i];

		Actor *cave = new Actor(x, y, '*', name.c_str(), TCODColor::white);
		cave->fovOnly = false;
		cave->blocks = false;
		map.actors.push(cave);
		// Make the tile walkable
		map.tiles[x + y * map.width].type = Tile::Type::PLAIN;
		map.map->setProperties(x, y, true, true);
	}
}

void MapFactory::addRivers(Map &map) {
	for (int i = 0; i < 7; i++) {
		std::cout << "creating river " << i << std::endl;
		int x, y;
		Tile *tile;
		do {
			x = map.rng->getInt(5, map.width - 5);
			y = map.rng->getInt(5, map.height - 5);
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
				}
				else if (prevDirection == DIR::right) {
					style = 5;
				}
				else {
					style = 0;
				}
				break;
			case DIR::down:
				if (prevDirection == DIR::left) {
					style = 2;
				}
				else if (prevDirection == DIR::right) {
					style = 3;
				}
				else {
					style = 0;
				}
				break;
			case DIR::left:
				if (prevDirection == DIR::up) {
					style = 5;
				}
				else if (prevDirection == DIR::down) {
					style = 2;
				}
				else {
					style = 1;
				}
				break;
			case DIR::right:
				if (prevDirection == DIR::up) {
					style = 5;
				}
				else if (prevDirection == DIR::down) {
					style = 3;
				}
				else {
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
			adjacentTiles.push_back(up);
			adjacentTiles.push_back(down);
			adjacentTiles.push_back(left);
			adjacentTiles.push_back(right);
			int lowestIndex = -1;
			float lowestValue = 999;
			for (int i = 0; i < adjacentTiles.size(); i++) {
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
void MapFactory::addFeatureSeed(Map &map, int x, int y, Tile::Type type, int minStrength, int maxStrength) {

	// set tile to type
	map.tiles[x + y * map.width].type = type;

	// strength is how many times it will try and continue to spread
	int strength = map.rng->getInt(minStrength, maxStrength);

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
			xd = map.rng->getInt(-1, 1);
			yd = map.rng->getInt(-1, 1);
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
			bool changeType = true;
			switch (tile->type) {
			case Tile::Type::MOUNTAIN:
			case Tile::Type::OCEAN:
			case Tile::Type::WATER_SHALLOW:
			case Tile::Type::DESERT:
			case Tile::Type::SHORE:
			case Tile::Type::LAKE: changeType = false; break;
			}
			if (changeType) {
				tile->type = type;
			}
		}
	}
}

void MapFactory::fillWithType(Map &map, Tile::Type type) {
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
			map.map->setProperties(x, y, transparent, walkable);
		}
	}
}

void MapFactory::generateTownBuildings(Map &map) {
	int buildings = 25;
	for (int i = 0; i < buildings; i++) {
		int x, y;
		do {
			x = map.rng->getInt(6, map.width - 7);
			y = map.rng->getInt(6, map.height - 7);
		} while (!checkBuildingPlacement(map, x, y, 5, 5));
		placeBuilding(map, x, y, 5, 5);
	}
}

bool MapFactory::checkBuildingPlacement(Map &map, int x, int y, int width, int height) {
	for (int curX = x; curX <= x + width; curX++) {
		for (int curY = y; curY <= y + height; curY++) {
			if (map.tiles[curX + curY * map.width].type == Tile::Type::WALL)
				return false;
		}
	}
	return true;
}

void MapFactory::placeBuilding(Map &map, int x, int y, int width, int height) {
	if (!checkBuildingPlacement(map, x, y, width, height))
		return;
	for (int curX = x; curX <= x + width; curX++) {
		for (int curY = y; curY <= y + height; curY++) {
			map.tiles[curX + curY * map.width].type = Tile::Type::WALL;
			map.map->setProperties(curX, curY, false, false);
		}
	}
}

void MapFactory::placeBoundingWall(Map &map, int x1, int y1, int x2, int y2) {
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