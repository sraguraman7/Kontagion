#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>
#define _USE_MATH_DEFINES

using namespace std;

void newxy(int x, int y, int& newx, int& newy)	//find a newx and a newy for duplicating bacteria
{
	newx = x;
	if (newx < VIEW_WIDTH / 2)
		newx += SPRITE_WIDTH / 2;
	else if (newx > VIEW_WIDTH / 2)
		newx -= SPRITE_WIDTH / 2;

	newy = y;
	if (newy < VIEW_HEIGHT / 2)
		newx += SPRITE_WIDTH / 2;
	else if (newx > VIEW_HEIGHT / 2)
		newx -= SPRITE_WIDTH / 2;
}

Actor::Actor(int image, int startx, int starty, int startd, int depth, StudentWorld* sw, bool canBlock, bool isDamageable, int hp, bool isEdible, bool dangerous) :GraphObject(image, startx, starty, startd, depth) 
{
	m_canBlock = canBlock;
	m_alive = true;
	m_arena = sw;
	m_isdamageable = isDamageable;
	m_hp = hp;
	m_dangerous = dangerous;
	m_edible = isEdible;
}

StudentWorld* Actor::arena() const	//arena pointer
{
	return m_arena;
}

Actor::~Actor()
{}

bool Actor::isDangerous() const	//is it dangerous
{
	if (!isAlive())
		return false;
	return m_dangerous;
}

bool Actor::canItBlock() const	//can it block movement
{
	if (!isAlive())
		return false;
	return m_canBlock;
}

void Actor::decHealth(int dmg)	//lower health of an actor
{
	m_hp -= dmg;
	if (m_hp <= 0)		//if less than 0, set to 0 and set the object to dead
	{
		m_hp = 0;
		setStatus(false);
	}
}

int Actor::health() const		//return actor hp
{
	return m_hp;
}

bool Actor::isEdible() const		//is the actor edible
{
	if (!isAlive())
		return false;
	return m_edible;
}

void Actor::incHealth(int x)		//increase actor's health. Set a hard cap at 100 (primarily (currently, only) used for Socrates)
{
	m_hp += x;
	if (m_hp >= 100)
		m_hp = 100;
}
bool Actor::isAlive() const		//is it alive
{
	return m_alive;
}

bool Actor::isDamageable() const		//is the actor damageable by sprays
{
	return m_isdamageable;
}

void Actor::setStatus(bool life)		//set the life status of an actor
{
	m_alive = life;
}

Bacterium::Bacterium(int image, int x, int y, int hp, StudentWorld* sw) : Actor(image, x, y, 90, 0, sw, false, true, hp, false, true)
{
	m_movePlan = 0;
	m_foodEaten = 0;
}

int Bacterium::foodEaten() const		//how much food has this bacterium eaten
{
	return m_foodEaten;
}

void Bacterium::resetFood()	//reset food counter
{
	m_foodEaten = 0;
}

int Bacterium::movePlan() const		//return movePlan
{
	return m_movePlan;
}

void Bacterium::incMovePlan(int x)	//increase the moveplan by x
{
	m_movePlan += x;
}

bool Bacterium::canMoveForward(int dist, int direction, Bacterium* b) const		//can it move forward?
{
	double x = b->getX(), y = b->getY();
	b->getPositionInThisDirection(direction, dist, x, y);
	if (arena()->isBlockAt(x, y))			//if there is a blocking object in front, it cannot
		return false;
	if (Distance(VIEW_WIDTH / 2, VIEW_HEIGHT / 2, x, y) >= VIEW_RADIUS)		//if it is about to hit the radius, it cannot
		return false;
	return true;	//otherwise, it can
}

void Bacterium::resetMovePlan(int x)
{
	m_movePlan = x;		//reset to x
}

void Bacterium::incFood()		//bacterium ate a food object
{
	m_foodEaten += 1;
}

Bacterium::~Bacterium()
{}

