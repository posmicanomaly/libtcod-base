#include <math.h>
#include "main.hpp"

Actor::Actor(int x, int y, int ch, const char *name,
	const TCODColor &col) :
	x(x), y(y), ch(ch), col(col),
	blocks(true), fovOnly(true), attacker(NULL), destructible(NULL), ai(NULL),
	pickable(NULL), container(NULL) {
	this->name = _strdup(name);
}

Actor::~Actor() {
	if (name != NULL) {
		//std::cout << name << " ~Actor()" << std::endl;
	}
	else {
		//std::cout << " NULL ~Actor()" << std::endl;
	}
	if (attacker) delete attacker;
	if (destructible) {
		if (destructible->corpseName != this->name) {
			free((char *)destructible->corpseName);
		}
		delete destructible;
	}
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete container;
	free((char *)name);
}

void Actor::render() const {
	TCODConsole::root->setChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, col);
}

void Actor::update() {
	if (ai) ai->update(this);
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrtf(dx*dx + dy*dy);
}
