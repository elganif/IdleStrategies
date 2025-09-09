enum Ranks{
    Base,
    Marine,
    Scout,
    Tank,
    COUNT_OF_RANKS,
};
struct StatBlock{
    string Name
    int initalHP;
    int size;
    int walkSpeed;
    int attackTime;
    int damage;
};
Unit::StatBlock[COUNT_OF_CLASSES] stat = 
    {
    {"Base", 1000, 0, 1,1,0},
    {"Marine",100,1,10,7,10},
    {"Scout",75,1,6,10,5}
    };
    
    
    
    
    
    };
// add to unit 
Ranks rank;
// to access 
//stat[rank].size

  // add rank to constructor arguments.
