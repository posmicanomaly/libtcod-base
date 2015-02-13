class MapFactory {
public:
	static void makeWorldMap(Map &map);
	static void makeTownMap(Map &map);
private:
	static void addFeatureSeeds(Map &map, Tile::Type type, int amount);
	static void fillWithType(Map &map, Tile::Type type);
	static void generateTownBuildings(Map &map);
	static bool checkBuildingPlacement(Map &map, int x, int y, int width, int height);
	static void placeBuilding(Map &map, int x, int y, int width, int height);
	static void placeBoundingWall(Map &map, int x, int y, int width, int height);
};