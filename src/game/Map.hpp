class Tile {
public:
	bool explored; // has the player already seen this tile ?
	int variation;
	float temperature;
	float weather;
	int style;
	enum Type {
		// World map
		PLAIN, FOREST, MOUNTAIN, HILL, JUNGLE, DESERT, SHORE, GLACIER, TUNDRA, OCEAN, LAKE, SWAMP, RIVER,
		// Area maps
		FLOOR, WALL, GRASS, TREE, WATER_SHALLOW, WATER_DEEP
	} type;
	enum Effect {
		NONE, SCORCHED, BLOODY, FROZEN
	} effect;
	Tile () : explored (false), type (WALL), effect (NONE), style (0), temperature (0.0f) {}
	char *typeToChar ();
};

class Map : public Persistent {
public:
	int width, height;
	float heightMapMin;
	float heightMapMax;
	std::string name;
	// Use loading for, well, loading the map, the map's load function will put back the correct type.
	enum Type {
		LOADING, DUNGEON, WORLD, TOWN
	} type;
	Map (int width, int height, Type type);
	~Map ();
	bool isWall (int x, int y) const;
	bool isMountain (int x, int y) const;
	bool isInFov (int x, int y) const;
	bool isExplored (int x, int y) const;
	bool canWalk (int x, int y) const;
	void computeFov ();
	void render (TCODConsole *target) const;
	void getRandomCoords (int *x, int *y);
	Tile *getTile (int x, int y);
	TCODList<Actor *> actors;
	bool hasFeatureAt (Actor *owner, const char featureGlyph) const;
	bool hasFeatureAt (int x, int y, const char featureGlyph) const;
	Actor *getFeatureAt (Actor *owner, const char featureGlyph);
	Actor *stairs;
	Actor *stairsUp;

	void setFullyExplored ();

	void load (int level, std::string mapName);
	//not used
	void load (TCODZip &zip);
	void save (TCODZip &zip);
	////////////////////////
	void save ();
	void init (bool withActors);
	void setTileEffect (int x, int y, Tile::Effect effect);
	void shimmer ();

protected:
	Tile *tiles;
	TCODMap *map;
	long seed;
	TCODRandom *rng;

	friend class BspListener;
	friend class MapFactory;

	void dig (int x1, int y1, int x2, int y2);
	void createRoom (bool first, int x1, int y1, int x2, int y2, bool withActors);
	void addMonster (int x, int y);
	void addItem (int x, int y);
};
