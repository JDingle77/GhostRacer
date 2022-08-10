#include "Actor.h"
#include "StudentWorld.h"
using namespace std;

//ACTOR CLASS:
bool Actor::overlap(double xA, double yA, double radiusA, double xB, double yB, double radiusB)
{
    double delta_x = xA - xB;
    
    if(delta_x < 0)
        delta_x *= -1;
    
    double delta_y = yA - yB;
    
    if(delta_y < 0)
        delta_y *= -1;
    
    double radius_sum = radiusA + radiusB;
    
    return delta_x < radius_sum*.25 && delta_y < radius_sum*.6;
}

bool Actor::getLiving() const
{
    return m_living;
}

void Actor::setDead()
{
    m_living = false;
}

double Actor::getSpeedv() const
{
    return m_speedv;
}

void Actor::setSpeedv(double speed)
{
    m_speedv = speed;
}

double Actor::getSpeedh() const
{
    return m_speedh;
}

void Actor::setSpeedh(double speed)
{
    m_speedh = speed;
}

bool Actor::getColl() const
{
    return m_coll;
}

int Actor::getHP() const
{
    return m_hp;
}

void Actor::getHP(int hp)
{
    m_hp += hp;
}

StudentWorld* Actor::getWorld() const
{
    return m_world;
}

bool Actor::beSprayedIfAppropriate()
{
    return false;
}

bool Actor::moveRelativeToGhostRacerVerticalSpeed(double dx)
{
    double vert_speed = getSpeedv() - getWorld()->getGr()->getSpeedv();
    double new_y = getY() + vert_speed;
    double new_x = getX() + dx;
    
    moveTo(new_x, new_y);
    
    // If Actor has gone off of the screen, kill it
    if(getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
    {
        setDead();
        return false;
    }
    
    return true;
}

bool Actor::isSprayable() const
{
    return m_spray;
}

// ZOMBIE CAB CLASS:
void ZombieCab::doSomething()
{
    if(!getLiving())
        return;
    
    GhostRacer* gr = getWorld()->getGr();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()) && !alreadyDamagedGhostRacer)
    {
        getWorld()->playSound(SOUND_VEHICLE_CRASH);
        gr->getHP(-20);
        
        // If this killed the Ghost Racer, return immediately
        if(gr->getHP() <= 0)
        {
            getWorld()->decLives();
            gr->setDead();
            getWorld()->playSound(SOUND_PLAYER_DIE);
            return;
        }
        
        double distance = gr->getX() - getX();
        
        
        if(distance >= 0)
        {
            // If the zombie cab is to the left of the Ghost Racer
            setSpeedh(-5);
            setDirection(120 + randInt(0, 19));
        }
        else if(distance < 0)
        {
            // If the zombie cab is to the right of the Ghost Racer
            setSpeedh(5);
            setDirection(60 - randInt(0, 19));
        }
        
        alreadyDamagedGhostRacer = true;
    }
    
    // If moving causes the zombie cab to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    // If the cab is moving up the screen
    if(getSpeedv() > gr->getSpeedv())
    {
        Actor* ptr = getWorld()->isThereCollInFront(getX(), getY());
        
        // If there is a "collision-avoidance worthy" actor in the zombie cab's lane that is in front of that zombie cab
        if(ptr != nullptr)
        {
            if(ptr->getY() - getY() < 96)
            {
                setSpeedv(getSpeedv() - .5);
                return;
            }
        }
    }
    
    // If the cab is moving down the screen or holding steady with Ghost Racer
    if(getSpeedv() <= gr->getSpeedv())
    {
        Actor* ptr = getWorld()->isThereCollBehind(getX(), getY());
        
        // If there is a "collision-avoidance worthy" actor in the zombie cab's lane that is behind that zombie cab
        if(ptr != nullptr)
        {
            if(getY() - ptr->getY() < 96)
            {
                setSpeedv(getSpeedv() + .5);
                
                return;
            }
        }
    }
    
    m_plan--;
    
    if(m_plan > 0)
        return;
    
    m_plan = randInt(4, 32);
    setSpeedv(getSpeedv() + randInt(-2, 2));
}

bool ZombieCab::beSprayedIfAppropriate()
{
    getHP(-1);
    
    // If the projectile killed the Zombie Cab
    if(getHP() <= 0)
    {
        setDead();
        getWorld()->playSound(SOUND_VEHICLE_DIE);
        
        // There is a 1 in 5 chance that the zombie cab will add a new oil slick at its current position
        if(randInt(1, 5) == 1)
            getWorld()->addActor(new Oil(getWorld(), getX(), getY()));

        getWorld()->increaseScore(200);
        return true;
    }
    else
    {
        getWorld()->playSound(SOUND_VEHICLE_HURT);
    }
    
    return true;
}

