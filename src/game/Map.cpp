#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;

class BspListener : public ITCODBspCallback {
private :
    Map &map; // a map to dig
    int roomNum; // room number
    int lastx,lasty; // center of the last room
public :
    BspListener(Map &map) : map(map), roomNum(0) {}
    bool visitNode(TCODBsp *node, void *userData) {
    	if ( node->isLeaf() ) {    
    		int x,y,w,h;
			// dig a room
			bool withActors=(bool)userData;
			w=map.rng->getInt(ROOM_MIN_SIZE, node->w-2);
			h=map.rng->getInt(ROOM_MIN_SIZE, node->h-2);
			x=map.rng->getInt(node->x+1, node->x+node->w-w-1);
			y=map.rng->getInt(node->y+1, node->y+node->h-h-1);
			map.createRoom(roomNum == 0, x, y, x+w-1, y+h-1, withActors);
			if ( roomNum != 0 ) {
			    // dig a corridor from last room
			    map.dig(lastx,lasty,x+w/2,lasty);
			    map.dig(x+w/2,lasty,x+w/2,y+h/2);
			}
            lastx=x+w/2;
            lasty=y+h/2;
            roomNum++;
        }
        return true;
    }
};

Map::Map(int width, int height, Type type) 
	: width(width),height(height), type(type), name("default map") {
	std::cout << "Map()" << std::endl;
	seed=TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
	// Down stairs
	stairs = new Actor(0, 0, '>', "stairs", TCODColor::white);
	stairs->blocks = false;
	stairs->fovOnly = false;
	actors.push(stairs);
	// Up stairs
	stairsUp = new Actor(0, 0, '<', "stairs", TCODColor::white);
	stairsUp->blocks = false;
	stairsUp->fovOnly = false;
	actors.push(stairsUp);
}

