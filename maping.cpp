
#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "OpenSimplexNoise.hh"

#include <queue>
#include <set>
#include <cmath>

#include "factions.h"
#include "entity.h"
#include "maping.h"

/// comparitor for priority queues to sort closest to furthest 
class distancePriorityCompare {
    public:
        bool operator()(std::pair<float,olc::vi2d> a, std::pair<float,olc::vi2d> b) {
            return a.first > b.first;
        }
        bool operator()(std::tuple<float,int,olc::vi2d> a, std::tuple<float,int,olc::vi2d> b) {
            return std::get<0>(a) > std::get<0>(b);
        }
};

Map::Map(int width, int height) : br({width,height}) {

    OSN::Noise<2> perlin;
    
    
    //Size gridHeight and vectorPathFeild vectors so we can use indexed access to cells, initialization follows
    gridHeight.resize(br.x);
    vectorPathField.resize(br.x);
    for (int x = 0; x < br.x; x++){
        gridHeight[x].resize(br.y);
        vectorPathField[x].resize(br.y);
    }
    
    //lambda to calculate perlin noise and scale to between 0 and 1 (default function gives -1 to 1)
    auto perlVal = [perlin](float x,float y){float p = perlin.eval(float(x),float(y)); return 1 - (p * p);};
    float perlScale = 0.1f;
    for(int y = 0; y < br.y; y++)
        for(int x = 0; x < br.x; x++)
        {
            gridHeight[x][y] = perlVal(float(x)*perlScale,float(y)*perlScale);
    }
    
    terrainS = new olc::Sprite(br.x * graphicalDetail,br.y * graphicalDetail);
    float pixelSize = perlScale / graphicalDetail;
    
      // shaded map
    for(int y = 0; y < terrainS->Size().y; y++)
        for(int x = 0; x < terrainS->Size().x; x++){
            float value = (perlin.eval(float(x)*pixelSize,float(y)*pixelSize) + 1.0f) * 0.5f;
            float temp = value * 255.0f;
            terrainS->SetPixel(x,y,olc::Pixel(int(temp),int(temp),int(temp)));
    } //*/
    // height map
    int numLines = 10;
    
    for(int y = 1; y < terrainS->Size().y -1; y++)
        for(int x = 1; x < terrainS->Size().x -1; x++){
            float center = perlVal(x*pixelSize,y*pixelSize);
            
            float north= perlVal(x*pixelSize,(y-1)*pixelSize);
            float south= perlVal(x*pixelSize,(y+1)*pixelSize);
            
            float west= perlVal((x-1)*pixelSize,y*pixelSize);
            float east= perlVal((x+1)*pixelSize,y*pixelSize);
            
            for(int i = 0; i < numLines;i++){
                float check = float(i) / float(numLines);
                if(north < check != south < check && center > check)
                    terrainS->SetPixel(x,y,olc::Pixel(0,check*255,0));
                if(west < check != east < check && center > check )
                    terrainS->SetPixel(x,y,olc::Pixel(0,check*255,0));
            }
    }
    
    terrainD = new olc::Decal(terrainS);
}

Map::~Map(){
    for(auto [location,unitData] : unitLocations){
        unitData.units.clear();
    }
    delete terrainD;
    delete terrainS;
}

void Map::generateResources(int amount){
    for(int i = 0;i < amount; /*i++ when node added*/){
        int randomx = rand() % br.x;
        int randomy = rand() % br.y;
        if(resourceNodes.find(olc::vi2d(randomx,randomy)) == resourceNodes.end()){
            resourceNodes[olc::vi2d(randomx,randomy)] = 10;
            i++;
        }
    }
    
}
/// generates vector maps for finding each faction
void Map::generateVectorMap(){
    
    std::priority_queue<std::tuple<float,int,olc::vi2d>,std::vector<std::tuple<float,int,olc::vi2d>>,distancePriorityCompare> cellList;
    
    std::vector<std::vector<std::set<int>>> visited;
    visited.resize(br.x);
    for (int x = 0; x < br.x; x++){
        visited[x].resize(br.y);
    }
    
    for(auto [loc,cell]:unitLocations){
        cellList.push({0.0f,cell.faction ,loc});
        vectorPathField[loc.x][loc.y][cell.faction] = 0.0f;
        visited[loc.x][loc.y].insert(cell.faction);
    }
    
    while(!cellList.empty())
    {
        float thisWeight = std::get<0>(cellList.top());
        int thisFaction = std::get<1>(cellList.top());
        olc::vi2d thisLoc = std::get<2>(cellList.top());
            
        cellList.pop();
        for(olc::vi2d offset : compass)
        {
            olc::vi2d nextCell = thisLoc + offset;
            
            if(!checkBounds(nextCell))
                continue;// if cell out of bounds, skip
            if(visited[nextCell.x][nextCell.y].find(thisFaction) == visited[nextCell.x][nextCell.y].end()){
                
                float nextWeight = thisWeight + (1 + terrainSlope(thisLoc,nextCell ));
                vectorPathField[nextCell.x][nextCell.y][thisFaction] = nextWeight;
                visited[nextCell.x][nextCell.y].insert(thisFaction);
                cellList.push({nextWeight,thisFaction,nextCell});
            }
        }
    }
    
};

