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
	int updateCount = 0;
    Actor *player;
    
    Map *map;
    int fovRadius;
    int screenWidth;
    int screenHeight;
	const int VIEW_WIDTH = 80;
	const int VIEW_HEIGHT = 43;
	int mouse_winX;
	int mouse_winY;
	int mouse_mapX;
	int mouse_mapY;
	int xOffset;
	int yOffset;
    Gui *gui;
    int level;

    Engine(int screenWidth, int screenHeight);
    ~Engine();
	void init();
	void translateMouseToView();
	void update();
    void render();
    
	void sendToBack(Actor *actor);
    
	Actor *getActor(int x, int y) const;
    Actor *getClosestMonster(int x, int y, float range) const;
    
	bool pickATile(int *x, int *y, float maxRange = 0.0f);
	
	void setFullyExplored();

	void changeLevel(signed int direction, Actor *actor = NULL);	
   	
	bool mapExists(int level, std::string mapName);
	void clearMapFiles();

	void translateToView(int &x, int &y);
	
	// Defined in Persistent.cpp
	void load(bool pause = false);
	void loadContinueHelper();
	void save();	    
	////////////////////////////

    void term();
};
 
extern Engine engine;
