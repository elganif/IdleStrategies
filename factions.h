#ifndef FACTIONS
#define FACTIONS
class Entity;
class Button;

enum ranks{
    CommandCenter,
    Marine,
    Scout,
    Tank,
    COUNT_OF_RANKS,
};

enum statIndex{
    typeName,
    initalHP,
    size,
    moveSpeed,
    actionSpeed, // unit attack speed or structure build speed
    actionEffect,// unit damage, or sturcture build cap
};

struct statBlock{
    std::string typeName;
    int initalHP;
    int size;
    int moveSpeed;
    int actionSpeed;
    int actionEffect;
};


class Faction : public std::enable_shared_from_this<Faction> {
    public:
        Faction(olc::vi2d location, olc::Pixel colour);
        ~Faction();
        int id();
        olc::Pixel colour();
        bool operator==(const Faction &other) const;
        
        std::shared_ptr<Entity> makeBase(ranks rank, ranks unit);
        std::shared_ptr<Entity> getBase(int index);
        
        int checkStat(ranks rank, statIndex stat);
        std::string getName(ranks rank);
        void improveStat(ranks rank, statIndex stat, int amount = 1);
        
    private:
        static int factionCounter;
        
        olc::vi2d _location;
        const int _id;
        olc::Pixel _colour;
        
        static const statBlock stats[COUNT_OF_RANKS]; //core stats of units. Does not change
        statBlock upgrades[COUNT_OF_RANKS]; // upgrades to units belonging to this faction
        
        std::vector<std::shared_ptr<Entity>> structures;
        std::vector<statBlock> buildingUpgrades; // upgrades to constructed buildings
        std::vector<Button> buttons;
};


#endif
