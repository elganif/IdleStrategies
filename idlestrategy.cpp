#include "olcPixelGameEngine.h"
#include "idlestrategy.h"
#include "OpenSimplexNoise.hh"

#include <queue>

Map::Map(){};

Map::Map(int width, int height) : br({width,height}) {

    OSN::Noise<2> perlin;
    
    terrainS = new olc::Sprite(br.x,br.y);
    
    gridHeight.resize(br.x);
    for (int x = 0; x < br.x; x++)
        gridHeight[x].resize(br.y);
        
    for(int y = 0; y < br.y; y++)
        for(int x = 0; x < br.x; x++)
        {
            gridHeight[x][y] = (perlin.eval(float(x)*0.1f,float(y)*0.1f) + 1.0f) * 0.5f;
            float temp = (gridHeight[x][y]) * 255.0f;
            terrainS->SetPixel(x,y,olc::Pixel(int(temp),int(temp),int(temp)));
    }
    
    terrainD = new olc::Decal(terrainS);
}

int Map::checkForRoom(olc::vi2d loc){
    if(unitLocations.count(loc) == 0) // location not registered
        return cellCapacity;
        
    int pop = 0;
    for(auto& count : unitLocations[loc])
    {
        pop += count.second;
    }
    return cellCapacity - pop;
}

void Map::addUnit(olc::vi2d loc, int faction, int size){
    if(unitLocations.count(loc) == 0 || unitLocations[loc].count(faction) == 0)
    {
        unitLocations[loc][faction] = size;
        return;
    }
    unitLocations[loc][faction] += size;
    return;
}

void Map::removeUnit(olc::vi2d loc, int faction, int size){
    if(unitLocations.count(loc) == 0 || unitLocations[loc].count(faction) == 0)
        return; // if we are here there is an error : this faction has no registered units at location
    unitLocations[loc][faction] -= size;
    if(this->checkForRoom(loc) == cellCapacity)
    {
        unitLocations.erase(loc);
        return;
    }
}
float Map::terrainSlope(olc::vi2d start, olc::vi2d end){
    return abs(gridHeight[end.x][end.y] - gridHeight[start.x][start.y]);
}
olc::vi2d Map::findEnemy(olc::vi2d initialPos, int yourFaction){
    // array to allow sequential checking of neighbors
    const olc::vi2d compass[8] = {{0,-1},{0,1},{-1,0},{1,0},{-1,-1},{-1,1},{1,1},{1,-1}};
    
    std::unordered_map<int,std::unordered_map<int,bool>> checked;
    std::pair<float,olc::vi2d> currentCell = {0,initialPos};
    // create priority queue - difficulty distance as priority function
    class MyCompare {
        public:
            bool operator()(std::pair<float,olc::vi2d> a, std::pair<float,olc::vi2d> b) {
                return a.first < b.first;
            }
    };
    std::priority_queue<std::pair<float,olc::vi2d>,std::vector<std::pair<float,olc::vi2d>>,MyCompare> cellList;
    // insert start as 0 difficulty
    cellList.push(currentCell);
    checked[currentCell.second.x][currentCell.second.y] = true;
    // pop out first element and check for targets.  
    while(!cellList.empty()){
        currentCell = cellList.top();
        cellList.pop();
        for(int i = 0;i < 8; i++)
        {
            if(checked.find(currentCell.second.x) == checked.end() )
            {
                int weight = (i<4) ? 1.0f : 1.4f;
                
            }
            
        }
    
    }
    
    // calculate each neighbor difficulty distance and add to queue
    
    
    
    return olc::vi2d(0,0);
}

// Static variables and methods in Unit
std::shared_ptr<Map> Unit::_mapData;
void Unit::setMap(std::shared_ptr<Map> mapData)
{
    _mapData = mapData;
} 

    
Unit::Unit(){};

Unit::Unit(olc::vi2d location,int faction,int size) 
: _location(location.clamp({0,0},_mapData->br)),_faction(faction),_size(size)
{
    _mapData->addUnit(_location,_faction,_size);
}

//Getters and Setters
olc::vi2d Unit::location()
{
    return _location;
}
int Unit::getFaction()
{
    return _faction;
}
olc::Pixel Unit::getFactionColour(){
    return _mapData->factionColours[_faction];
}
float Unit::getHP()
{
    return _hp;
}

void Unit::setTarget(olc::vi2d target)
{
    _target = target;
}
//Methods
bool Unit::operator <(const Unit& o) const
{
    return nextTurn > o.nextTurn;
}


int Unit::move()
{
    // TODO:replace with pathfinding
    
    float movetime = _moveSpeed;
    
    // needs pathfinding algorithm
    olc::vi2d diff = _target - _location;
    olc::vi2d step = diff.clamp({-1,-1},{1,1});
    
    //olc::vi2d target = _mapData->findEnemy(_location,1);
    
    // set target destination and clamp to map area
    olc::vi2d target = (_location + step).clamp({0,0},_mapData->br);
    if(_size <= _mapData->checkForRoom(target))
    {
        float startheight = _mapData->gridHeight[_location.x][_location.y];
        _mapData->removeUnit(_location,_faction,_size);
        _location = target;
        float endHeight = _mapData->gridHeight[_location.x][_location.y];
        _mapData->addUnit(_location,_faction,_size);
        if(step.x != 0 && step.y != 0)
        {
            movetime = movetime * 1.4;
        }
        movetime = movetime * (1 + std::abs(endHeight - startheight));
    }
    
    if(_location == _target)
        _hp = 0;
    return int(movetime);
}

int Unit::update()
{
    return move();
}



Base::Base(){};

Base::Base(olc::vi2d location, int faction)
{
    _cooldown = 0;
    _location = location;
    _faction = faction;
}

void Base::setOpponent(olc::vi2d opposition)
{
    _target = opposition;
}

int Base::move() {return 0;}
 
int Base::update(std::vector<std::weak_ptr<Unit>> &navy)
{
    
    for(auto soldier = army.begin(); soldier != army.end();/*soldier++*/)
    {
        if((*soldier)->getHP() <= 0)
        {
            soldier = army.erase(soldier);
        }
        else
        {
            ++soldier;
        }   
        
    }
    
    if(_cooldown > 0)
    {
        _cooldown--;
        return 1;
    }
    if(army.size() >= _unitMax)
    {
        return 1;
    }
    
    std::shared_ptr<Unit> soldier = std::shared_ptr<Unit>(new Unit(_location,_faction));
    soldier->setTarget(_target);
    army.emplace_back(soldier);
    navy.emplace_back(std::weak_ptr<Unit>(soldier));
    _cooldown = _buildSpeed;
    return 1;
    
}