Ecoli::Ecoli(int x, int y, StudentWorld* sw) : Bacterium(IID_ECOLI, x, y, 5, sw)
{}

void Ecoli::doSomething()
{
	if (!isAlive())		//if dead
		return;

	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))		//if overlapping with Socrates
		arena()->playerDecHealth(4);
	else if (foodEaten() >= 3)		//if ready to divide
	{
		int newx = 0, newy = 0;
		newxy(getX(), getY(), newx, newy);
		arena()->createBacterium(newx, newy, 5);
		resetFood();
	}
	else if (arena()->eatFood(getX(), getY()))		//if about to eat
		incFood();

	if (Distance(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()) <= 256)
	{
		setDirection(findAngle(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()));		//if close enough to Socrates to target him
		for (int i = 0; i != 10; i++)
		{
			if (canMoveForward(2, getDirection(), this))		//if it can move forward, move
			{
				moveAngle(getDirection(), 2);
				return;
			}
			else setDirection((getDirection() + 10) % 360);		//otherwise, increment direction by 10 degrees
		}
	}
}

void Ecoli::decHealth(int dmg)
{
	Actor::decHealth(dmg);
	if (isAlive())
		arena()->playSound(SOUND_ECOLI_HURT);		//if ecoli is hurt, play sound
	else
	{
		arena()->playSound(SOUND_ECOLI_DIE);		//if ecoli dies, play sound
		arena()->increaseScore(100);
		int spawn = randInt(0, 1);
		if (spawn == 1)
			arena()->createFood(getX(), getY());	//50% chance to spawn new food
	}
}

Ecoli::~Ecoli()
{}

Salmonella::Salmonella(int x, int y, int hp, StudentWorld* sw) : Bacterium(IID_SALMONELLA, x, y, hp, sw)
{}

void Salmonella::decHealth(int dmg)
{
	Actor::decHealth(dmg);		//if salmonella takes dmg, play sound
	if (isAlive())
		arena()->playSound(SOUND_SALMONELLA_HURT);
	else
	{
		arena()->playSound(SOUND_SALMONELLA_DIE);		//if salmonella dies, play sound
		arena()->increaseScore(100);
		int spawn = randInt(0, 1);
		if (spawn == 1)
			arena()->createFood(getX(), getY());		//chance to spawn food
	}
}

void Salmonella::finishMove()		//common code between all Salmonella
{
	if (movePlan() > 0)
	{
		incMovePlan(-1);
		if (canMoveForward(3, getDirection(), this))	//if it can move forward, do so
			moveAngle(getDirection(), 3);
		else
		{
			int x = randInt(0, 359);		//otherwise, random direction and reset moveplan
			setDirection(x);
			resetMovePlan(10);
		}
		return;
	}
	else				//if no current moveplan
	{
		if (arena()->isFoodNearby(getX(), getY()))		//if thre is food nearby
		{
			setDirection(arena()->findFoodNearby(getX(), getY()));		//find it and move towards it, if possible
			if (canMoveForward(3, getDirection(), this))
				moveAngle(getDirection(), 3);
			else
			{
				setDirection(randInt(0, 359));		//otherwise, random direction
				resetMovePlan(10);
				return;
			}
		}
		else
		{
			setDirection(randInt(0, 359));		//set to random direction
			resetMovePlan(10);
			return;
		}
	}
}

Salmonella::~Salmonella()
{}

RegularSalmonella::RegularSalmonella(int x, int y, StudentWorld* sw) : Salmonella(x, y, 4, sw)
{}

void RegularSalmonella::doSomething()
{
	if (!isAlive())
		return;

	if (Overlap(arena()->SocratesX(), arena()->SocratesY(), getX(), getY()))	//if overlapping with Socrates, dmg him
		arena()->playerDecHealth(1);
	else if (foodEaten() >= 3)		//if it can divide
		{
			int newx = 0, newy = 0;
			newxy(getX(), getY(), newx, newy);
			arena()->createBacterium(newx, newy, 4);		//create new regular salmonella and reset food counter
			resetFood();
		}
	else if (arena()->eatFood(getX(), getY()))		// eat food and increment food counter
			incFood();

	finishMove();
}

