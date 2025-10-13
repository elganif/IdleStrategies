#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"

#include <set>

#include "button.h"
#include "factions.h"
#include "entity.h"



const statBlock Faction::stats[COUNT_OF_RANKS] = {
    {"Base", 1000, 0, 0,  60, 10},
    {"Marine",100, 1, 10,  7, 10},
    {"Scout",  75, 1,  6, 10, 5},
    {"Tank",  150, 4, 15, 10, 20}
};

int Faction::factionCounter = 0;

Faction::Faction(olc::vi2d location,olc::Pixel colour):_id(factionCounter++),_location(location),_colour(colour){
    // initialize upgrade array to zeros
    for(int i  = 0; i < COUNT_OF_RANKS; i++){
        upgrades[i] = {"",0,0,0,0,0};
    }
}

    
Faction::~Faction(){
    for(auto building : structures){
        building.reset();
    }
}

int Faction::id(){ return _id; }
olc::Pixel Faction::colour(){ return _colour; }

bool Faction::operator ==(const Faction &other) const{
    return this->_id == other._id;
}

std::shared_ptr<Entity> Faction::makeBase(ranks rank, ranks unit){
    std::shared_ptr<Entity> newBase = Entity::makeBase(rank,unit,_location,shared_from_this() );
    structures.push_back(newBase);
    buildingUpgrades.push_back({"",0,0,0,0,0});
    return newBase;
}

std::shared_ptr<Entity> Faction::getBase(int index){
    if(index < structures.size() && index > 0)
        return structures[index];
    return nullptr;
}

/// Returns the value of a stat using static base + faction upgrade. (Might include percent increases later?)
int Faction::checkStat(ranks rank, statIndex stat){
    switch(stat) {
        case initalHP:
        return stats[rank].initalHP + upgrades[rank].initalHP;
    
        case size:
        return stats[rank].size; // not upgradeable or adjustable
    
        case moveSpeed:
        return stats[rank].moveSpeed + upgrades[rank].moveSpeed;
    
        case actionSpeed:
        return stats[rank].actionSpeed + upgrades[rank].actionSpeed;
    
        case actionEffect:
        return stats[rank].actionEffect + upgrades[rank].actionEffect;
    }
    return 0;
}

std::string Faction::getName(ranks rank){
    return stats[rank].typeName;
}

void Faction::improveStat(ranks rank, statIndex stat, int amount){
    switch(stat) {
        case initalHP:
        upgrades[rank].initalHP+= amount;
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
