#ifndef ENGINE_COMPONENTS
#define ENGINE_COMPONENTS
    
class in_queue
{
public:
    in_queue() {};
    virtual ~in_queue(){};
    virtual int update() = 0;
    virtual bool can_queue() = 0;
    int nextTurn;
};

struct turnPriorityCompare {
    public:
        bool operator()(std::weak_ptr<in_queue> a, std::weak_ptr<in_queue> b) {
            std::shared_ptr<in_queue> a_ptr;
            std::shared_ptr<in_queue> b_ptr;
            if(a_ptr = a.lock() ){
                if(b_ptr = b.lock() ){
                    return (a_ptr->nextTurn > b_ptr->nextTurn);
                }
            }
            return true;
        }
};

class TurnQueue {
    public:
    static TurnQueue queue;
    TurnQueue();
    void addItem(std::shared_ptr<in_queue> newItem);
    
    void runTurn();
    
    
    private:
    int turnCount = 0;
    std::priority_queue<std::weak_ptr<in_queue>,std::deque<std::weak_ptr<in_queue>>,turnPriorityCompare> turnOrder;
    
};

#endif
