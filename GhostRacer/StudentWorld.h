#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <list>
using namespace std;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class Actor;

class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    // Return a pointer to the world's GhostRacer.
    GhostRacer* getGr();
    
    // Decrement the number of souls to save
    void decSouls2Save()
    {
        m_souls2save--;
    }
    
    // If actor a overlaps some live actor that is affected by a holy water
    // projectile, inflict a holy water spray on that actor and return true;
    // otherwise, return false.  (See Actor::beSprayedIfAppropriate.)
    bool sprayFirstAppropriateActor(Actor* a);
    
    // Add an actor to the world.
    void addActor(Actor* a);
    
    // Tell if there is a "collision-avoidance worthy" actor in the zombie cab's lane that is in front of that zombie cab
    Actor* isThereCollInFront(double x, double y);
    
    // Tell if there is a "collision-avoidance worthy" actor in the zombie cab's lane that is behind of that zombie cab
    Actor* isThereCollBehind(double x, double y);
    
private:
    list<Actor*> m_la;
    GhostRacer* m_gr;
    double lastBW;
    int m_souls2save;
    int m_bonus;
    
    // Determine the closest “collision avoidance-worthy actor” to the BOTTOM of the screen in candidate lane
    double determineClosestToBottom(int cur_lane);
    
    // Determine the closest “collision avoidance-worthy actor” to the TOP of the screen in candidate lane
    double determineClosestToTop(int cur_lane);
    
    // Determine if the current lane is too dangerous
    bool isLaneTooDangerous(int cur_lane, double& startY, double& initialVertSpeed);
    
    // Return true if actor a1 overlaps actor a2, otherwise false.
    bool overlaps(const Actor* a1, const Actor* a2) const;
};

#endif // STUDENTWORLD_H_