RegularSalmonella::~RegularSalmonella()
{}

AggressiveSalmonella::AggressiveSalmonella(int x, int y, StudentWorld* sw) : Salmonella(x, y, 10, sw)
{}

void AggressiveSalmonella::doSomething()
{
	if (!isAlive())
		return;

	bool step6 = true;		//as of right now, we should do step6 onward
	if (Distance(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()) <= 72)
	{
		setDirection(findAngle(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()));	//if we can move to Socrates, attempt to do so and don't do step 6 onward
		if (canMoveForward(3, getDirection(), this))
			moveAngle(getDirection(), 3);
		step6 = false;
	}

	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))
	{
		arena()->playerDecHealth(2);		//if overlapping with Socrates, dmg him
		if (!step6)		//if we're done here, we can return
			return;
	}
	else if (foodEaten() >= 3)		//else if we are ready to divide
	{
		int newx = 0, newy = 0;
		newxy(getX(), getY(), newx, newy);		//find newx, newy for new bacterium

		arena()->createBacterium(newx, newy, 10);		//create new bacteria and reset food
		resetFood();
		if (!step6)
			return;
	}
	else if (arena()->eatFood(getX(), getY()))		//if we can eat, do so and increment food
		incFood();
	if (!step6)
		return;
	finishMove();		//finish move with common behavior for all salmonella
}

AggressiveSalmonella::~AggressiveSalmonella()
{}

Socrates::Socrates(StudentWorld* sw) :Actor(IID_PLAYER, 0, VIEW_HEIGHT/2, 0, 100, sw, false, false, 100, false, false)
{
	m_spray = 20;
	m_flame = 5;
}

void Socrates::incFlame(int x)		//add flame charges
{
	m_flame += x;
}

void Socrates::decHealth(int dmg)
{
	Actor::decHealth(dmg);		//deal dmg
	if (isAlive())
		arena()->playSound(SOUND_PLAYER_HURT);		//play appropriate sounds depending on whether Socrates has been hurt or has died
	else
		arena()->playSound(SOUND_PLAYER_DIE);
}

void Socrates::doSomething()
{
	if (!isAlive())
		return;
	int ch;
	if (arena()->getKey(ch))	//switch on the input
		switch (ch)
		{
		case KEY_PRESS_RIGHT:		//if right press, minus 5 degrees direction and move appropriately
			setDirection(getDirection() - 5);
			moveTo(128 - 128 * cos(getDirection() * (3.1415926535)/ 180), 128 - 128 * sin(getDirection() * (3.1415926535)/180));
			break;
		case KEY_PRESS_LEFT:
			setDirection(getDirection() + 5);		//if left, plus 5 degrees and move as you should
			moveTo(128 - 128 * cos(getDirection() * (3.1415926535) / 180), 128 - 128 * sin(getDirection() * (3.1415926535) / 180));
			break;
		case KEY_PRESS_SPACE:		//spray; if we have charges
			if (m_spray != 0)
			{
				double x = 0, y = 0;
				getPositionInThisDirection(getDirection(), SPRITE_WIDTH, x, y);	//get position in front of Socrates and add spray here
				arena()->addSpray(x, y, getDirection());
				m_spray -= 1;
				arena()->playSound(SOUND_PLAYER_SPRAY);		//decrement spray and play sound
			}
			break;
		case KEY_PRESS_ENTER:
			if (m_flame != 0)		//if enter is pressed, and we have flame charge
			{
				for (int i = 0; i <= 360; i += 22)		//repeat 16 times
				{
					double x = 0, y = 0;
					getPositionInThisDirection(getDirection() + i, SPRITE_WIDTH, x, y);	//shoot flames outward around Soc, 16 times
					arena()->addFlame(x, y, getDirection() + i);
				}

				m_flame -= 1;
				arena()->playSound(SOUND_PLAYER_FIRE);		//decrement and play sound
			}
			break;
		}
	else
		if(m_spray < 20)		//if no move input, add a spray charge
			m_spray += 1;
}

