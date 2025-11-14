#ifndef FACTIONS
#define FACTIONS

class Entity;
class Button;
class Controller;
class Player;
class Structure;

enum UnitRank {
    Gatherer,
    Marine,
    Tank,
    COUNT_OF_UNIT_RANKS,
};

enum UnitStats {
    unitName,
    initialHP,
    actionSpeed, // unit attack speed or structure build speed
    actionEffect,// unit damage, or sturcture build cap
    size,
    moveSpeed,
};

struct UnitStatBlock{
    std::string typeName;
    int initialHP;
    int actionSpeed;
    int actionEffect;
    int moveSpeed;
    int size;
};

enum StructureRank {
    CommandCenter,
    Barracks,
    Garage,
    COUNT_OF_STRUCTURE_RANKS
};

enum StructureStats {
    structureName,
    maxHP,
    buildSpeed, 
    buildCap,
    productionUnit,
    
};

struct StructureStatBlock{
    std::string typeName;
    int maxHP;
    int actionSpeed;
    int actionEffect;
    UnitRank productionUnit;
};

class Faction : public std::enable_shared_from_this<Faction> {
public:
    Faction(olc::vi2d location, olc::Pixel colour);
    ~Faction();
    int id();
    olc::Pixel colour();
    olc::vf2d location();
    int resource();
    bool operator==(const Faction &other) const;
    
    std::shared_ptr<Entity> makeBase(int index,StructureRank rank, UnitRank unit);
    std::shared_ptr<Entity> getBase(int index);
    std::shared_ptr<Player> setAsPlayer(olc::vi2d tlControl,olc::vi2d brControl,olc::PixelGameEngine* engine);
    void setAsComputer();
    
    int checkStat(UnitRank rank, UnitStats stat);
    int checkStat(StructureRank rank, StructureStats stat);
    std::string getName(UnitRank rank);
    std::string getName(StructureRank rank);
    void improveStat(UnitRank rank, UnitStats stat, int amount = 1);
    void improveStat(int structureIndex, StructureStats stat, int amount = 1);
    void deliverResource(int amount);
    olc::Decal* drawing();
    olc::vf2d drawSize();
    
private:
    const static int numBuildings = 9;
    static int factionCounter;
    std::shared_ptr<Controller> _controller;

    olc::vi2d _location;
    const int _id;
    olc::Pixel _colour;
    int _resources = 0;
    olc::Sprite* drawingS;
    olc::Decal* drawingD;
    
    static const UnitStatBlock unitStats[COUNT_OF_UNIT_RANKS]; //core stats of units. Does not change
    static const StructureStatBlock structureStats[COUNT_OF_STRUCTURE_RANKS];
    UnitStatBlock upgrades[COUNT_OF_UNIT_RANKS]; // upgrades to units belonging to this faction
    
    std::shared_ptr<Structure> structures[numBuildings];
};


#endif
