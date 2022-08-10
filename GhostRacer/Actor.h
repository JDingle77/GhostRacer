#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

class Actor: public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, double startX, double startY, int startDirection, double size, unsigned int depth, double speedv, bool coll, int hp, bool spray):GraphObject(imageID, startX, startY, startDirection, size, depth)
    {
        m_living = true;
        m_world = world;
        m_speedv = speedv;
        m_speedh = 0;
        m_coll = coll;
        m_hp = hp;
        m_spray = spray;
    }
    
    // Action to perform for each tick.
    virtual void doSomething() = 0;
    
    // Do the two actors overlap?
    bool overlap(double xA, double yA, double radiusA, double xB, double yB, double radiusB);
    
    // Is this actor dead?
    bool getLiving() const;
    
    // Mark this actor as dead.
    void setDead();
    
    // Get this actor's vertical speed.
    double getSpeedv() const;
    
    // Set this actor's vertical speed.
    void setSpeedv(double speed);
    
    // Get this actor's horizontal speed.
    double getSpeedh() const;
    
    // Set this actor's horizontal speed.
    void setSpeedh(double speed);
    
    // Does this object affect zombie cab placement and speed?
    bool getColl() const;
    
    // Get hit points.
    int getHP() const;

    // Increase hit points by hp.
    void getHP(int hp);
    
    // Get this actor's world
    StudentWorld* getWorld() const;
    
    // If this actor is affected by holy water projectiles, then inflict that
    // affect on it and return true; otherwise, return false.
    virtual bool beSprayedIfAppropriate();
    
    // Adjust the x coordinate by dx to move to a position with a y coordinate
    // determined by this actor's vertical speed relative to GhostRacser's
    // vertical speed.  Return true if the new position is within the view;
    // otherwise, return false, with the actor dead.
    bool moveRelativeToGhostRacerVerticalSpeed(double dx);
    
    // Return whether the object is affected by a holy water projectile.
    bool isSprayable() const;
private:
    bool m_living;
    double m_speedv;
    double m_speedh;
    bool m_coll;
    int m_hp;
    bool m_spray;
    StudentWorld* m_world;
    
};

class Car: public Actor
{
public:
    Car(StudentWorld* world, int imageID, double startX, double startY, double speedv, int hp):Actor(world, imageID, startX, startY, 90, 4.0, 0, speedv, true, hp, true)
    {}
};

class ZombieCab: public Car
{
public:
    ZombieCab(StudentWorld* world, double startX, double startY, double speedv):Car(world, IID_ZOMBIE_CAB, startX, startY, speedv, 3)
    {}
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
private:
    int m_plan = 0;
    bool alreadyDamagedGhostRacer = false;
};

class GhostRacer: public Car
{
public:
    GhostRacer(StudentWorld* world):Car(world, IID_GHOST_RACER, 128, 32, 0, 100)
    {}
    virtual void doSomething();
    
    // Increase water by 10 for grabbing a Holy Water Goodie
    void incWater()
    {
        m_water += 10;
    }
    
    // How many holy water projectiles does the object have?
    int getNumSprays() const
    {
        return m_water;
    }

    // Spin as a result of hitting an oil slick.
    void spin();
private:
    int m_water = 10;
};

class Pedestrian: public Actor
{
public:
    Pedestrian(StudentWorld* world, int imageID, double startX, double startY, double size):Actor(world, imageID, startX, startY, 0, size, 0, -4, true, 2, true)
    {}

    // Move the pedestrian.  If the pedestrian doesn't go off screen and
    // should pick a new movement plan, pick a new plan.
    void moveAndPossiblyPickPlan();
    
    // Decrement plan by one
    void decPlan()
    {
        m_plan--;
    }
    
    // Get the length of the plan
    int getPlan() const
    {
        return m_plan;
    }
    
    // Set the length of the plan to the given parameter
    void setPlan(int plan)
    {
        m_plan = plan;
    }
private:
    int m_plan = 0;
};

class Human: public Pedestrian
{
public:
    Human(StudentWorld* world, double startX, double startY):Pedestrian(world, IID_HUMAN_PED, startX, startY, 2.0)
    {}
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
};

class Zombie: public Pedestrian
{
public:
    Zombie(StudentWorld* world, double startX, double startY):Pedestrian(world, IID_ZOMBIE_PED, startX, startY, 3.0)
    {}
    virtual void doSomething();
    virtual bool beSprayedIfAppropriate();
private:
    int m_grunt = 0;
};

class Stationary: public Actor
{
public:
    Stationary(StudentWorld* world, int imageID, double startX, double startY, int direction, double size, bool spray):Actor(world, imageID, startX, startY, direction, size, 2, -4, false, 0, spray)
    {}
    
    virtual bool beSprayedIfAppropriate();

//       Do the object's special activity (increase health, spin Ghostracer, etc.)
    virtual void doActivity(GhostRacer* gr) = 0;
};

class Border: public Actor
{
public:
    Border(StudentWorld* world, double startX, double startY, bool yellow):Actor(world, isYellow(yellow), startX, startY, 0, 2.0, 2, -4, false, 0, false)
    {
        if(!yellow)
            m_yellow = false;
    }
    virtual void doSomething();
private:
    int isYellow(bool yellow)
    {
        if(yellow)
            return IID_YELLOW_BORDER_LINE;
        return IID_WHITE_BORDER_LINE;
    }
    bool m_yellow = true;
};

class Soul: public Stationary
{
public:
    Soul(StudentWorld* world, double startX, double startY):Stationary(world, IID_SOUL_GOODIE, startX, startY, 0, 4.0, false)
    {}
    virtual void doSomething();
    virtual void doActivity(GhostRacer* gr);
};

class HolyWaterG: public Stationary
{
public:
    HolyWaterG(StudentWorld* world, double startX, double startY):Stationary(world, IID_HOLY_WATER_GOODIE, startX, startY, 90, 2.0, true)
    {}
    virtual void doSomething();
    virtual void doActivity(GhostRacer* gr);
};

class Heal: public Stationary
{
public:
    Heal(StudentWorld* world, double startX, double startY):Stationary(world, IID_HEAL_GOODIE, startX, startY, 0, 1.0, true)
    {}
    virtual void doSomething();
    virtual void doActivity(GhostRacer* gr);
};

class Oil: public Stationary
{
public:
    Oil(StudentWorld* world, double startX, double startY):Stationary(world, IID_OIL_SLICK, startX, startY, 0, randInt(2, 5), false)
    {}
    virtual void doSomething();
    virtual void doActivity(GhostRacer* gr);
};

class HolyWaterP: public Actor
{
public:
    HolyWaterP(StudentWorld* world, double startX, double startY, int dir):Actor(world, IID_HOLY_WATER_PROJECTILE, startX, startY, dir, 1.0, 1, 0, false, 0, false)
    {}
    virtual void doSomething();
private:
    int m_maxdist = 160;
};

#endif // ACTOR_H_
