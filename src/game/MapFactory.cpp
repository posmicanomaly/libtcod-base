#include "main.hpp"
void MapFactory::makeTownMap(Map &map) {
	// Fill with grass
	fillWithType(map, Tile::Type::GRASS);
	placeBoundingWall(map, 5, 5, map.width - 10, map.height - 10);
	generateTownBuildings(map);
}
void MapFactory::makeWorldMap(Map &map) {
	fillWithType(map, Tile::Type::GRASS);
	addFeatureSeeds(map, Tile::Type::FOREST, map.rng->getInt(10, 50));
	addFeatureSeeds(map, Tile::Type::MOUNTAIN, map.rng->getInt(20, 100));
	addFeatureSeeds(map, Tile::Type::WATER, map.rng->getInt(10, 50));
	// set the map properties for mountains
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			if (map.tiles[x + y * map.width].type == Tile::Type::MOUNTAIN) {
				map.map->setProperties(x, y, true, false);
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
		map.tiles[x + y * map.width].type = Tile::Type::GRASS;
		map.map->setProperties(x, y, true, true);
	}
	// Add some towns
	names.clear();
	names.push_back("Town 1");
	names.push_back("Town 2");
	names.push_back("Town 3");
	for (int i = 0; i < names.size(); i++) {
		Tile *tile;
		int x, y;
		do {
			x = map.rng->getInt(1, map.width - 1);
			y = map.rng->getInt(1, map.height - 1);
			tile = &map.tiles[x + y * map.width];
		} while (tile->type != Tile::Type::GRASS && map.hasFeatureAt(x, y, '*'));
		std::string name = names[i];

		Actor *town = new Actor(x, y, 'O', name.c_str(), TCODColor::white);
		town->fovOnly = false;
		town->blocks = false;
		map.actors.push(town);
		// Make the tile walkable
		map.tiles[x + y * map.width].type = Tile::Type::GRASS;
		map.map->setProperties(x, y, true, true);
	}
}
/*
Adds <amount> of <type> to <map>
for each amount, use a random strength to spread it around
*/
void MapFactory::addFeatureSeeds(Map &map, Tile::Type type, int amount) {
	// Place seeds
	for (int i = 0; i < amount; i++) {
		// random x,y
		int x = map.rng->getInt(1, map.width - 1);
		int y = map.rng->getInt(1, map.height - 1);
		
		// set tile to type
		map.tiles[x + y * map.width].type = type;
		
		// strength is how many times it will try and continue to spread
		int strength = map.rng->getInt(10, 50);
		
		// init nextx and nexty to the base seed
		int nextx = x;
		int nexty = y;

		// init directional modifiers
		int xd;
		int yd;

		// Grow seed
		for (int s = 0; s < strength; s++) {
			// Get a random direction while the nextx, nexty is out of range
			do {
				xd = map.rng->getInt(-1, 1);
				yd = map.rng->getInt(-1, 1);
			} while ((nextx + xd < 0 || nextx + xd >= map.width) || (nexty + yd < 0 || nexty + yd >= map.height));
			
			// nextx, nexty will be in range, add the directional modifiers to nextx, nexty
			nextx += xd;
			nexty += yd;
			
			// set the nextx, nexty tile to type
			map.tiles[nextx + nexty * map.width].type = type;
		}
	}
}

void MapFactory::fillWithType(Map &map, Tile::Type type) {
	bool transparent = true;
	bool walkable = true;
	switch (type) {
	case Tile::Type::MOUNTAIN: walkable = false; break;
	case Tile::Type::GRASS: break;
	case Tile::Type::WATER: break;
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
	int buildings = 2;
	for (int i = 0; i < buildings; i++) {
		int x, y;
		do {
			x = map.rng->getInt(map.width / 8, map.width - map.width / 8);
			y = map.rng->getInt(map.height / 8, map.height - map.height / 8);
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