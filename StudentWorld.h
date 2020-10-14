#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <vector>
#include <string>

//helper functions
bool Overlap(int x1, int y1, int x2, int y2);
bool Moverlap(int x1, int y1, int x2, int y2);
int Distance(int x1, int y1, int x2, int y2);
int findAngle(int x1, int y1, int x2, int y2);

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();

	//functions for Socrates
	int playerHealth() const;
	void incHealth(int x);
	void playerDecHealth(int x);
	void incFlame(int x);
	int SocratesX() const;
	int SocratesY() const;

	//functions for food
	void createFood(int x, int y);
	bool isFoodNearby(int x, int y) const;
	int findFoodNearby(int x, int y);
	bool eatFood(int x, int y);

	//creating bacteria
	void createBacterium(int x, int y, int hp);

	//to determine whether an object is blocking movement at (x,y)
	bool isBlockAt(int x, int y) const;

	//projectile functions
	void addSpray(int x, int y, int direction);
	void addFlame(int x, int y, int direction);

	//functions to damage actors
	void hurtObjectAt(int x, int y, int dmg);
	bool DamageableObjectAt(int x, int y) const;

	//is the level over
	bool isLevelOver() const;

	~StudentWorld();

private:
	std::vector <Actor*> actors;
	Socrates* player;
};

#endif // STUDENTWORLD_H_
