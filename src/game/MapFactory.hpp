class MapFactory {
public:
	static void makeWorldMap(Map &map);
private:
	static void addFeatureSeeds(Map &map, Tile::Type type, int amount);

};