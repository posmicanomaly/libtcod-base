#include "main.hpp"

void MapFactory::makeWorldMap(Map &map) {
	// Fill with grass
	for (int x = 0; x < map.width; x++) {
		for (int y = 0; y < map.height; y++) {
			map.tiles[x + y * map.width].type = Tile::Type::GRASS;
			// If we don't do this, fov can't compute
			map.map->setProperties(x, y, true, true);
		}
	}
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
	names.push_back("Generic Caverns of ASCII");
	names.push_back("Similar Dungeon of R");
	names.push_back("Terrific Example of Dungeon");
	names.push_back("Another Dungeon");
	names.push_back("Dungeon of Dismay");
	names.push_back("Caves of Clacking");
	for (int i = 0; i < names.size(); i++) {
		Tile *tile;
		int x, y;
		do {
			x = map.rng->getInt(1, map.width);
			y = map.rng->getInt(1, map.height);
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