void Map::init(bool withActors) {
	rng = new TCODRandom(seed, TCOD_RNG_CMWC);
    tiles = new Tile[width*height];
	// Give all tiles a variation to make them appear slightly different from each other
	// (more visually appealing, no affect on gameplay)
	for (int i = 0; i < width * height; i++) {
		tiles[i].variation = rng->getInt(1, 32);
	}
	/////////////////////////////////////////////////////////////////////////////////////
    map = new TCODMap(width,height);
	
	/*
	Not sure how I want to refactor the dungeon generation yet
	So for now, MapFactory will take care of all the new generation additions
	*/

	// Dungeon
	if (type == Type::DUNGEON) {
		TCODBsp bsp(0, 0, width, height);
		bsp.splitRecursive(rng, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
		BspListener listener(*this);
		bsp.traverseInvertedLevelOrder(&listener, (void *)withActors);
	}

	// World
	else if(type == Type::WORLD) {
		MapFactory::makeWorldMap(*this);
		name = "world";
	}
}

Map::~Map() {
	std::cout << " ~Map()" << std::endl;
	// Added delete rng, memory leak
	delete rng;
	////////////////////////////////
    delete [] tiles;
    delete map;
}

void Map::dig(int x1, int y1, int x2, int y2) {
    if ( x2 < x1 ) {
        int tmp=x2;
        x2=x1;
        x1=tmp;
    }
    if ( y2 < y1 ) {
        int tmp=y2;
        y2=y1;
        y1=tmp;
    }
    for (int tilex=x1; tilex <= x2; tilex++) {
        for (int tiley=y1; tiley <= y2; tiley++) {
            map->setProperties(tilex,tiley,true,true);
			// Set the tile type
			tiles[tilex + tiley * width].type = Tile::Type::FLOOR;
        }
    }
}

void Map::addMonster(int x, int y) {
	TCODRandom *rng=TCODRandom::getInstance();
    if ( rng->getInt(0,100) < 80 ) {
        // create an orc
        Actor *orc = new Actor(x,y,'o',"orc",
            TCODColor::desaturatedGreen);
        orc->destructible = new MonsterDestructible(10,0,"dead orc",35);
        orc->attacker = new Attacker(3);
        orc->ai = new MonsterAi();
        actors.push(orc);
    } else {
        // create a troll
        Actor *troll = new Actor(x,y,'T',"troll",
             TCODColor::darkerGreen);
        troll->destructible = new MonsterDestructible(16,1,"troll carcass",100);
        troll->attacker = new Attacker(4);
        troll->ai = new MonsterAi();
        actors.push(troll);
    }
}

void Map::addItem(int x, int y) {
	TCODRandom *rng=TCODRandom::getInstance();
	int dice = rng->getInt(0,100);
	if ( dice < 70 ) {
		// create a health potion
		Actor *healthPotion=new Actor(x,y,'!',"health potion",
			TCODColor::violet);
		healthPotion->blocks=false;
		healthPotion->pickable=new Healer(4);
		actors.push(healthPotion);
	} else if ( dice < 70+10 ) {
		// create a scroll of lightning bolt 
		Actor *scrollOfLightningBolt=new Actor(x,y,'#',"scroll of lightning bolt",
			TCODColor::lightYellow);
		scrollOfLightningBolt->blocks=false;
		scrollOfLightningBolt->pickable=new LightningBolt(5,20);
		actors.push(scrollOfLightningBolt);
	} else if ( dice < 70+10+10 ) {
		// create a scroll of fireball
		Actor *scrollOfFireball=new Actor(x,y,'#',"scroll of fireball",
			TCODColor::lightYellow);
		scrollOfFireball->blocks=false;
		scrollOfFireball->pickable=new Fireball(3,12);
		actors.push(scrollOfFireball);
	} else {
		// create a scroll of confusion
		Actor *scrollOfConfusion=new Actor(x,y,'#',"scroll of confusion",
			TCODColor::lightYellow);
		scrollOfConfusion->blocks=false;
		scrollOfConfusion->pickable=new Confuser(10,8);
		actors.push(scrollOfConfusion);
	}
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors) {
    dig (x1,y1,x2,y2);
    if (!withActors) {
    	return;
    }      
    if ( first ) {
		// We will insert the player manually
        //// put the player in the first room
        //engine.player->x=(x1+x2)/2;
        //engine.player->y=(y1+y2)/2;
		// Add stairs going up in first room
		stairsUp->x = (x1 + x2) / 2;
		stairsUp->y = (y1 + y2) / 2;
		///////////////////////////////////////
    } else {
		TCODRandom *rng=TCODRandom::getInstance();
		// add monsters
		int nbMonsters=rng->getInt(0,MAX_ROOM_MONSTERS);
		while (nbMonsters > 0) {
		    int x=rng->getInt(x1,x2);
		    int y=rng->getInt(y1,y2);
    		if ( canWalk(x,y) ) {
				addMonster(x,y);
			}
		    nbMonsters--;
		}
		// add items
		int nbItems=rng->getInt(0,MAX_ROOM_ITEMS);
		while (nbItems > 0) {
		    int x=rng->getInt(x1,x2);
		    int y=rng->getInt(y1,y2);
    		if ( canWalk(x,y) ) {
				addItem(x,y);
			}
		    nbItems--;
		}
		// set stairs position
		stairs->x=(x1+x2)/2;
		stairs->y=(y1+y2)/2;
    }
}

bool Map::hasCaveAt(Actor *owner) const{
	for (Actor **iterator = actors.begin();
		iterator != actors.end();
		iterator++) {
		Actor *actor = *iterator;
		if (actor->x == owner->x && actor->y == owner->y) {
			// Hack
			if (actor->ch == '*') {
				std::cout << "cave: " << actor->x << ", " << actor->y << " owner: " << owner->x << ", " << owner->y << std::endl;
				return true;
			}
		}
	}
	return false;
}

Actor *Map::getCaveAt(Actor *owner) {
	for (Actor **iterator = actors.begin();
		iterator != actors.end();
		iterator++) {
		Actor *actor = *iterator;
		if (actor->x == owner->x && actor->y == owner->y) {
			// Hack
			if (actor->ch == '*') {
				std::cout << "cave: " << actor->x << ", " << actor->y << " owner: " << owner->x << ", " << owner->y << std::endl;
				return actor;
			}
		}
	}
	return NULL;
}

bool Map::isWall(int x, int y) const {
    //return !map->isWalkable(x,y);
	return tiles[x + y * width].type == Tile::Type::WALL;
}

bool Map::canWalk(int x, int y) const {
	/*if (x < 0 || x >= width) {
		return false;
	}
	if (y < 0 || y >= height) {
		return false;
	}*/
    if (isWall(x,y)) {
        // this is a wall
        return false;
    }
    for (Actor **iterator=actors.begin();
        iterator!=actors.end();iterator++) {
        Actor *actor=*iterator;
        if ( actor->blocks && actor->x == x && actor->y == y ) {
            // there is a blocking actor here. cannot walk
            return false;
        }
    }
    return true;
}
 
bool Map::isExplored(int x, int y) const {
    return tiles[x+y*width].explored;
}

void Map::setFullyExplored() {
	for (int i = 0; i < width * height; i++) {
		tiles[i].explored = true;
	}
}

bool Map::isInFov(int x, int y) const {
	if ( x < 0 || x >= width || y < 0 || y >= height ) {
		return false;
	}
    if ( map->isInFov(x,y) ) {
        tiles[x+y*width].explored=true;
        return true;
    }
    return false;
}
 
void Map::computeFov() {
    map->computeFov(engine.player->x,engine.player->y,
        engine.fovRadius);
}

void Map::render() const {
	/*
	If using tile variation to affect the final color, using 255 will prevent that from occuring
	*/
    static const TCODColor wall(TCODColor(40, 30, 30));
	static const TCODColor floor(TCODColor(5, 5, 10));
	static const TCODColor grass(TCODColor(0, 40, 0));
	static const TCODColor forest(TCODColor(20, 70, 0));
	static const TCODColor mountain(TCODColor(100, 50, 0));
	static const TCODColor water(TCODColor(0, 0, 100));

	static const TCODColor lightFore(TCODColor::white);
	static const TCODColor darkFore(TCODColor::black);

	for (int x=0; x < width; x++) {
	    for (int y=0; y < height; y++) {
			char glyph = '?';
			TCODColor backColor = wall;
			TCODColor foreColor = lightFore;
			switch (tiles[x + y * width].type) {
			case Tile::Type::FLOOR:	glyph = '.'; backColor = floor; foreColor = lightFore; break;
			case Tile::Type::WALL:	glyph = '#'; backColor = wall; foreColor = darkFore; break;
			case Tile::Type::GRASS:	glyph = '"'; backColor = grass; foreColor = darkFore; break;
			case Tile::Type::FOREST: glyph = '&'; backColor = forest; foreColor = darkFore; break;
			case Tile::Type::MOUNTAIN: glyph = '^'; backColor = mountain; foreColor = darkFore; break;
			case Tile::Type::WATER: glyph = '~'; backColor = water; foreColor = darkFore; break;
			}
			// If its not the world map
			// Adjust brightness based on variation
			// variation is currently set to be 1 - 10, so this gives us variation / 10 + 1, 10 would be twice the brightness
			if (type != Type::WORLD) {
				backColor = backColor * (float)((tiles[x + y * width].variation / 10) + 1);
			}
	        if ( isInFov(x,y) ) {
				TCODConsole::root->setChar(x, y, glyph);
	            TCODConsole::root->setCharBackground(x,y,
	                backColor);
				TCODConsole::root->setCharForeground(x, y, foreColor);
	        } else if ( isExplored(x,y) ) {
				backColor = backColor * 0.5f;
				foreColor = foreColor * 0.5f;
				TCODConsole::root->setChar(x, y, glyph);
				TCODConsole::root->setCharBackground(x, y,
					backColor);
				TCODConsole::root->setCharForeground(x, y, foreColor);
	        }
   	    }
	}
}
