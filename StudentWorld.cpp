#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
using namespace std;

class AggressiveSalmonella;
class RegularSalmonella;

bool Overlap(int x1, int y1, int x2, int y2)	//determine if 2 points overlap
{
	if (Distance(x1, y1, x2, y2) <= SPRITE_WIDTH)
		return true;
	else return false;
}

bool Moverlap(int x1, int y1, int x2, int y2)	//movement overlap
{
	if (Distance(x1, y1, x2, y2) <= SPRITE_WIDTH / 2)
		return true;
	else return false;
}


int Distance(int x1, int y1, int x2, int y2)	//distance between 2 points
{
	int distance_x = x2 - x1;
	int distance_y = y2 - y1;

	double distance = (distance_x * distance_x) + (distance_y * distance_y);
	distance = sqrt(distance);
	int x = round(distance);
	return x;
}

int findAngle(int x1, int y1, int x2, int y2) //(x2, y2) is goal object, find angle between them
{
	int x = x2 - x1;
	int y = y2 - y1;

	if (x == 0)
	{
		if (y2 > y1)
			return 0;
		else return 180;
	}

	if (y == 0)
	{
		if (x2 > x1)
			return 90;
		else return 270;
	}
	
	
	double theta = atan2(y, x);
	theta = theta * 180 / (3.1415926535);

	int answer = round(theta);
	return answer;
}

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
	player = nullptr;
}

