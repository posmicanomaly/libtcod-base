struct Tile {
    bool explored; // has the player already seen this tile ?
	int variation;
	enum Type {
		FLOOR, WALL, GRASS, FOREST, MOUNTAIN, WATER
	} type;
    Tile() : explored(false), type(WALL) {}
};
 
class Map : public Persistent {
public :
    int width,height;
	std::string name;
	// Use loading for, well, loading the map, the map's load function will put back the correct type.
	enum Type {
		LOADING, DUNGEON, WORLD
	} type;
    Map(int width, int height, Type type);
    ~Map();
    bool isWall(int x, int y) const;
	bool isMountain(int x, int y) const;
	bool isInFov(int x, int y) const;
    bool isExplored(int x, int y) const;
    bool canWalk(int x, int y) const;
    void computeFov();
    void render() const;
	TCODList<Actor *> actors;
	bool hasCaveAt(Actor *owner) const;
	Actor *getCaveAt(Actor *owner);
	Actor *stairs;
	Actor *stairsUp;

	void setFullyExplored();

	void load(int level, std::string mapName);
	//not used
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	////////////////////////
	void save();
 	void init(bool withActors);
	
protected :
    Tile *tiles;
    TCODMap *map;
    long seed;
	TCODRandom *rng;

    friend class BspListener;
	friend class MapFactory;
 
    void dig(int x1, int y1, int x2, int y2);
    void createRoom(bool first, int x1, int y1, int x2, int y2,bool withActors);
    void addMonster(int x, int y);
    void addItem(int x, int y);
};
