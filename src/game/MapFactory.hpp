class MapFactory {
public:
	static void setMapTileProperties(Map &map);
	static void makeWorldMap(Map &map);
	static void makeTownMap(Map &map);
	static void makeDungeonMap(Map &map);

	static const float DESERT_TEMPERATURE;
	static const float JUNGLE_TEMPERATURE;
	static const float FREEZING_TEMPERATURE;

private:
	static void calcTemperatures(Map &map);
	static void addDeserts(Map &map, int divisor);
	static void addTrees(Map &map, int divisor);
	static void addRivers(Map &map);
	static void addCaves(Map &map);
	static void addTowns(Map &map);
	static void addFeatureSeed(Map &map, int x, int y, Tile::Type type, int minStrength, int maxStrength);
	static void fillWithType(Map &map, Tile::Type type);
	static void generateTownBuildings(Map &map);
	static bool checkBuildingPlacement(Map &map, int x, int y, int width, int height);
	static void placeBuilding(Map &map, int x, int y, int width, int height);
	static void placeBoundingWall(Map &map, int x, int y, int width, int height);
};