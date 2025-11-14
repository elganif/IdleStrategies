#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"

#include <string>
#include <queue>
#include <set>
#include <cmath>

#include "engine.h"
#include "maping.h"
#include "factions.h"
#include "entity.h"


// Static variables and methods in Unit

Entity::Entity(std::variant<UnitRank,StructureRank> rank,std::shared_ptr<Faction> faction) 
: _myRank(rank), _faction(faction) {
    
}

Entity::~Entity(){
}

int Entity::faction(){
    if(std::shared_ptr<Faction> faction = _faction.lock() )
        return faction->id();
    return -1;
}

olc::Pixel Entity::factionColour(){
    if(std::shared_ptr<Faction> faction = _faction.lock() )
        return faction->colour();
    return olc::WHITE;
}

bool Entity::alive(){
    return _alive;
}

float Entity::getHP(){
    return _hp;
}

int Unit::checkStat(std::variant<UnitStats,StructureStats> stat){
    if(std::shared_ptr<Faction> faction = _faction.lock() ) {
        if(std::holds_alternative<StructureStats>(stat) ){
            return -10;
        }
        return faction->checkStat(std::get<UnitRank>(_myRank),std::get<UnitStats>(stat) );
    }
    return -1; // no faction
}

int Structure::checkStat(std::variant<UnitStats,StructureStats> stat){
    if(std::shared_ptr<Faction> faction = _faction.lock() ) {
        if (std::holds_alternative<UnitStats>(stat) ){
            return -10; // might consider a throw 
        }
        int improvement = 0;
        switch(std::get<StructureStats>(stat) ) {
        case maxHP:
            improvement = upgrades.maxHP;
            break;
        case buildSpeed:
            improvement = upgrades.actionSpeed;
            break;
        case buildCap:
            improvement = upgrades.actionEffect;
            break;
        }
        return faction->checkStat(std::get<StructureRank>(_myRank),std::get<StructureStats>(stat) ) + improvement;
    }
    return -1; // no faction
}

std::string Entity::getName(){
    if(std::shared_ptr<Faction> faction = _faction.lock() ){
        return std::visit([faction](auto arg){return faction->getName(arg);},_myRank );
    }
    return "";
}

Entity& Entity::set(FLAGS flag){
    _flags.emplace(flag);
    return *this;
}

Entity& Entity::unset(FLAGS flag){
    if(_flags.find(flag) != _flags.end() )
        _flags.erase(flag);
    return *this;
}

bool Entity::flag(FLAGS flag){
    if(_flags.find(flag) != _flags.end() )
        return true;
    return false;
}

void Entity::dealDamage(int dam){
    _hp -= dam;
    if(_hp <= 0.0)
        onDeath();
}

bool Entity::can_queue() {
    return alive();
}

Unit::Unit(UnitRank rank, std::shared_ptr<Structure> home, std::shared_ptr<Faction> faction) 
: Entity(rank,faction), _homeBase(home) {
    _hp = this->checkStat(initialHP);
}

Unit::~Unit(){
    onDeath();
}

olc::vi2d Unit::location(){
    return in_map::_location;
}

void Unit::onDeath(){
    _alive = false;
    clearFromMap();
    if(std::shared_ptr<Structure> homeBase = _homeBase.lock() ){
        homeBase->honourDead( _homeBaseIter);
        _homeBase.reset();
    }
}

void Unit::addToMap(olc::vi2d location){
    _location = location;
    _mapIter = Map::mapData->addUnit(_location,checkStat(size),std::static_pointer_cast<Unit>(mySelf.lock() ) );
    
}

int Unit::update(){
    std::shared_ptr<Faction> faction = _faction.lock();
    if(!faction)
        return -1; // error faction not avaliable?
        
    // check for surrounding enemies and attack if found
    olc::vi2d targetStep = _location;
    float targetDist = 99999.9f; 
    for(olc::vi2d offset : Map::mapData->compass){
        olc::vi2d checkStep = _location + offset;
        if(!Map::mapData->checkBounds(checkStep) )
                continue; // if the location is outside map skip it
        
        if(Map::mapData->checkFactionEnemy(faction->id(),checkStep)  ) // found enemy, attack it
            return attack(checkStep);
        
        // continue to check vector pathing for optimal paths
        for(auto [checkFact,checkDist] : Map::mapData->vectorPathField[checkStep.x][checkStep.y]){
            if(checkFact != faction->id() && checkDist < targetDist 
               && Map::mapData->checkForRoom(checkStep,faction->id() ) > checkStat(UnitStats::size) ){
                targetStep = checkStep;
                targetDist = checkDist;
            }
        }
    }
    return move(targetStep);
    
    return 1;// no action, wait for next turn
}

int Unit::move(olc::vi2d step){
    if(_location == step)
        return 1; // not moving, wait a turn
    
    // set target destination and clamp to map area
    float movetime = this->checkStat(UnitStats::moveSpeed) * (1 + Map::mapData->terrainSlope(step,_location) );
    Map::mapData->moveUnit(_location,step,checkStat(size),_mapIter);
    _location = step;
    return int(movetime);
}

int Unit::attack(olc::vi2d location){
    std::shared_ptr<Entity> target = Map::mapData->getUnit(location);
    if(target != nullptr && target->faction() != faction() ) // check target exists and is not friendly
        target->dealDamage(checkStat(UnitStats::actionEffect) );
    return checkStat(actionSpeed);

}

Miner::Miner(UnitRank rank,std::shared_ptr<Structure> home,std::shared_ptr<Faction> faction) 
: Unit(rank,home,faction) {
    
}

