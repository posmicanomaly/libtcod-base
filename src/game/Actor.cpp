#include <math.h>
#include "main.hpp"

Actor::Actor (int x, int y, int ch, const char *name,
			  const TCODColor &col) :
			  x (x), y (y), ch (ch), col (col),
			  blocks (true), fovOnly (true), attacker (NULL), destructible (NULL), ai (NULL),
			  pickable (NULL), container (NULL) {
	this->name = _strdup (name);
}

Actor::~Actor () {
	if (this == engine.player) {
		std::cout << "Deleting player" << std::endl;
	}
	if (name != NULL) {
		//std::cout << name << " ~Actor()" << std::endl;
	} else {
		//std::cout << " NULL ~Actor()" << std::endl;
	}
	if (attacker) delete attacker;
	if (destructible) {
		// If the corpse name is not equal to this actor's name
		// Then that means this actor isn't dead, and the destructible corpse name
		// has never been assigned to this actor, so we need to free the pointer to prevent
		// a leak.
		// Doing this here instead of destructor so I can compare names
		if (destructible->corpseName != this->name) {
			free ((char *)destructible->corpseName);
		}
		delete destructible;
	}
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete container;
	// This should always be a valid free, for instance if destructible calls die, in the
	// die function the old name is now freed.
	free ((char *)name);
}

void Actor::render (TCODConsole *target) const {
	/*
	Skew X and Y based on the offset for drawing
	*/
	int skewX = x;
	int skewY = y;
	engine.translateToView (skewX, skewY);
	target->setChar (skewX, skewY, ch);
	target->setCharForeground (skewX, skewY, col);
}

void Actor::update () {
	if (ai) ai->update (this);
}

float Actor::getDistance (int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrtf (dx*dx + dy*dy);
}