// GHOSTRACER CLASS:
void GhostRacer::doSomething()
{
    if(!getLiving())
        return;
    
    StudentWorld* sw = getWorld();
    
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    int ch;
    
    if(getX() <= LEFT_EDGE)
    {
        if(getDirection() > 90)
        {
            // If Ghost Racer is swerving off the road
            getHP(-10);
            
            if(getHP() <= 0)
            {
                getWorld()->decLives();
                setDead();
                sw->playSound(SOUND_PLAYER_DIE);
                return;
            }
        }
        
        setDirection(82);
        
        sw->playSound(SOUND_VEHICLE_CRASH);
    }
    else if(getX() >= RIGHT_EDGE)
    {
        if(getDirection() < 90)
        {
            // If Ghost Racer is swerving off the road
            getHP(-10);
            
            if(getHP() <= 0)
            {
                getWorld()->decLives();
                setDead();
                sw->playSound(SOUND_PLAYER_DIE);
                return;
            }
        }
        
        setDirection(98);
        
        sw->playSound(SOUND_VEHICLE_CRASH);
    }
    else if(getWorld()->getKey(ch))
    {
        // If the player pressed a key
        switch(ch)
        {
            case KEY_PRESS_SPACE:
                if(getNumSprays() >= 1)
                {
                    sw->addActor(new HolyWaterP(sw, getX() + SPRITE_HEIGHT * cos(getDirection() * 3.1415 / 180), getY() + SPRITE_HEIGHT * sin(getDirection() * 3.1415 / 180), getDirection()));
                    
                    sw->playSound(SOUND_PLAYER_SPRAY);
                    
                    m_water--;
                }
                break;
            case KEY_PRESS_LEFT:
                if(getDirection() < 114)
                    setDirection(getDirection() + 8);
                break;
            case KEY_PRESS_RIGHT:
                if(getDirection() > 66)
                    setDirection(getDirection() - 8);
                break;
            case KEY_PRESS_UP:
                if(getSpeedv() < 5)
                    setSpeedv(getSpeedv() + 1);
                break;
            case KEY_PRESS_DOWN:
                if(getSpeedv() > -1)
                    setSpeedv(getSpeedv() - 1);
                break;
        }
    }
    
    // Ghost Racer Movement Algorithm
    double max_shift_per_tick = 4.0;
    
    int direction = getDirection();
    
    double delta_x = cos(direction*(3.14/180)) * max_shift_per_tick;
    double cur_x = getX();
    double cur_y = getY();
    
    moveTo(cur_x + delta_x, cur_y);
    
    return;
}

void GhostRacer::spin()
{
    int newDirection;
    
    do
    {
        // Choose between adjusting clockwise or counterclockwise
        if(randInt(1, 2) == 1)
            newDirection = getDirection() + randInt(5, 20);
        else
            newDirection = getDirection() - randInt(5, 20);
        // Ghost Racer's angle must never go below 60 degrees or above 120 degrees
    } while (newDirection < 60 || newDirection > 120);
    
    setDirection(newDirection);
}

// PEDESTRIAN CLASS
void Pedestrian::moveAndPossiblyPickPlan()
{
    // If moving causes the Pedestrian to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    if(getPlan() > 0)
    {
        decPlan();
        return;
    }
    
    // Set the pedestrian’s horizontal speed to a random integer from -3 to 3, inclusive, NOT including zero
    do
    {
        setSpeedh(randInt(-3, 3));
    } while (getSpeedh() == 0);
    
    setPlan(randInt(4, 32));
    
    if(getSpeedh() < 0)
        setDirection(180);
    else
        setDirection(0);
}

// HUMAN CLASS:
void Human::doSomething()
{
    if(!getLiving())
        return;
    
    GhostRacer* gr = getWorld()->getGr();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        getWorld()->decLives();
        
        gr->setDead();
        
        return;
    }
    
    moveAndPossiblyPickPlan();
    
}

bool Human::beSprayedIfAppropriate()
{
    // Reverse its direction
    setSpeedh(getSpeedh() * -1);
    
    if(getDirection() == 0)
        setDirection(180);
    else
        setDirection(0);
    
    getWorld()->playSound(SOUND_PED_HURT);
    
    return true;
}

// ZOMBIE CLASS:
void Zombie::doSomething()
{
    if(!getLiving())
        return;
    
    GhostRacer* gr = getWorld()->getGr();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        gr->getHP(-5);
        
        if(gr->getHP() <= 0)
        {
            getWorld()->decLives();
            gr->setDead();
            
            getWorld()->playSound(SOUND_PLAYER_DIE);
            
            return;
        }
        
        setDead();
        
        return;
    }
    
    double distance = gr->getX() - getX();
    