int StudentWorld::init()
{
	player = new Socrates(this);		//new Socrates

	int pitlimit = getLevel();
	for (int i = 0; i != pitlimit; i++)	//randomly create pits within bounds
	{
		int pitx = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
		int pity = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
	
	if ((Distance(128, 128, pitx, pity) > 120))
	{
		i--;
		continue;
	}
	bool overlap = false;
	for (int k = 0; k != actors.size(); k++)		//if an overlap is created, don't place pit
	{
		if (Overlap(actors[k]->getX(), actors[k]->getY(), pitx, pity))
		{
			overlap = true;
			break;
		}
	}
	if (overlap)
	{
		i--;
		continue;
	}
	actors.push_back(new Pit(pitx, pity, this));		//new pit
	}

	int foodlimit;		//similar process for food
	if (5 * getLevel() < 25)
		foodlimit = 5 * getLevel();
	else foodlimit = 25;
	for (int i = 0; i != foodlimit; i++)
	{
		int foodx = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
		int foody = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
		if ((Distance(128, 128, foodx, foody) > 120))
		{
			i--;
			continue;
		}
		bool overlap = false;
		for (int k = 0; k != actors.size(); k++)
		{
			if (Overlap(actors[k]->getX(), actors[k]->getY(), foodx, foody))
			{
				overlap = true;
				break;
			}
		}
		if (overlap)
		{
			i--;
			continue;
		}
		actors.push_back(new Food(foodx, foody, this));
	}

	int dirtlimit;		//almost similar process for dirt
	if (180 - 20 * getLevel() >= 20)
		dirtlimit = 180 - 20 * getLevel();
	else dirtlimit = 20;
	for (int i = 0; i != dirtlimit; i++)
	{
		int dirtx = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
		int dirty = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
		if (Distance(128, 128, dirtx, dirty) > 120)
		{
			i--;
			continue;
		}
		bool overlap = false;
		for (int k = 0; k != actors.size(); k++)
		{
			if (!actors[k]->canItBlock())		//if the actor cannot block, check for Overlap. Otherwise, skip this step
			{
				if (Overlap(actors[k]->getX(), actors[k]->getY(), dirtx, dirty))
				{
					overlap = true;
					break;
				}
			}
		}
		if (overlap)
		{
			i--;
			continue;
		}
		actors.push_back(new Dirt(dirtx, dirty, this));
	}

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	player->doSomething();		//Socrates goes first
	vector<Actor*>::iterator it = actors.begin();
	while (it != actors.end())		//delete dead actors
	{
		if (!((*it)->isAlive()))
		{
			delete *it;
			actors.erase(it);
			it = actors.begin();
			continue;
		}
		it++;
	}
	for(int i = 0; i != actors.size(); i++)
	{
			actors[i]->doSomething();		//do Something for everyone
			if (playerHealth() <= 0)
			{
				decLives();		//lower lives and return dead
				return GWSTATUS_PLAYER_DIED;
			}
			else if (isLevelOver())		//if level is over, finish level
			{
				playSound(SOUND_FINISHED_LEVEL);
				return GWSTATUS_FINISHED_LEVEL;
			}
	}
	it = actors.begin();
	while (it != actors.end())		//delete dead actors after all doSomethings are called
	{
		if (!((*it)->isAlive()))
		{
			delete* it;
			actors.erase(it);
			it = actors.begin();
			continue;
		}
		it++;
	}

	int ChanceFungus = 200;		//give fungi a chance to spawn
	if (500 - getLevel() * 10 > ChanceFungus)
		ChanceFungus = 500 - getLevel() * 10;
	int p = randInt(0, ChanceFungus - 1);
	if (p == 0)
	{
		int x = 0;
		int y = 0;
		for (;;)		//make sure fungus is on radius
		{
			x = randInt(VIEW_WIDTH / 2 - VIEW_RADIUS, VIEW_WIDTH / 2 + VIEW_RADIUS);
			y = randInt(VIEW_HEIGHT / 2 - VIEW_RADIUS, VIEW_HEIGHT / 2 + VIEW_RADIUS);
			if (Distance(x, y, VIEW_WIDTH / 2, VIEW_HEIGHT / 2) != VIEW_RADIUS)
				continue;
			else break;
		}
		actors.push_back(new Fungus(x, y, this));
	}

	int ChanceGoodie = 250;		//give goodies a chance to spawn
	if (500 - getLevel() * 10 > ChanceGoodie)
		ChanceGoodie = 500 - getLevel() * 10;
	int q = randInt(0, ChanceGoodie - 1);
	if (q == 0)
	{
		int x;
		int y;
		for (;;)		//make sure the goodie is on the radius
		{
			x = randInt(VIEW_WIDTH / 2 - VIEW_RADIUS, VIEW_WIDTH / 2 + VIEW_RADIUS);
			y = randInt(VIEW_WIDTH / 2 - VIEW_RADIUS, VIEW_WIDTH / 2 + VIEW_RADIUS);
			if (Distance(x, y, VIEW_WIDTH / 2, VIEW_HEIGHT / 2) != VIEW_RADIUS)
				continue;
			else break;
		}
		int r = randInt(0, 99);		//randomly choose the goodie
		if (r < 60)
			actors.push_back(new HealthGoodie(x, y, this));
		else if (r < 90)
			actors.push_back(new FlameGoodie(x, y, this));
		else actors.push_back(new ExtraLifeGoodie(x, y, this));
	}
	ostringstream displayText;		//display text
	displayText << "Score: ";
	displayText.fill('0');
	if (getScore() >= 0)
	{
		displayText << setw(6) << getScore() << "  " << "Level: " << getLevel() << "  " << "Lives: " << getLives() << "  " << "Health: " << player->health() << "  " << "Sprays: " << player->getSpray() << "  " << "Flames: " << player->getFlame();
	}
	else
	{
		int score = getScore();
		score = score * (-1);
		displayText << "-" << setw(5) << score << "  " << "Level: " << getLevel() << "  " << "Lives: " << getLives() << "  " << "Health: " << player->health() << "  " << "Sprays: " << player->getSpray() << "  " << "Flames: " << player->getFlame();
	}
	string text = displayText.str();		//turn it to a string
	setGameStatText(text);

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()		
{
	for (int i = 0; i != actors.size(); i++)		//delete all actors
		delete actors[i];
	actors.clear();		//clear the vector
	delete player;		//delete Socrates
	player = nullptr;		//set to nullptr
}

int StudentWorld::playerHealth() const
{
	return player->health();		//get Soc health
}

void StudentWorld::incHealth(int x)
{
	player->incHealth(x);		//increase Soc Health
}

void StudentWorld::playerDecHealth(int x)
{
	player->decHealth(x);		//decrease Soc Health
}

void StudentWorld::incFlame(int x)
{
	player->incFlame(x);		//increase Soc flame charges
}

int StudentWorld::SocratesX() const
{
	return player->getX();		//get Soc X
}

int StudentWorld::SocratesY() const
{
	return player->getY();	//get Soc Y
}

void StudentWorld::createFood(int x, int y)
{
	actors.push_back(new Food(x, y, this));		//dynamically allocate a new food
}

bool StudentWorld::isFoodNearby(int x, int y) const
{
	for (int i = 0; i != actors.size(); i++)		//if food is within 128 pixels
		if (actors[i]->isEdible())
			if (Distance(x, y, actors[i]->getX(), actors[i]->getY()) <= 128)
				return true;
	return false;
}

int StudentWorld::findFoodNearby(int x, int y)
{
	for (int i = 0; i != actors.size(); i++)
		if (actors[i]->isEdible())
			if (Distance(x, y, actors[i]->getX(), actors[i]->getY()) <= 128)	//find first food within 128 units
			{
				int angle = 0;
				angle = findAngle(x, y, actors[i]->getX(), actors[i]->getY());		//find angle between food and passed object
				return angle;
			}
	return 0;
}

bool StudentWorld::eatFood(int x, int y)
{
	for (int i = 0; i != actors.size(); i++)
	{
		if (Overlap(actors[i]->getX(), actors[i]->getY(), x, y))	//if an edible object is overlapped, delete the food
			if (actors[i]->isEdible())
			{
				actors[i]->setStatus(false);
				return true;
			}
	}
	return false;
}

void StudentWorld::createBacterium(int x, int y, int hp)	//push back new bacteria based on hp value
{
	if (hp == 4)
		actors.push_back(new RegularSalmonella(x, y, this));	//pick bacteria based on hp
	else if (hp == 5)
		actors.push_back(new Ecoli(x, y, this));
	else
		actors.push_back(new AggressiveSalmonella(x, y, this));
}

bool StudentWorld::isBlockAt(int x, int y) const
{
	for (int i = 0; i != actors.size(); i++)
	{
		if (Moverlap(actors[i]->getX(), actors[i]->getY(), x, y))	//if there's a movement overlap caused by a blockable object
		{
			if (actors[i]->canItBlock())
				return true;
		}
	}
	return false;
}


void StudentWorld::addSpray(int x, int y, int direction)
{
	actors.push_back(new Spray(x, y, direction, this));		//create new spray
}

void StudentWorld::addFlame(int x, int y, int direction)
{
	actors.push_back(new Flame(x, y, direction, this));	//create new flame
}

void StudentWorld::hurtObjectAt(int x, int y, int dmg)
{
	for (int i = 0; i != actors.size(); i++)
		if (Overlap(x, y, actors[i]->getX(), actors[i]->getY()))	//if object is damageable, decHealth
		{
			if (actors[i]->isAlive() && actors[i]->isDamageable())
			{
				actors[i]->decHealth(dmg);
				break;
			}
		}
}

bool StudentWorld::DamageableObjectAt(int x, int y) const
{
	for (int i = 0; i != actors.size(); i++)
	{
		if (Overlap(actors[i]->getX(), actors[i]->getY(), x, y))		//if overlapping with a damageable object, return true
			if (actors[i]->isAlive() && actors[i]->isDamageable())
				return true;
	}
	return false;
}

bool StudentWorld::isLevelOver() const
{
	for (int i = 0; i != actors.size(); i++)		//if no dangerous objects are left, level is over
		if (actors[i]->isDangerous())
			return false;
	return true;
}

StudentWorld::~StudentWorld()
{
	cleanUp();		//cleanup
}
