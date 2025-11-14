#ifndef ENTITY_DEFINED
#define UNTITY_DEFINED

#include <variant>

// forward declarations
class EntityFactory;
class Map;
class Faction;
class Unit;
class Structure;

enum FLAGS{
    builder,
};

class Entity : public in_queue
{
protected:
    Entity() = delete;
    Entity(std::variant<UnitRank,StructureRank> rank,std::shared_ptr<Faction> faction);
    friend EntityFactory;
public:
    ~Entity();

    int faction();
    olc::Pixel factionColour();
    bool alive();
    float getHP();

    std::string getName();
    
    Entity& set(FLAGS flag);
    Entity& unset(FLAGS flag);
    bool flag(FLAGS flag);
    
    void dealDamage(int dam);
    
    /// Entity virtual functions 
    virtual olc::vi2d location() = 0;
    virtual int checkStat(std::variant<UnitStats,StructureStats> stat) = 0;
    virtual void onDeath() = 0;
    
    /// overrides for in_queue interface
    bool can_queue() override;
    virtual int update() = 0;

protected:
    std::variant<UnitRank,StructureRank> _myRank;
    std::weak_ptr<Entity> mySelf;
    std::weak_ptr<Faction> _faction;
    bool _alive = true;
    float _hp;
    
    std::set<FLAGS> _flags;
};

class Unit : public Entity, public in_map
{
protected:
    Unit() = delete;
    Unit(UnitRank rank, std::shared_ptr<Structure> home, std::shared_ptr<Faction> faction);
    friend EntityFactory;
    friend Structure;
public:
    ~Unit();
    virtual void onDeath();

    /// Entity overrides
    olc::vi2d location() override;
    int checkStat(std::variant<UnitStats,StructureStats> stat) override;
    
    /// in_map overrides
    void addToMap(olc::vi2d location) override;
    //void clearFromMap() override;
    
    /// in_queue overrides
    int update() override;
    
protected:
    virtual int move(olc:: vi2d location);
    int attack(olc::vi2d location);
    
    std::weak_ptr<Structure> _homeBase;
    std::list<std::shared_ptr<Unit>>::iterator _homeBaseIter;
};

class Miner : public Unit
{
    enum MinerState {
        Planning,
        Approaching,
        Mining,
        Delivery,
    };
    Miner() = delete;
    Miner(UnitRank rank,std::shared_ptr<Structure> home,std::shared_ptr<Faction> faction);
    friend EntityFactory;
public:
    
    void onDeath() override;
    int update() override;
    
private:
    int calculatePath();
    int moveToResource();
    int mineFromResource();
    int returnHome();
    
    MinerState _currentTask = Planning;
    olc::vi2d _targetResource = {-1,-1};
    int resourcesHeld = 0;
    // TODO : container for path to resource once calculated
};

class Structure : public Entity
{
protected:
    Structure() = delete;
    Structure(StructureRank rank,olc::vi2d location,std::shared_ptr<Faction> faction);
    friend EntityFactory;
public:
    ~Structure();
    void honourDead(std::list<std::shared_ptr<Unit>>::iterator _iter);
    void statUpgrade(StructureStats stat, int amount);

    /// Entity override
    olc::vi2d location() override;
    int checkStat(std::variant<UnitStats,StructureStats> stat) override;
    void onDeath() override;

    /// in_queue override
    int update() override;
    
private:
    StructureStatBlock upgrades;
    olc::vi2d _location;
    UnitRank _buildable;
    std::list<std::shared_ptr<Unit>> _armyRegistry;
};

class EntityFactory{
private:
    EntityFactory() = delete;
public:
    static std::shared_ptr<Unit> deploy(UnitRank rank, olc::vi2d location,std::shared_ptr<Structure> home, std::shared_ptr<Faction> faction);
    static std::shared_ptr<Structure> build(StructureRank baseRank,UnitRank unitRank, olc::vi2d location,std::shared_ptr<Faction> faction);
};
#endif
