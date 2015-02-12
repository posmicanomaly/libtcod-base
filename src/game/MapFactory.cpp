#include "main.hpp"

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