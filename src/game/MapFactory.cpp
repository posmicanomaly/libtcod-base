#include "main.hpp"

void MapFactory::addFeatureSeeds(Map &map, Tile::Type type, int amount) {
	// Place seeds
	std::cout << "Placing seeds" << std::endl;
	for (int i = 0; i < amount; i++) {
		int x = map.rng->getInt(1, map.width - 1);
		int y = map.rng->getInt(1, map.height - 1);
		map.tiles[x + y * map.width].type = type;
		// Grow seed
		int strength = map.rng->getInt(10, 50);
		int nextx = x;
		int nexty = y;
		int xd;
		int yd;
		for (int s = 0; s < strength; s++) {
			do {
				xd = map.rng->getInt(-1, 1);
				yd = map.rng->getInt(-1, 1);
			} while ((nextx + xd < 0 || nextx + xd >= map.width) || (nexty + yd < 0 || nexty + yd >= map.height));
			nextx += xd;
			nexty += yd;
			map.tiles[nextx + nexty * map.width].type = type;
		}
	}
}