class Menu {
public:
	enum MenuItemCode {
		NONE,
		NEW_GAME,
		CONTINUE,
		EXIT,
		CONSTITUTION,
		STRENGTH,
		AGILITY
	};
	enum DisplayMode {
		MAIN,
		PAUSE
	};
	~Menu ();
	void clear ();
	void addItem (MenuItemCode code, const char *label);
	MenuItemCode pick (DisplayMode mode = MAIN);
protected:
	struct MenuItem {
		MenuItemCode code;
		const char *label;
	};
	TCODList<MenuItem *> items;
};

class Gui : public Persistent {
public:
	Menu menu;

	Gui ();
	~Gui ();
	void render ();
	void message (const TCODColor &col, const char *text, ...);
	void load (TCODZip &zip);
	void save (TCODZip &zip);
	void clear ();
	static const int LEFT_PANEL_WIDTH = 14;
	static const int RIGHT_PANEL_WIDTH = 14;
	static const int MESSAGE_PANEL_HEIGHT = 14;
protected:
	TCODConsole *con;
	TCODConsole *left;
	TCODConsole *right;
	struct Message {
		char *text;
		TCODColor col;
		Message (const char *text, const TCODColor &col);
		~Message ();
	};
	TCODList<Message *> log;
	void renderBar (TCODConsole *target, int x, int y, int width, const char *name,
					float value, float maxValue, const TCODColor &barColor,
					const TCODColor &backColor);
	void renderMouseLook (int x, int y);
	void renderLeftPanel ();
	void renderRightPanel ();
	void renderMessagePanel ();
};
