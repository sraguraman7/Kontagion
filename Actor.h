#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

/*GraphObject(int imageID,
	int startX, // column first
	int startY, // then row!
	Direction startDirection,
	int depth)*/
class StudentWorld;
void newxy(int x, int y, int& newx, int& newy);
class Actor : public GraphObject
{
public:
	Actor(int image, int startx, int starty, int startd, int depth, StudentWorld* sw, bool canBlock, bool isDamageable, int hp, bool isEdible, bool dangerous);
	virtual void doSomething() = 0;
	StudentWorld* arena() const;

	bool isAlive() const;	//is it alive
	void setStatus(bool life);	//change life status
	
	int health() const;	//return health
	void incHealth(int x);	//increase health of the object by x
	virtual void decHealth(int dmg);	//decrease health by dmg

	bool isEdible() const;	//is it edible
	bool isDangerous() const;	//is it dangerous
	bool isDamageable() const; // is it damaged by projectiles
	bool canItBlock() const;	//can it block movement   **'it' refers to the object

	virtual ~Actor();
private:
	bool m_alive;
	bool m_dangerous;
	bool m_isdamageable;
	bool m_canBlock;
	int m_hp;
	bool m_edible;
	StudentWorld* m_arena;
};

class Bacterium : public Actor
{
public:
	Bacterium(int image, int x, int y, int hp, StudentWorld* sw);
	int movePlan() const;		//moveplan
	void incMovePlan(int x);	//increase moveplan by x
	void resetMovePlan(int x);	//reset moveplan to x
	int foodEaten() const;	//how much food has been eaten
	void incFood();	//increase food count
	void resetFood();	//reset count to 0
	bool canMoveForward(int dist, int direction, Bacterium* b) const;	//can object move forward
	virtual ~Bacterium();
private:
	int m_movePlan;
	int m_foodEaten;
};

class Ecoli : public Bacterium
{
public:
	Ecoli(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual void decHealth(int dmg);
	virtual ~Ecoli();
};

class Salmonella : public Bacterium
{
public:
	Salmonella(int x, int y, int hp, StudentWorld* sw);
	virtual void decHealth(int dmg);
	void finishMove();
	virtual ~Salmonella();
private:
};

class RegularSalmonella : public Salmonella
{
public:
	RegularSalmonella(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~RegularSalmonella();
};

class AggressiveSalmonella : public Salmonella
{
public:
	AggressiveSalmonella(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~AggressiveSalmonella();
};

class Projectile : public Actor
{
public:
	Projectile(int image, int x, int y, int direction, int distance, int damage, StudentWorld* sw);
	void incDistance();
	int dmg() const;
	int Distance() const;
	virtual void doSomething();
	virtual ~Projectile();
private:
	int m_distance;
	int m_dmg;
};

class Flame : public Projectile
{
public:
	Flame(int x, int y, int direction, StudentWorld* sw);
	virtual ~Flame();
private:
};

class Spray : public Projectile
{
public:
	Spray(int x, int y, int direction, StudentWorld* sw);
	virtual ~Spray();
private:
};

class Goodie : public Actor
{
public:
	Goodie(int image, int startx, int starty, StudentWorld* sw);
	void updateTime();
	int timeLeft() const;
	virtual ~Goodie();
private:
	int m_time;
};

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~ExtraLifeGoodie();
private:
};
class HealthGoodie : public Goodie
{
public:
	HealthGoodie(int startx, int starty, StudentWorld* sw);
	virtual void doSomething();
	virtual ~HealthGoodie();
private:
};

class FlameGoodie : public Goodie
{
public:
	FlameGoodie(int startx, int starty, StudentWorld* sw);
	virtual void doSomething();
	virtual ~FlameGoodie();
private:
};

class Socrates: public Actor
{
public:
	Socrates(StudentWorld* sw);
	virtual void doSomething();
	int getSpray() const;
	int getFlame() const;
	virtual void decHealth(int dmg);
	void incFlame(int x);
	virtual ~Socrates();
private:
	int m_spray;
	int m_flame;
};

class Dirt : public Actor
{
public:
	Dirt(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~Dirt();
private:
};

class Food : public Actor
{
public:
	Food(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~Food();
private:
};

class Pit : public Actor
{
public:
	Pit(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	int bacteriaLeft() const;
	virtual ~Pit();
private:
	int m_salmonella;
	int m_aggro;
	int m_ecoli;
};

class Fungus : public Goodie
{
public:
	Fungus(int x, int y, StudentWorld* sw);
	virtual void doSomething();
	virtual ~Fungus();
private:
};
#endif // ACTOR_H_
