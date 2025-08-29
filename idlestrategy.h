#ifndef UNIT_DEFINED
#define UNIT_DEFINED


class Map{
    public:
        Map();
        Map(int width, int height);
        
        olc::vi2d br;
        
        olc::Sprite* terrainS;
        olc::Decal* terrainD;
        
        std::vector<std::vector<float>> gridHeight;
        
        //Key is location, second key is faction, last is count of units in that location
        std::map<olc::vi2d,std::map<int,int>> unitLocations;
        int cellCapacity = 16; 
        std::map<int,olc::Pixel> factionColours;
        // methods
        int checkForRoom(olc::vi2d loc);
        void addUnit(olc::vi2d loc, int faction, int size);
        void removeUnit(olc::vi2d, int faction, int size);
        float terrainSlope(olc::vi2d start, olc::vi2d end);
        olc::vi2d findEnemy(olc::vi2d initalPos, int yourFaction);
        
};

class Unit{
    protected:
    static std::shared_ptr<Map> _mapData;
    public:
    static void setMap(std::shared_ptr<Map> mapData);
    public:

        //constructors
        Unit();
        Unit(olc::vi2d location,int faction,int size = 1);
        ~Unit(){};
        
        //Getters and Setters
        olc::vi2d location();
        float getHP();
        int getFaction();
        olc::Pixel getFactionColour();
        
        void setTarget(olc::vi2d target);
        
        //methods
        bool operator <(const Unit& o) const;

        
        int update();
        int move();
        
        int nextTurn = 0;
    protected:
        float _hp = 100;
        int _faction;
        olc::vi2d _target;
        olc::vi2d _location;
        int _size;
        int _moveSpeed = 5;
        
};

class Base : public Unit{
    public:
        Base();
        Base(olc::vi2d location,int faction);
        ~Base(){};
        
        //getters and Setters
        
        void setOpponent(olc::vi2d opposition);
        
        //methods
        int move();
        int update(std::vector<std::weak_ptr<Unit>> &navy);
    
    private:
        olc::vi2d _target;
        int _unitMax = 10;
        int _buildSpeed = 15;
        int _cooldown;
        std::vector<std::shared_ptr<Unit>> army;
    
};
#endif
