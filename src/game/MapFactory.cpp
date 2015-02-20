#include "main.hpp"
void MapFactory::makeTownMap(Map &map) {
	
	placeBoundingWall(map, 5, 5, map.width - 5, map.height - 5);
	TCODHeightMap heightMap(map.width, map.height);
	map.heightMapMin = 256.f;
	map.heightMapMax = 512.f;
	float waterLevel = 50.f + map.heightMapMin;
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
	generateTownBuildings(map);
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
				tile->type = Tile::Type::DESERT;
			}
			else if (value > 256) {
				// Shore / low lakes
				tile->type = Tile::Type::WATER_SHALLOW;
			}
			else
				tile->type = Tile::Type::OCEAN;
		}
	}

	// Plant some trees
	for (int i = 0; i < map.width / 16 * map.height / 16; i++) {
		bool jungle = false;
		int x, y;
		do {
			int jungleChance = map.rng->getInt(0, 100);
			if (jungleChance < 25) {
				jungle = true;
			}
			x = map.rng->getInt(1, map.width - 1);
			y = map.rng->getInt(1, map.height - 1);
		} while (map.tiles[x + y * map.width].type != Tile::Type::PLAIN);
		if (jungle) {
			MapFactory::addFeatureSeed(map, x, y, Tile::Type::JUNGLE, 100, 1000);
		}
		else {
			MapFactory::addFeatureSeed(map, x, y, Tile::Type::FOREST, 100, 1000);
		}
	}

	// set the map properties for mountains
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			if (map.tiles[x + y * map.width].type == Tile::Type::MOUNTAIN) {
				map.map->setProperties(x, y, true, true);
			}
		}
	}
	// Hack: move stairsUp off screen
	map.stairsUp->x = -1;
	map.stairsUp->y = -1;
	///////////////////////////////////
	// Add some caves
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
		do {
			x = map.rng->getInt(1, map.width - 1);
			y = map.rng->getInt(1, map.height - 1);
			tile = &map.tiles[x + y * map.width];
		} while (tile->type != Tile::Type::MOUNTAIN);
		std::string name = names[i];

		Actor *cave = new Actor(x, y, '*', name.c_str(), TCODColor::white);
		cave->fovOnly = false;
		cave->blocks = false;
		map.actors.push(cave);
		// Make the tile walkable
		map.tiles[x + y * map.width].type = Tile::Type::PLAIN;
		map.map->setProperties(x, y, true, true);
	}
	// Add some towns
	names.clear();
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
	int buildings = 8;
	for (int i = 0; i < buildings; i++) {
		int x, y;
		do {
			x = map.rng->getInt(6, map.width - 7);
			y = map.rng->getInt(6, map.height - 7);
		} while (!checkBuildingPlacement(map, x, y, 3, 3));
		placeBuilding(map, x, y, 3, 3);
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