int Socrates::getFlame() const		//return flame charges
{
	return m_flame;
}

int Socrates::getSpray() const		//return spray charges
{
	return m_spray;
}

Socrates::~Socrates()
{}

Dirt::Dirt(int x, int y, StudentWorld* sw) :Actor(IID_DIRT, x, y, 0, 1, sw, true, true, 1, false, false)
{}

void Dirt::doSomething()
{}

Dirt::~Dirt()
{}

Food::Food(int x, int y, StudentWorld* sw) :Actor(IID_FOOD, x, y, 90, 1, sw, false, false, 1, true, false)
{}

void Food::doSomething()
{}

Food::~Food()
{}

Pit::Pit(int x, int y, StudentWorld* sw) : Actor(IID_PIT, x, y, 0, 1, sw, false, false, 1, false, true)	//appropriate number of bacteria
{
	m_salmonella = 5;
	m_aggro = 3;
	m_ecoli = 2;
}

int Pit::bacteriaLeft() const
{
	return (m_salmonella + m_aggro + m_ecoli);		//return total bacteria
}

void Pit::doSomething()
{
	if (bacteriaLeft() == 0)		//if no more bacteria, kill it 
		setStatus(false);
	else
	{
		int arr[3] = { 0,0,0 };		//array of statuses
		if (m_salmonella != 0)		//if we have corresponding bacteria, set their statuses to true
			arr[0] = 1;
		if (m_aggro != 0)
			arr[1] = 1;
		if (m_ecoli != 0)
			arr[2] = 1;

		int spawn = randInt(1, 50);		//2% chance of spawning
		if (spawn == 50)
		{
			int x = getX();
			int y = getY();
			for (;;)		//manually break once we make a random choice between bacteria we can choose from
			{
				int choice = randInt(1, 3);		//choose 1-3
				bool leave = false;
				switch (choice)
				{
				case 1:
					if(arr[0])		//if we have regular salmonella, create one
					{
						arena()->createBacterium(x, y, 4);
						m_salmonella--;
						leave = true;
					}
					break;
				case 2:
					if (arr[1])		//if we have aggressive salmonella, create one
					{
						arena()->createBacterium(x, y, 10);
						m_aggro--;
						leave = true;
					}
					break;
				case 3:
					if (arr[2])
					{
						arena()->createBacterium(x, y, 5);		//if we have ecoli, create one
						m_ecoli--;
						leave = true;
					}
					break;
				default:
					break;
				}
				if (leave)
					break;
			}
			arena()->playSound(SOUND_BACTERIUM_BORN);		//play same sound for all: bacteria born
		}
	}
}

Pit::~Pit()
{}

Goodie::Goodie(int image, int startx, int starty, StudentWorld* sw) : Actor(image, startx, starty, 0, 1, sw, false, true, 1, false, false)
{
	int x = randInt(0, 300 - 10 * (sw->getLevel()) - 1);	//random lifetime
	if (50 > x)
		x = 50;
	m_time = x;
}

void Goodie::updateTime()		//update time
{
	m_time--;
}

int Goodie::timeLeft() const		//return lifetime
{
	return m_time;
}

Goodie::~Goodie()
{}

ExtraLifeGoodie::ExtraLifeGoodie(int x, int y, StudentWorld* sw) : Goodie(IID_EXTRA_LIFE_GOODIE, x, y, sw)
{}

