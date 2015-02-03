class Attacker : public Persistent {
public :
	float power; // hit points given

	Attacker(float power);
	void attack(Actor *owner, Actor *target);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};
