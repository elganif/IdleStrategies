#ifndef ENTITY_DEFINED
#define UNTITY_DEFINED

// forward declarations
class Map;
class Faction;

enum FLAGS{
    builder,
    attacker,
    miner,
    mobile,
    
};
    
class Entity : public std::enable_shared_from_this<Entity>
{
    public:
        static std::unique_ptr<Map> mapData;
        static void setMap(int x, int y);
        static std::shared_ptr<std::list<std::shared_ptr<Entity>>> bootCamp;

//constructors and object initialization
    private:
        Entity() = delete;
        Entity(ranks assignment, olc::vi2d location,std::shared_ptr<Faction> faction);
        void addToMap();
    public:
        ~Entity();
        static std::shared_ptr<Entity> makeUnit(ranks rank, olc::vi2d location,std::shared_ptr<Entity> home, std::shared_ptr<Faction> faction);
        static std::shared_ptr<Entity> makeBase(ranks baseRank,ranks unitRank, olc::vi2d location,std::shared_ptr<Faction> faction);
        
//Getters and Setters
        int faction();
        olc::Pixel factionColour();
        olc::vi2d location();
        bool alive();
        float getHP();
        int checkStat(statIndex stat);
        int getSize();
        std::string getName();
        
        Entity& set(FLAGS flag);
        Entity& unset(FLAGS flag);
        bool flag(FLAGS flag);
        
        int update();
        int move(olc:: vi2d location);
        int attack(olc::vi2d location);
        void dealDamage(int dam);
        void onKill();
        void honourDead(std::list<std::weak_ptr<Entity>>::iterator _iter);
        
        int nextTurn = 0;
        
    private:
        std::list<std::shared_ptr<Entity>>::iterator _mapIter;
        std::list<std::weak_ptr<Entity>>::iterator _homeIter;
        std::weak_ptr<Entity> _homeBase;
        std::weak_ptr<Faction> _faction;
        
        ranks _rank;
        olc::vi2d _location;
        bool _alive = true;
        float _hp;
        
        std::set<FLAGS> _flags;
        
        // Structure related stats
        ranks _buildable; // the Unit to build
        std::list<std::weak_ptr<Entity>> _armyRegistry;
};

    
#endif
