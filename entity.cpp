#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"

#include <string>
#include <queue>
#include <set>
#include <cmath>

#include "maping.h"
#include "factions.h"
#include "entity.h"


// Static variables and methods in Unit
std::unique_ptr<Map> Entity::mapData;
std::shared_ptr<std::list<std::shared_ptr<Entity>>> Entity::bootCamp;



void Entity::setMap(int x, int y){
    mapData = std::make_unique<Map>(170,85);
    mapData->generateVectorMap();
}

Entity::Entity(ranks assignment, olc::vi2d location,std::shared_ptr<Faction> faction) 
:_rank(assignment), _location(location.clamp({0,0},mapData->br - olc::vi2d(1,1))),_faction(faction) {
    _hp = faction->checkStat(_rank,initalHP);
}

Entity::~Entity(){
    
}

//static factorys
std::shared_ptr<Entity> Entity::makeUnit(ranks rank, olc::vi2d location, std::shared_ptr<Entity> home,std::shared_ptr<Faction> faction){
    std::shared_ptr<Entity> newUnit(new Entity(rank,location,faction) );
    newUnit->_homeBase = home;
    newUnit->addToMap();
    newUnit->set(mobile).set(attacker);
    return newUnit;
}

std::shared_ptr<Entity> Entity::makeBase(ranks baseClass,ranks unitClass, olc::vi2d location,std::shared_ptr<Faction> faction){

    std::shared_ptr<Entity> newBase(new Entity(baseClass,location,faction) );
    newBase->_buildable = unitClass;
    newBase->addToMap();
    mapData->factionColours[faction->id()] = faction->colour();
    mapData->setBaseLocation(newBase->location(),newBase->faction());
    newBase->set(builder);
    newBase->_homeBase.reset();
    return newBase;
}

        
//Getters and Setters
void Entity::addToMap(){
    if(std::shared_ptr<Faction> fact = _faction.lock())
        _mapIter = mapData->addUnit(_location,shared_from_this());
}

olc::vi2d Entity::location(){
    return _location;
}

int Entity::faction(){
    if(std::shared_ptr<Faction> faction = _faction.lock())
        return faction->id();
    return -1;
}

olc::Pixel Entity::factionColour(){
    if(std::shared_ptr<Faction> fact = _faction.lock())
        return fact->colour();
    return olc::WHITE;
}

float Entity::getHP(){
    return _hp;
}

int Entity::checkStat(statIndex stat){
    if(std::shared_ptr<Faction> fact = _faction.lock())
        return fact->checkStat(_rank,stat);
    return -1;
}
void Entity::dealDamage(int dam){
    _hp -= dam;
    if(_hp <= 0.0f){
        onKill();
    }
}
void Entity::onKill(){
    if(!_alive)
        return; // nothing to do
    _alive = false;
    
    if(flag(builder)){
        // if this Entity builds others they will need to be destroyed when this dies
        // on kill function expected to remove unit from list
        while(_armyRegistry.begin() != _armyRegistry.end())
            if(std::shared_ptr<Entity> ptr = _armyRegistry.begin()->lock()){
                ptr->onKill();
            } else { // deadlock prevention, incase unit was previously invaldiated but not correctly removed
                _armyRegistry.pop_front();
            }
    }
    
    if(std::shared_ptr<Entity> homeBase = _homeBase.lock())
        homeBase->honourDead( _homeIter);
    
    mapData->removeUnit(_location,_mapIter);
}

int Entity::getSize(){
    if(std::shared_ptr<Faction> fact = _faction.lock())
        return fact->checkStat(_rank,size);
    return 0;
}

std::string Entity::getName(){
    if(std::shared_ptr<Faction> fact = _faction.lock())
        return fact->getName(_rank);
    return "";
    
}

//Methods

int Entity::update(){
    std::shared_ptr<Faction> faction = _faction.lock();
    if(!faction)
        return -1; // error faction not avaliable?
    
    if(flag(builder)){
        // if room build new unit
        if(_armyRegistry.size() < faction->checkStat(_rank,actionEffect) ){
            std::shared_ptr<Entity> soldier = makeUnit(_buildable,_location,shared_from_this(),faction);
            bootCamp->emplace_back(soldier);
            _armyRegistry.emplace_back(soldier);
            soldier->_homeIter = std::prev(_armyRegistry.end());
            return faction->checkStat(_rank,actionSpeed);
        }
    }
    
    // check for surrounding enemies and attack if found
    if(flag(mobile)){
        olc::vi2d targetStep = _location;
        float targetDist = 99999.9f; 
        for(olc::vi2d offset : compass){
            olc::vi2d checkStep = _location + offset;
            if(!mapData->checkBounds(checkStep))
                    continue; // if the location is outside map skip it
            
            if(mapData->checkFactionEnemy(faction->id(),checkStep) && flag(attacker)) // found enemy, attack it
                return attack(checkStep);
            
            // continue to check vector pathing for optimal paths
            for(auto [checkFact,checkDist] : mapData->vectorPathField[checkStep.x][checkStep.y]){
                if(checkFact != faction->id() && checkDist < targetDist && mapData->checkForRoom(checkStep,faction->id()) > faction->checkStat(_rank,size) ){
                    targetStep = checkStep;
                    targetDist = checkDist;
                }
            }
        }
        return move(targetStep);
    }
    
    return 1;// no action, wait for next turn
}

int Entity::attack(olc::vi2d location){
    std::shared_ptr<Faction> faction = _faction.lock();
    if(!faction)
        return -1; // error faction deleted
    
    std::shared_ptr<Entity> target = mapData->getUnit(location);
    if(target != nullptr && target->faction() != faction->id()) // check target exists and is not friendly
        target->dealDamage(faction->checkStat(_rank,actionEffect));
    return faction->checkStat(_rank,actionSpeed);

}

int Entity::move(olc::vi2d step){
    std::shared_ptr<Faction> faction = _faction.lock();
    if(!faction)
        return -1; // error faction deleted

    if(_location == step)
        return 1; // not moving, wait a turn
    
    // set target destination and clamp to map area
    float movetime = faction->checkStat(_rank,moveSpeed) * (1 + mapData->terrainSlope(step,_location) );
    mapData->moveUnit(_location,step,_mapIter);
    _location = step;
    return int(movetime);
}

void Entity::honourDead(std::list<std::weak_ptr<Entity>>::iterator _iter){
    _armyRegistry.erase(_iter);
}

Entity& Entity::set(FLAGS flag){
    _flags.emplace(flag);
    return *this;
}

Entity& Entity::unset(FLAGS flag){
    if(_flags.find(flag) != _flags.end())
        _flags.erase(flag);
    return *this;
}

bool Entity::flag(FLAGS flag){
    if(_flags.find(flag) != _flags.end())
        return true;
    return false;
}

bool Entity::alive(){
    return _alive;
}
