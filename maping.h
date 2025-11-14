#ifndef MAP_DEFINED
#define MAP_DEFINED
//forward declare class Entity
class Unit;
class in_map;

struct HASH_OLC_VI2D {
    std::size_t operator ()(const olc::vi2d&v) const
    {
        return int64_t(v.y << sizeof(int32_t) | v.x);
    }
};
// used to iterate through neighboring cells starting with north and clockwise


struct Resource 
{
    /// negitive resource amounts are treated as infinite
    Resource(int turn = 0,int amount = -1):_turnReserve(turn),_amount(amount){};
    bool reserved = false;
    int _turnReserve;
    int _amount;
};

class Map{
public:

    const olc::vi2d compass[8] = {{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1}};
    static void createMap(int x, int y);
    static std::unique_ptr<Map> mapData;
private:
    struct nodeData{
        int faction = -1;
        int occupation = 0;
        std::list<std::weak_ptr<in_map>> units;
    };
public:
    Map(int width, int height);
    ~Map();
    
    void generateResources(int count);
    void generateVectorMap();
    float terrainSlope(olc::vi2d start, olc::vi2d end);
    
    void setBaseLocation(olc::vi2d loc,int faction);
    
    int checkForRoom(olc::vi2d loc,int faction);
    
    std::list<std::weak_ptr<in_map>>::iterator addUnit(olc::vi2d loc,int size, std::shared_ptr<Unit> unit);
    void moveUnit(olc::vi2d origin, olc::vi2d destination,int size, std::list<std::weak_ptr<in_map>>::iterator unit);
    void removeUnit(olc::vi2d loc, std::list<std::weak_ptr<in_map>>::iterator unit);
    std::shared_ptr<Unit> getUnit(olc::vi2d location);
    bool checkFactionEnemy(int faction,olc::vi2d location);
    
    bool checkBounds(olc::vi2d location);
    
    
    olc::vi2d br; //bottomRight corner index
    const int graphicalDetail = 10;
    const int cellCapacity = 16;
    
    
    std::map<int,olc::Pixel> factionColours;
    std::map<int,olc::vi2d> baseLocations;
    std::unordered_map<olc::vi2d,Resource,HASH_OLC_VI2D> resourceNodes;
    std::unordered_map<olc::vi2d,nodeData,HASH_OLC_VI2D> unitLocations; //Location, faction, list of units
    
    std::vector<std::vector<float>> gridHeight;
    std::vector<std::vector<std::map<int,float>>> vectorPathField; // x,y,faction number, distance metric
    olc::Renderable terrain;
};

class in_map
{
    public:
    in_map();
    ~in_map();
    virtual void addToMap(olc::vi2d location) = 0;
    virtual void clearFromMap();
    
    std::list<std::weak_ptr<in_map>>::iterator _mapIter;
    olc::vi2d _location = {-1,-1};
};
#endif
