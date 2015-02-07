class Engine {
public :
	enum GameStatus {
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT
	} gameStatus;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;
    
    Actor *player;
    
    Map *map;
    int fovRadius;
    int screenWidth;
    int screenHeight;
    Gui *gui;
    int level;

    Engine(int screenWidth, int screenHeight);
    ~Engine();
    void update();
    void render();
    void sendToBack(Actor *actor);
    Actor *getActor(int x, int y) const;
    Actor *getClosestMonster(int x, int y, float range) const;
    bool pickATile(int *x, int *y, float maxRange = 0.0f);
	void setFullyExplored();
	void changeLevel(signed int direction);
    void nextLevel();
	void previousLevel();
   	void load(bool pause=false);
	bool mapExists(int level);
	void clearMapFiles();
	
	void save();
	
    void init();
    void term();
};
 
extern Engine engine;