/// Returns how difficult the terrain is to travel between 2 neighboring nodes
float Map::terrainSlope(olc::vi2d start, olc::vi2d end){
    // TODO: should I verify cells are neighbors?
    float diff = gridHeight[end.x][end.y] - gridHeight[start.x][start.y];
    if(start.x == end.x || start.y == start.y)
        return diff;
    return diff * 1.414f;
}

/// sets base locations
void Map::setBaseLocation(olc::vi2d loc, int faction){
    baseLocations[faction] = loc;
}

/// Returns Capacity remaining at loc
int Map::checkForRoom(olc::vi2d loc,int faction){
    if(unitLocations.find(loc) == unitLocations.end()) 
        return cellCapacity;// location not registered, full capacity is avaliable
    
    if(faction != unitLocations[loc].faction)
        return 0; // location occupied by another faction, no room
        
    int pop = 0;
    for(auto& unit : unitLocations[loc].units){
        if(unit->alive()){
            pop += unit->getSize();
        }
    }
    return cellCapacity - pop;
}

/// adds new unit to map at location
std::list<std::shared_ptr<Entity>>::iterator Map::addUnit(olc::vi2d location, std::shared_ptr<Entity> unit){
    if(unitLocations.find(location) == unitLocations.end() ){
        unitLocations[location].faction = unit->faction();
    }
    unitLocations[location].units.push_back(unit);
    return std::prev(unitLocations[location].units.end());
}

/// moves unit iterator from one map location to another without iterator invalidation
void Map::moveUnit(olc::vi2d origin, olc::vi2d destination, std::list<std::shared_ptr<Entity>>::iterator unitIter){
    if(unitLocations.find(destination) == unitLocations.end() ){
        unitLocations[destination].faction = (*unitIter)->faction();
    }
    
    unitLocations[destination].units.splice(unitLocations[destination].units.end(),unitLocations[origin].units,unitIter);
    
    // clean up any empty lists
    if(unitLocations[origin].units.empty())
        unitLocations.erase(origin);
}

/// remove unit tracker and cleans up memory locations if needed
void Map::removeUnit(olc::vi2d loc,std::list<std::shared_ptr<Entity>>::iterator unitIter){
    if(unitLocations.find(loc) == unitLocations.end() )
        return; // location not registered, no units to remove. (should not happen, but safety check)
        
    unitLocations[loc].units.erase(unitIter);
    
    // clean up location and faction keys if no longer needed
    if(unitLocations[loc].units.empty())
        unitLocations.erase(loc);
}

/// checks if a cell has units listed at location that are not in the same faction 
/// (Could this be an area check? return a unit to attack directly? considerations) 
bool Map::checkFactionEnemy(int yourFaction,olc::vi2d location){
    if(unitLocations.find(location) == unitLocations.end())
        return false; //location not registered, no enemies
    if(unitLocations[location].faction != yourFaction){
        return true;
    }
    return false;
}

/// returns shared_ptr to a random unit within the given location
std::shared_ptr<Entity> Map::getUnit(olc::vi2d location){
    if(unitLocations.find(location) == unitLocations.end())
        return std::shared_ptr<Entity>(); //location not registered, null return (caller should check)
    
    
    int randNum = rand() % unitLocations[location].units.size();
    auto randUnit = unitLocations[location].units.begin();
    std::advance(randUnit,randNum);
    return *randUnit;
}

bool Map::checkBounds(olc::vi2d location){
    if(location.x < 0 || location.x >= br.x)
        return false;
    if(location.y < 0 || location.y >= br.y)
        return false;
    return true;
}
