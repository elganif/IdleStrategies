#ifndef MAP_DEFINED
#define MAP_DEFINED
//forward declare class Entity
class Entity;

struct HASH_OLC_VI2D {
    std::size_t operator ()(const olc::vi2d&v) const
    {
        return int64_t(v.y << sizeof(int32_t) | v.x);
    }
};
// used to iterate through neighboring cells starting with north and clockwise
const olc::vi2d compass[8] = {{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1}};

class Map{
    private:
    struct factionList{int faction;std::list<std::shared_ptr<Entity>> units;};
    public:
        Map(int width, int height);
        ~Map();
        // methods
        void generateResources(int amount);
        void generateVectorMap();
        float terrainSlope(olc::vi2d start, olc::vi2d end);
        
        void setBaseLocation(olc::vi2d loc,int faction);
        
        int checkForRoom(olc::vi2d loc,int faction);
        
        std::list<std::shared_ptr<Entity>>::iterator addUnit(olc::vi2d loc, std::shared_ptr<Entity> unit);
        void moveUnit(olc::vi2d origin, olc::vi2d destination, std::list<std::shared_ptr<Entity>>::iterator unit);
        void removeUnit(olc::vi2d loc, std::list<std::shared_ptr<Entity>>::iterator unit);
        std::shared_ptr<Entity> getUnit(olc::vi2d location);
        bool checkFactionEnemy(int faction,olc::vi2d location);
        
        bool checkBounds(olc::vi2d location);
        
        
        //variables
        olc::vi2d br; //bottomRight corner index
        const int graphicalDetail = 10;
        const int cellCapacity = 16;
        
        
        std::map<int,olc::Pixel> factionColours;
        std::map<int,olc::vi2d> baseLocations;
        std::unordered_map<olc::vi2d,int,HASH_OLC_VI2D> resourceNodes;
        std::unordered_map<olc::vi2d,factionList,HASH_OLC_VI2D> unitLocations; //Location, faction, list of units
        
        std::vector<std::vector<float>> gridHeight;
        std::vector<std::vector<std::map<int,float>>> vectorPathField; // x,y,faction number, distance metric
        olc::Sprite* terrainS;
        olc::Decal* terrainD;
};

#endif
