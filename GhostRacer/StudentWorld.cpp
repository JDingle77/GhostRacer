#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    // Initialize member variables
    m_gr = NULL;
    lastBW = 0;
    m_bonus = 5000;
    m_souls2save = 7;
}

StudentWorld::~StudentWorld()
{
    // Clean up function will delete all Actors created with the new command
    cleanUp();
}

int StudentWorld::init()
{
    // Reset level
    m_souls2save = 2 * getLevel() + 5;
    m_bonus = 5000;
    lastBW = 0;
    m_gr = new GhostRacer(this);
    
    int N = VIEW_HEIGHT / SPRITE_HEIGHT;
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    
    // Add yellow border lines
    for(int j = 0; j < N; j++)
    {
        addActor(new Border(this, LEFT_EDGE, j * SPRITE_HEIGHT, true));
        addActor(new Border(this, RIGHT_EDGE, j * SPRITE_HEIGHT, true));
    }
    
    int M = VIEW_HEIGHT / (4*SPRITE_HEIGHT);
    
    // Add white border lines
    for(int j = 0; j < M; j++)
    {
        addActor(new Border(this, LEFT_EDGE + ROAD_WIDTH/3, j * (4*SPRITE_HEIGHT), false));
        addActor(new Border(this, RIGHT_EDGE - ROAD_WIDTH/3, j * (4*SPRITE_HEIGHT), false));
    }
    
    // new white border every 8 ticks
    lastBW = m_la.back()->getY();
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // If ghost racer is alive let it do something
    if(m_gr->getLiving())
        m_gr->doSomething();
    
    list<Actor*>::iterator itr;
    itr = m_la.begin();
    
    // Give each actor a chance to do something
    while(itr != m_la.end())
    {
        // If the actor is still active/alive
        if((*itr)->getLiving())
        {
            // Tell that actor to do smomething
            (*itr)->doSomething();
            
            // If Ghost Racer was destroyed during this tick
            if(!m_gr->getLiving())
                return GWSTATUS_PLAYER_DIED;
            
            // If Ghost Racer completed the current level
            if(m_souls2save == 0)
            {
                playSound(SOUND_FINISHED_LEVEL);
                increaseScore(m_bonus);
                return GWSTATUS_FINISHED_LEVEL;
            }
            
        }
        itr++;
    }
    
    itr = m_la.begin();
    
    // Remove newly-dead actors after each tick
    while(itr != m_la.end())
    {
        if(!(*itr)->getLiving())
        {
            delete *itr;
            itr = m_la.erase(itr);
        }
        else
        {
            itr++;
        }
    }
    
    lastBW += -4 - m_gr->getSpeedv();
    
    int new_border_y = VIEW_HEIGHT - SPRITE_HEIGHT;
    int delta_y = new_border_y - lastBW;
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    
    // Add new yellow borders
    if(delta_y >= SPRITE_HEIGHT)
    {
        addActor(new Border(this, LEFT_EDGE, new_border_y, true));
        addActor(new Border(this, RIGHT_EDGE, new_border_y, true));
    }
    
    // Add new white borders
    if(delta_y >= 4 * SPRITE_HEIGHT)
    {
        addActor(new Border(this, LEFT_EDGE + ROAD_WIDTH/3, new_border_y, false));
        addActor(new Border(this, RIGHT_EDGE - ROAD_WIDTH/3, new_border_y, false));
        lastBW = m_la.back()->getY();
    }
    
    // Potentially add new souls
    if(randInt(0, 99) == 0)
        addActor(new Soul(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
    
    // Potentially add new Holy Water Goodies
    int ChanceOfHolyWater = 100 + 10 * getLevel();
    if(randInt(0, ChanceOfHolyWater - 1) == 0)
        addActor(new HolyWaterG(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
    
    // Potentially add new Human Pedestrians
    int ChanceHumanPed = max(200 - getLevel() * 10, 30);
    if(randInt(0, ChanceHumanPed - 1) == 0)
        addActor(new Human(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
    
    // Potentially add new Zombie Pedestrians
    int ChanceZombiePed = max(100 - getLevel() * 10, 20);
    if(randInt(0, ChanceZombiePed - 1) == 0)
        addActor(new Zombie(this, randInt(0, VIEW_WIDTH), VIEW_HEIGHT));
    
    // Potentially add new Oil Slicks
    int ChanceOilSlick = max(150 - getLevel() * 10, 40);
    if(randInt(0, ChanceOilSlick - 1) == 0)
        addActor(new Oil(this, randInt(LEFT_EDGE, RIGHT_EDGE), VIEW_HEIGHT));
    
    // Potentially add new Zombie Cabs
    int ChanceVehicle = max(100 - getLevel() * 10, 20);
    if(randInt(0, ChanceVehicle - 1) == 0)
    {
        int cur_lane = randInt(1, 3);
        
        bool laneOneChecked = false;
        bool laneTwoChecked = false;
        bool laneThreeChecked = false;
        
        double startY = -99;
        double initialVertSpeed = -99;
        
        for(;;)
        {
            if(cur_lane == 1)
            {
                // Check if the left lane is too dangerous
                if(!isLaneTooDangerous(cur_lane, startY, initialVertSpeed))
                    break;

                laneOneChecked = true;

                if(laneOneChecked && laneTwoChecked && laneThreeChecked)
                    break;

                // The left lane is too dangerous to add a new zombie cab, so set cur_lane to the next of the three lanes to check
                if(laneTwoChecked)
                    cur_lane = 3;
                else
                    cur_lane = 2;
            }
            else if(cur_lane == 2)
            {
                // Check if the middle lane is too dangerous
                if(!isLaneTooDangerous(cur_lane, startY, initialVertSpeed))
                    break;

                laneTwoChecked = true;

                if(laneOneChecked && laneTwoChecked && laneThreeChecked)
                    break;

                // The middle lane is too dangerous to add a new zombie cab, so set cur_lane to the next of the three lanes to check
                if(laneThreeChecked)
                    cur_lane = 1;
                else
                    cur_lane = 3;
            }
            else
            {
                // Check if the right lane is too dangerous
                if(!isLaneTooDangerous(cur_lane, startY, initialVertSpeed))
                    break;

                laneThreeChecked = true;

                if(laneOneChecked && laneTwoChecked && laneThreeChecked)
                    break;

                // The right lane is too dangerous to add a new zombie cab, so set cur_lane to the next of the three lanes to check
                if(laneOneChecked)
                    cur_lane = 2;
                else
                    cur_lane = 1;
            }
        }
        
        // No lane was viable
        if(startY == -99 && initialVertSpeed == -99)
            return GWSTATUS_CONTINUE_GAME;
        
        // Add new Zombie Cab
        switch (cur_lane)
        {
            case 1:
                addActor(new ZombieCab(this, ROAD_CENTER - ROAD_WIDTH/3, startY, initialVertSpeed));
                break;
            case 2:
                addActor(new ZombieCab(this, ROAD_CENTER, startY, initialVertSpeed));
                break;
            case 3:
                addActor(new ZombieCab(this, ROAD_CENTER + ROAD_WIDTH/3, startY, initialVertSpeed));
                break;
            default:
                break;
        }
    }
    
    // Decrement bonus once per tick
    if(m_bonus > 0)
        m_bonus--;
    
    // Update the Game Status Line
    ostringstream oss;
    oss << "Score: " << getScore();
    oss << setw(7) << "Lvl: " << getLevel();
    oss << setw(14) << "Souls2Save: " << m_souls2save;
    oss << setw(9) << "Lives: " << getLives();
    oss << setw(10) << "Health: " << m_gr->getHP();
    oss << setw(10) << "Sprays: " << m_gr->getNumSprays();
    oss << setw(9) << "Bonus: " << m_bonus;
    setGameStatText(oss.str());
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete m_gr;
    list<Actor*>::iterator itr;
    
    itr = m_la.begin();
    
    // Iterate through the list of pointers to Actors and delete each one
    while(itr != m_la.end())
    {
        delete *itr;
        itr = m_la.erase(itr);
    }
}

GhostRacer* StudentWorld::getGr()
{
    return m_gr;
}

bool StudentWorld::sprayFirstAppropriateActor(Actor* a)
{
    list<Actor*>::iterator it = m_la.begin();
    
    // Iterate through the list of actors and spray them if they overlap w the Spray
    while(it != m_la.end())
    {
        if(overlaps(*it, a))
        {
            if(!(*it)->beSprayedIfAppropriate())
            {
                it++;
                continue;
            }
            else
            {
                return true;
            }
        }
        it++;
    }
    
    return false;
}

void StudentWorld::addActor(Actor* a)
{
    m_la.push_back(a);
}

Actor* StudentWorld::isThereCollInFront(double x, double y)
{
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    int cur_lane = 0;
    
    // Figure out what lane Zombie Cab is in
    if(x == ROAD_CENTER - ROAD_WIDTH/3)
        cur_lane = 1;
    else if(x == ROAD_CENTER)
        cur_lane = 2;
    else if(x == ROAD_CENTER + ROAD_WIDTH/3)
        cur_lane = 3;
    
    list<Actor*>::iterator it = m_la.begin();
    
    double minimum = 300;
    
    Actor* closestActor = nullptr;
    
    // Find closest collision avoidance worthy actor in front of the Zombie Cab
    if(cur_lane == 1)
    {
        // Left lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE && (*it)->getX() < LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getY() > y && (*it)->getY() < minimum && (*it)->getColl())
            {
                minimum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    else if(cur_lane == 2)
    {
        // Middle lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getY() > y && (*it)->getY() < minimum && (*it)->getColl())
            {
                minimum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    else if(cur_lane == 3)
    {
        // Right lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE && (*it)->getY() > y && (*it)->getY() < minimum && (*it)->getColl())
            {
                minimum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    return closestActor;
}

Actor* StudentWorld::isThereCollBehind(double x, double y)
{
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    int cur_lane = 0;
    
    // Figure out what lane Zombie Cab is in
    if(x == ROAD_CENTER - ROAD_WIDTH/3)
        cur_lane = 1;
    else if(x == ROAD_CENTER)
        cur_lane = 2;
    else if(x == ROAD_CENTER + ROAD_WIDTH/3)
        cur_lane = 3;
    
    list<Actor*>::iterator it = m_la.begin();
    
    double maximum = -1;
    
    Actor* closestActor = nullptr;
    
    // Find closest collision avoidance worthy actor in front of the Zombie Cab
    if(cur_lane == 1)
    {
        // Left lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE && (*it)->getX() < LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getY() < y && (*it)->getY() > maximum && (*it)->getColl())
            {
                maximum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    else if(cur_lane == 2)
    {
        // Middle lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getY() < y && (*it)->getY() > maximum && (*it)->getColl())
            {
                maximum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    else if(cur_lane == 3)
    {
        // Right lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE && (*it)->getY() < y && (*it)->getY() > maximum && (*it)->getColl())
            {
                maximum = (*it)->getY();
                closestActor = *it;
            }
            it++;
        }
        
        return closestActor;
    }
    return closestActor;
}

double StudentWorld::determineClosestToBottom(int cur_lane)
{
    list<Actor*>::iterator it = m_la.begin();
    
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    
    double minimum = 300;
    
    // Find closest collision avoidance worthy actor to the bottom
    if(cur_lane == 1)
    {
        // Left lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE && (*it)->getX() < LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getY() < minimum && (*it)->getColl())
                minimum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the bottom
        if(m_gr->getX() >= LEFT_EDGE && m_gr->getX() < LEFT_EDGE + ROAD_WIDTH/3 && m_gr->getY() < minimum)
            minimum = m_gr->getY();
        
        return minimum;
    }
    else if(cur_lane == 2)
    {
        // Middle lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getY() < minimum && (*it)->getColl())
                minimum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the bottom
        if(m_gr->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && m_gr->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && m_gr->getY() < minimum)
            minimum = m_gr->getY();
        
        return minimum;
    }
    else
    {
        // Right lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE && (*it)->getY() < minimum && (*it)->getColl())
                minimum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the bottom
        if(m_gr->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && m_gr->getX() < RIGHT_EDGE && m_gr->getY() < minimum)
            minimum = m_gr->getY();
        
        return minimum;
    }
}

double StudentWorld::determineClosestToTop(int cur_lane)
{
    list<Actor*>::iterator it = m_la.begin();
    
    int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    
    double maximum = -1;
    
    // Find closest collision avoidance worthy actor to the top
    if(cur_lane == 1)
    {
        // Left lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE && (*it)->getX() < LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getY() > maximum && (*it)->getColl())
                maximum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the top
        if(m_gr->getX() >= LEFT_EDGE && m_gr->getX() < LEFT_EDGE + ROAD_WIDTH/3 && m_gr->getY() > maximum)
            maximum = m_gr->getY();
        
        return maximum;
    }
    else if(cur_lane == 2)
    {
        // Middle lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && (*it)->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && (*it)->getY() > maximum && (*it)->getColl())
                maximum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the top
        if(m_gr->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && m_gr->getX() < RIGHT_EDGE - ROAD_WIDTH/3 && m_gr->getY() > maximum)
            maximum = m_gr->getY();
        
        return maximum;
    }
    else
    {
        // Right lane
        while(it != m_la.end())
        {
            if((*it)->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 &&  (*it)->getX() < RIGHT_EDGE && (*it)->getY() > maximum && (*it)->getColl())
                maximum = (*it)->getY();
            it++;
        }
        
        // Check if Ghost Racer is the closest to the top
        if(m_gr->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && m_gr->getX() < RIGHT_EDGE && m_gr->getY() > maximum)
            maximum = m_gr->getY();
        
        return maximum;
    }
}

bool StudentWorld::isLaneTooDangerous(int cur_lane, double& startY, double& initialVertSpeed)
{
    double minimum = determineClosestToBottom(cur_lane);
    double maximum = determineClosestToTop(cur_lane);
    
    // If there is enough room in the bottom, the lane is not too dangerous
    if(minimum == 300 || minimum > (VIEW_HEIGHT / 3))
    {
        startY = SPRITE_HEIGHT / 2;
        initialVertSpeed = m_gr->getSpeedv() + randInt(2, 4);
        return false;
    }
    
    // If there is enough room at the top, the lane is not too dangerous
    if(maximum == -1 || maximum < (VIEW_HEIGHT * 2/3))
    {
        startY = VIEW_HEIGHT - SPRITE_HEIGHT / 2;
        initialVertSpeed = m_gr->getSpeedv() - randInt(2, 4);
        return false;;
    }
    
    return true;
}

bool StudentWorld::overlaps(const Actor* a1, const Actor* a2) const
{
    double xA = a1->getX();
    double xB = a2->getX();
    double yA = a1->getY();
    double yB = a2->getY();
    double radiusA = a1->getRadius();
    double radiusB = a2->getRadius();
    double delta_x = xA - xB;
    
    if(delta_x < 0)
        delta_x *= -1;
    
    double delta_y = yA - yB;
    
    if(delta_y < 0)
        delta_y *= -1;
    
    double radius_sum = radiusA + radiusB;
    
    return delta_x < radius_sum*.25 && delta_y < radius_sum*.6;
}