void Miner::onDeath(){
    // release any resource reservation
    if(_targetResource != olc::vi2d(-1,-1) ){
        Map::mapData->resourceNodes[_targetResource].reserved = false;
        _targetResource = olc::vi2d(-1,-1);
    }
    Unit::onDeath();
}

int Miner::update(){
    switch (_currentTask){
    case Planning :
        calculatePath();
        return 1;
    case Approaching :
        return moveToResource();
    case Mining :
        return mineFromResource();
    case Delivery :
        return returnHome();
    default:
        return 1;
    }
}

int Miner::calculatePath(){
    /// find closest unreserved node
    int distanceMetric = 999999999;
    olc::vi2d currentDirection = {-1,-1};
    for( auto& [location,node] : Map::mapData->resourceNodes){
        if(!node.reserved){
            olc::vi2d realitiveLoc = location - _location;
            int testMetric = (realitiveLoc.x * realitiveLoc.x) + (realitiveLoc.y * realitiveLoc.y);
            if (testMetric < distanceMetric){
                distanceMetric = testMetric;
                currentDirection = location;
            }
        }
    }
    if(currentDirection != olc::vi2d(-1,-1) ){
        _targetResource = currentDirection;
        Map::mapData->resourceNodes[currentDirection].reserved = true;
        _currentTask = Approaching;
    }
    /// determine best resource to target based on wait and distance?
    
    /// calculate path with time taken to get to resource (A* pathing)
    
    /// reserve slot on chosen resource including mining time
    
    return 1;
}

int Miner::moveToResource(){
    olc::vi2d direction = _targetResource - _location;
    direction = direction.clamp(olc::vi2d(-1,-1),olc::vi2d(1,1) );
    if(direction == olc::vi2d(0,0) ){ // arrived at resource
        _currentTask = Mining;
        return checkStat(UnitStats::actionSpeed);
    }
    return move(_location + direction);
    
}

int Miner::mineFromResource(){
    Map::mapData->resourceNodes[_targetResource].reserved = false;
    resourcesHeld = checkStat(UnitStats::actionEffect);
    _currentTask = Delivery;
    return 1;
}

int Miner::returnHome(){
    if(std::shared_ptr<Structure> home = _homeBase.lock() ){
        olc::vi2d direction = home->location() - _location;
        direction = direction.clamp(olc::vi2d(-1,-1),olc::vi2d(1,1) );
        if(direction == olc::vi2d(0,0) ){ // arrived at resource
            if(std::shared_ptr<Faction> faction = _faction.lock() ){
                faction->deliverResource(resourcesHeld);
                resourcesHeld = 0;
            }
            _currentTask = Planning;
            return checkStat(actionSpeed);
        }
    return move(_location + direction);
    }
    return -1; // error state - no home base
}

Structure::Structure(StructureRank rank, olc::vi2d location, std::shared_ptr<Faction> faction) 
: Entity(rank,faction),_location(location) {
    _hp = this->checkStat(StructureStats::maxHP);
}

Structure::~Structure() {
    // On deconstruction we clean up the Units the belong to this building
    for(std::shared_ptr<Unit> unit : _armyRegistry){
        unit->clearFromMap();
        unit->_homeBase.reset();
    }
    _armyRegistry.clear();
}

void Structure::honourDead(std::list<std::shared_ptr<Unit>>::iterator _iter){
    _armyRegistry.erase(_iter);
}

void Structure::statUpgrade(StructureStats stat, int amount){
     switch(stat) {
        case maxHP:
        upgrades.maxHP += amount;
        return;

    break;
        case buildSpeed:
         upgrades.actionSpeed+= amount;
         return;
    break;
        case buildCap:
        upgrades.actionEffect += amount;
        return;
    // size not upgradeable, deliberate exclusion.
    }
}
olc::vi2d Structure::location() {
    return _location;
}

void Structure::onDeath(){
    return;
}

int Structure::update() {
    std::shared_ptr<Faction> faction = _faction.lock();
    if(!faction)
        return -1; // error faction not avaliable?
    
    if(flag(builder) ){
        // if room build new unit
        if(_armyRegistry.size() < checkStat(StructureStats::buildCap) ){
            std::shared_ptr<Unit> soldier = EntityFactory::deploy(_buildable,_location,std::static_pointer_cast<Structure>(mySelf.lock() ),faction);
            _armyRegistry.emplace_back(soldier);
            soldier->_homeBaseIter = std::prev(_armyRegistry.end() );
            return checkStat(StructureStats::buildSpeed);
            
        }
    }
    return 1;// no action, wait for next turn
}

/// static factory for Units
std::shared_ptr<Unit> EntityFactory::deploy(UnitRank rank, olc::vi2d location, std::shared_ptr<Structure> home,std::shared_ptr<Faction> faction){
    std::shared_ptr<Unit> newUnit;
    if(rank == Gatherer) {
        std::shared_ptr<Miner> newMiner(new Miner(rank,home,faction) );
        newUnit = newMiner;
    } else {
        std::shared_ptr<Unit> newDeployment(new Unit(rank,home,faction) );
        newUnit = newDeployment;
    }
    
    newUnit->mySelf = newUnit;
    newUnit->_homeBase = home;
    newUnit->addToMap(location);
    TurnQueue::queue.addItem(newUnit);
    return newUnit;
}

///static factory for Structures
std::shared_ptr<Structure> EntityFactory::build(StructureRank baseClass,UnitRank unitClass, olc::vi2d location,std::shared_ptr<Faction> faction){
    std::shared_ptr<Structure> newBase(new Structure(baseClass,location,faction) );
    
    newBase->mySelf = newBase;
    newBase->set(builder);
    newBase->_buildable = unitClass;
    TurnQueue::queue.addItem(newBase);
    return newBase;
}
