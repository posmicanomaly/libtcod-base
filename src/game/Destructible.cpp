#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float defense, const char *corpseName, int xp) :
	maxHp(maxHp),hp(maxHp),defense(defense),xp(xp) {
	//std::cout << " Destructible() name: " << (corpseName == NULL ? "null" : corpseName) << std::endl;
	this->corpseName = _strdup(corpseName);
}

Destructible::~Destructible() {
	//std::cout << " ~Destructible()" << std::endl;
	//free((char *)corpseName);
	
}

float Destructible::takeDamage(Actor *owner, float damage) {
	damage -= defense;
	if ( damage > 0 ) {
		hp -= damage;
		if ( hp <= 0 ) {
			die(owner);
		}
	} else {
		damage=0;
	}
	return damage;
}

float Destructible::heal(float amount) {
	hp += amount;
	if ( hp > maxHp ) {
		amount -= hp-maxHp;
		hp=maxHp;
	}
	return amount;
}

void Destructible::die(Actor *owner) {
	// transform the actor into a corpse!
	owner->ch='%';
	owner->col=TCODColor::darkRed;	
	// The original name is longer needed, this destructible will assign the actor
	// a new name. So we have to free the old name or we'll leak it.
	free((char *)owner->name);
	//////////////////////////////////////////////////////////////////////////////
	owner->name=corpseName;
	owner->blocks=false;
	// make sure corpses are drawn before living actors
	engine.sendToBack(owner);
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName,int xp) :
	Destructible(maxHp,defense,corpseName,xp) {
}

void MonsterDestructible::die(Actor *owner) {
	// transform it into a nasty corpse! it doesn't block, can't be
	// attacked and doesn't move
	engine.gui->message(TCODColor::lightGrey,"%s is dead. You gain %d xp",
		owner->name, xp);
	engine.player->destructible->xp += xp;
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
	Destructible(maxHp,defense,corpseName,0) {
}

void PlayerDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::red,"You died!");
	Destructible::die(owner);
	engine.gameStatus=Engine::DEFEAT;
}
