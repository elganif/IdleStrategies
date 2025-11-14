#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"

#include <queue>
#include <set>

#include "engine.h"
#include "maping.h"
#include "button.h"
#include "factions.h"
#include "entity.h"

const StructureStatBlock Faction::structureStats[COUNT_OF_STRUCTURE_RANKS] = {
    {"Base", 1000, 60, 10, Gatherer},
    {"Barracks",500,60,10, Marine},
};

const UnitStatBlock Faction::unitStats[COUNT_OF_UNIT_RANKS] = {
    {"Miner",  75, 15, 5, 9, 1},
    {"Marine",100, 10, 10, 15, 1},
    {"Tank",  200, 20, 15, 25, 4}
};

int Faction::factionCounter = 0;

Faction::Faction(olc::vi2d location,olc::Pixel colour):_id(factionCounter++),_location(location),_colour(colour){
    // initialize upgrade array to zeros
    for(int i  = 0; i < COUNT_OF_UNIT_RANKS; i++){
        upgrades[i] = {"",0,0,0,0,0};
    }
    Map::mapData->setBaseLocation(location,id() );
    Map::mapData->factionColours[id()] = colour;
    drawingS = new olc::Sprite(3,3);
    for(int y = 0; y < 3; y++)
        for(int x = 0; x < 3; x++)
            drawingS->SetPixel(x,y,olc::Pixel(0,0,0,0) );
    drawingD = new olc::Decal(drawingS);
}

    
Faction::~Faction(){
    for(auto building : structures){
        building.reset();
    }
    delete drawingD;
    delete drawingS;
}

int Faction::id(){
    return _id; 
}

olc::Pixel Faction::colour(){ 
    return _colour;
}

olc::vf2d Faction::location(){
    return _location;
}

int Faction::resource(){
    return _resources;
}

bool Faction::operator ==(const Faction &other) const{
    return this->_id == other._id;
}

std::shared_ptr<Player> Faction::setAsPlayer(olc::vi2d tlControl,olc::vi2d brControl,olc::PixelGameEngine* engine){
    std::shared_ptr<Player> tmp = std::make_shared<Player>(shared_from_this(), tlControl,brControl ,engine);
    _controller = tmp;
    return tmp;
}

void Faction::setAsComputer(){
    _controller = std::make_shared<Computer>(shared_from_this() );
    _controller->initialize();
}

std::shared_ptr<Entity> Faction::makeBase(int index, StructureRank rank, UnitRank unit){
    std::shared_ptr<Structure> newBase = EntityFactory::build(rank,unit,_location,shared_from_this() );
    olc::vi2d pixelLoc = olc::vi2d(index%3,index/3);
    drawingS->SetPixel(pixelLoc,_colour);
    structures[index] = newBase;
    drawingD->Update(); 
    return newBase;
}

std::shared_ptr<Entity> Faction::getBase(int index){
    if(index < numBuildings)
        return structures[index];
    return nullptr;
}



/// Returns the value of a stat using static base + faction upgrade. (Might include percent increases later?)
int Faction::checkStat(UnitRank rank, UnitStats stat){
    switch(stat) {
        case maxHP:
        return unitStats[rank].initialHP + upgrades[rank].initialHP;
    
        case size:
        return unitStats[rank].size; // not upgradeable or adjustable
    
        case moveSpeed:
        return unitStats[rank].moveSpeed + upgrades[rank].moveSpeed;
    
        case actionSpeed:
        return unitStats[rank].actionSpeed + upgrades[rank].actionSpeed;
    
        case actionEffect:
        return unitStats[rank].actionEffect + upgrades[rank].actionEffect;
    }
    return 0;
}

int Faction::checkStat(StructureRank rank, StructureStats stat){
    switch(stat) {
        case maxHP:
        return structureStats[rank].maxHP;
    
        case buildSpeed:
        return structureStats[rank].actionSpeed;
    
        case buildCap:
        return structureStats[rank].actionEffect;
    }
    return 0;
}

std::string Faction::getName(UnitRank rank){
    return unitStats[rank].typeName;
}

std::string Faction::getName(StructureRank rank){
    return structureStats[rank].typeName;
}

void Faction::improveStat(int structureIndex, StructureStats stat, int amount){
    structures[structureIndex]->statUpgrade(stat,amount);
}

void Faction::improveStat(UnitRank rank, UnitStats stat, int amount){
    switch(stat) {
        case initialHP:
        upgrades[rank].initialHP += amount;
        return;
    break;
        case moveSpeed:
         upgrades[rank].moveSpeed+= amount;
         return;
    break;
        case actionSpeed:
         upgrades[rank].actionSpeed+= amount;
         return;
    break;
        case actionEffect:
        upgrades[rank].actionEffect += amount;
        
    // size not upgradeable, deliberate exclusion.
    }
}
void Faction::deliverResource(int amount){
    _resources += amount;
}

olc::Decal* Faction::drawing() {
    return drawingD;
}
olc::vf2d Faction::drawSize(){
    return drawingS->Size();
}
