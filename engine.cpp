
#include "olcPixelGameEngine.h"

#include "olcPGEX_TransformedView.h"

#include <queue>
#include <set>

#include "engine.h"
#include "maping.h"
#include "factions.h"
#include "entity.h"



TurnQueue::TurnQueue(){}

void TurnQueue::addItem(std::shared_ptr<in_queue> newItem){
    newItem->nextTurn = turnCount + 1;
    turnOrder.push(newItem);
}

void TurnQueue::runTurn(){
    turnCount++;
    
    std::deque<std::shared_ptr<in_queue>> unitsToTakeTurn;
    while(!turnOrder.empty() ){
            if(std::shared_ptr<in_queue> queueTop = turnOrder.top().lock() ){
                if(queueTop->nextTurn > turnCount){
                    break;
                }
                unitsToTakeTurn.push_back(queueTop);
            }
            turnOrder.pop();
    }
    for(auto &ptr : unitsToTakeTurn){
        
        int wait = ptr->update(); // units whos turn was ready get to take thier turn
        // any units that are still alive requeue
        if(ptr->can_queue() && wait > 0){ // a zero or negitive wait time indicates not to requeue unit
            ptr->nextTurn = turnCount + wait;
            turnOrder.push(ptr);
        }
    }
}
TurnQueue TurnQueue::queue;