void ExtraLifeGoodie::doSomething()
{
	if (!isAlive())
		return;
	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))	//if it overlaps
	{
		arena()->increaseScore(500);		//increase score, play sound, kill the goodie, and increase Soc lives
		setStatus(false);
		arena()->playSound(SOUND_GOT_GOODIE);
		arena()->incLives();
		return;
	}
	updateTime();
	if (timeLeft() == 0)		//if lifetime is up, kill it
		setStatus(false);
}

ExtraLifeGoodie::~ExtraLifeGoodie()
{}

HealthGoodie::HealthGoodie(int startx, int starty, StudentWorld* sw) : Goodie(IID_RESTORE_HEALTH_GOODIE, startx, starty, sw)
{}

void HealthGoodie::doSomething()
{
	if (!isAlive())
		return;
	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))
	{
		arena()->increaseScore(250);		//inc score, kill goodie, play sound, inc Soc hp
		setStatus(false);
		arena()->playSound(SOUND_GOT_GOODIE);
		arena()->incHealth(100);
		return;
	}
	updateTime();
	if (timeLeft() == 0)		//if lifetime is up, kill it
		setStatus(false);
}

HealthGoodie::~HealthGoodie()
{}

FlameGoodie::FlameGoodie(int startx, int starty, StudentWorld* sw) : Goodie(IID_FLAME_THROWER_GOODIE, startx, starty, sw)
{}

void FlameGoodie::doSomething()
{
	if (!isAlive())
		return;
	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))
	{
		arena()->increaseScore(300);		//increase score, kill goodie, play sound, inc flame
		setStatus(false);
		arena()->playSound(SOUND_GOT_GOODIE);
		arena()->incFlame(5);
		return;
	}
	updateTime();
	if (timeLeft() == 0)		//if lifetime is up, kill goodie
		setStatus(false);
}

FlameGoodie::~FlameGoodie()
{}

Fungus::Fungus(int x, int y, StudentWorld* sw) : Goodie(IID_FUNGUS, x, y, sw)
{}

void Fungus::doSomething()
{
	if (!isAlive())
		return;
	if (Overlap(getX(), getY(), arena()->SocratesX(), arena()->SocratesY()))		//if Socrates overlaps with funugs, hurt Soc and kill fungus
	{
		arena()->increaseScore(-50);		//dec score
		setStatus(false);
		arena()->playerDecHealth(20);
		return;
	}
	updateTime();
	if (timeLeft() == 0)		//if lifetime is up, kill fungus
		setStatus(false);
}

Fungus::~Fungus()
{}

Projectile::Projectile(int image, int x, int y, int direction, int distance, int damage, StudentWorld* sw) : Actor(image, x, y, direction, 1, sw, false, false, 1, false, false)
{
	m_distance = distance;
	m_dmg = damage;
}

int Projectile::dmg() const // return dmg
{
	return m_dmg;
}

void Projectile::incDistance()		//decrease distance (implying it has traveled 8 pixels forward)
{
	m_distance -= SPRITE_WIDTH;
}

int Projectile::Distance() const		//return distance left
{
	return m_distance;
}

void Projectile::doSomething()		//all projectiles behave the same
{
	if (!isAlive())
		return;

	if (arena()->DamageableObjectAt(getX(), getY()))		//dmg object 
	{
		arena()->hurtObjectAt(getX(), getY(), dmg());		//kill object 
		setStatus(false);
		return;
	}
	else
	{
		moveAngle(getDirection(), SPRITE_WIDTH);		//else continue moving in the direction
		incDistance();
	}
	if (Distance() <= 0)		//if no more distance to travel, kill it
		setStatus(false);
}

Projectile::~Projectile()
{}

Flame::Flame(int x, int y, int direction, StudentWorld* sw) : Projectile(IID_FLAME, x, y, direction, 32, 5, sw)
{}

Flame::~Flame()
{}

Spray::Spray(int x, int y, int direction, StudentWorld* sw) : Projectile(IID_SPRAY, x, y, direction, 112, 2, sw)
{}

Spray::~Spray()
{}