//    If the zombie pedestrian’s X coordinate is within 30 pixels of Ghost Racer’s X coordinate, either left or right, AND the zombie pedestrian is in front of the Ghost Racer on the road
    if(distance <= 30 && distance >= -30 && getY() > gr->getY())
    {
        // Face down
        setDirection(270);
        
        
        if(distance > 0)
            // If zombie ped is to the left of Ghost Racer
            setSpeedh(1);
        else if(distance < 0)
            // If zombie ped is to the right of Ghost Racer
            setSpeedh(-1);
        else
            // Same X coordinate as Ghost Racer
            setSpeedh(0);
        
        m_grunt--;
        
        if(m_grunt <= 0)
        {
            getWorld()->playSound(SOUND_ZOMBIE_ATTACK);
            m_grunt = 20;
        }
    }
    
    moveAndPossiblyPickPlan();
}

bool Zombie::beSprayedIfAppropriate()
{
    StudentWorld* sw = getWorld();
    
    getHP(-1);
    
    // If the projectile killed the Zombie
    if(getHP() <= 0)
    {
        setDead();
        
        sw->playSound(SOUND_PED_DIE);
        
        if(randInt(1, 5) == 1)
            sw->addActor(new Heal(sw, getX(), getY()));
        
        sw->increaseScore(150);
    }
    else
    {
        sw->playSound(SOUND_PED_HURT);
    }
    
    return true;
}

// STATIONARY CLASS:
bool Stationary::beSprayedIfAppropriate()
{
    if(!isSprayable())
        return false;
    
    // If the stationary is sprayable, kill it
    setDead();
    
    return true;
}

// BORDER CLASS:
void Border::doSomething()
{
    // If moving causes the Border to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
}

// SOUL CLASS:
void Soul::doSomething()
{
    GhostRacer* gr = getWorld()->getGr();
    
    // If moving causes the Soul to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    // Do activity to Ghost Racer
    doActivity(gr);
    
    // Rotate itself by 10 degrees clockwise
    setDirection(getDirection() - 10);
}

void Soul::doActivity(GhostRacer *gr)
{
    StudentWorld* sw = getWorld();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        // Decrease the number of souls that need to be saved
        sw->decSouls2Save();
        
        setDead();
        
        sw->playSound(SOUND_GOT_SOUL);
        sw->increaseScore(100);
    }
}

// HOLY WATER G CLASS:
void HolyWaterG::doSomething()
{
    GhostRacer* gr = getWorld()->getGr();
    
    // If moving causes the Holy Water Goodie to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    // Do activity to Ghost Racer
    doActivity(gr);
}

void HolyWaterG::doActivity(GhostRacer *gr)
{
    StudentWorld* sw = getWorld();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        // Increase Ghost Racer's holy water charges by 10 charges
        gr->incWater();
        
        setDead();
        
        sw->playSound(SOUND_GOT_GOODIE);
        sw->increaseScore(50);
    }
}

// HEAL CLASS:
void Heal::doSomething()
{
    // If moving causes the Healing Goodie to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    GhostRacer* gr = getWorld()->getGr();
    
    // Do activity to Ghost Racer
    doActivity(gr);
}

void Heal::doActivity(GhostRacer *gr)
{
    StudentWorld* sw = getWorld();
    
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        // Heal Ghost Racer by 10 hit points but max health is 100
        if(gr->getHP() < 95)
            gr->getHP(10);
        else if(gr->getHP() < 100 && gr->getHP() >= 95)
            gr->getHP(5);
        
        setDead();
        
        sw->playSound(SOUND_GOT_GOODIE);
        sw->increaseScore(250);
    }
}

// OIL CLASS:
void Oil::doSomething()
{
    GhostRacer* gr = getWorld()->getGr();
    
    // If moving causes the Oil Slick to go off the screen, return immediately
    if(!moveRelativeToGhostRacerVerticalSpeed(getSpeedh()))
       return;
    
    // Do activity to Ghost Racer
    doActivity(gr);
}

void Oil::doActivity(GhostRacer *gr)
{
    if(overlap(getX(), getY(), getRadius(), gr->getX(), gr->getY(), gr->getRadius()))
    {
        // Tell Ghost Racer to spin itself
        getWorld()->playSound(SOUND_OIL_SLICK);
        gr->spin();
    }
}

// HOLY WATER P CLASS:
void HolyWaterP::doSomething()
{
    if(!getLiving())
        return;
    
    StudentWorld* w = getWorld();
    
    // If this Holy Water Projectile has hit something that can be affected
    if(w->sprayFirstAppropriateActor(this))
    {
        setDead();
        return;
    }
    
    moveForward(SPRITE_HEIGHT);
    m_maxdist -= SPRITE_HEIGHT;
    
    // If moving causes the Holy Water Projectile to go off the screen, return immediately
    if(getY() < 0 || getX() < 0 || getX() > VIEW_WIDTH || getY() > VIEW_HEIGHT)
    {
        setDead();
        return;
    }
    
    // If the holy water projectile has moved a total of 160 pixels, kill it
    if(m_maxdist == 0)
        setDead();
}

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
