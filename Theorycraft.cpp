enum Ranks{
    Base,
    Marine,
    Scout,
    Tank,
    COUNT_OF_RANKS,
};
struct StatBlock{
    int initalHP;
    int size;
    int walkSpeed;
};
StatBlock[COUNT_OF_CLASSES] stat;
// add to unit 
Ranks rank;
// to access 
stat[rank].size

  // add rank to constructor arguments.
