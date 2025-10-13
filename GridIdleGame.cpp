
#include "olcPixelGameEngine.h"

#include "olcPGEX_TransformedView.h"

#include <queue>
#include <vector>
#include <set>

#include "factions.h"
#include "entity.h"
#include "maping.h"
#include "button.h"


class IdleGame : public olc::PixelGameEngine
{
public:
    int resolutionX = 1920;
    int resolutionY = 1080;
    IdleGame()
    {
        sAppName = "Idle Strategy";
    }
    
private:
    
    const int turnsPerSecond = 30;
    int turnCount = 0;
    olc::TransformedView tv;
    
    struct turnPriorityCompare {
        public:
            bool operator()(std::weak_ptr<Entity> a, std::weak_ptr<Entity> b) {
                std::shared_ptr<Entity> a_ptr;
                std::shared_ptr<Entity> b_ptr;
                if(a_ptr = a.lock()){
                    if(b_ptr = b.lock()){
                        return (a_ptr->nextTurn > b_ptr->nextTurn);
                    }
                    return true;
                }
                return false;
            }
    };
    std::priority_queue<std::weak_ptr<Entity>,std::vector<std::weak_ptr<Entity>>,turnPriorityCompare> turnOrder;
    std::vector<std::shared_ptr<Faction>> factions;
     //playerFaction;
    //std::shared_ptr<Faction> npcFaction;
    
    Player playerControls;
    Computer aiControls;


    // Physics operations and ingame time
    void tick(){
        
        // update any units whos turn has come up
        std::vector<std::shared_ptr<Entity>> unitsToTakeTurn;
        while(!turnOrder.empty()){
            if(std::shared_ptr<Entity> queueTop = turnOrder.top().lock()){
                if(queueTop->nextTurn > turnCount)
                    break; // out of while loop, next unit in queue is waiting for a future turn
                
                unitsToTakeTurn.push_back(queueTop);
            }
            turnOrder.pop(); //lock failed or turn was taken
        }
        
        for(auto &ptr : unitsToTakeTurn){
            int wait = ptr->update(); // units whos turn was ready get to take thier turn
            // any units that are still alive requeue
            if(ptr->alive() && wait > 0){ // a zero or negitive wait time indicates not to requeue unit
                ptr->nextTurn = turnCount + wait;
                turnOrder.push(ptr);
            }
        }
        // add all new units to the turnTracker
        while(Entity::bootCamp->begin() != Entity::bootCamp->end()){
            std::shared_ptr<Entity> tmp = Entity::bootCamp->front();
            tmp->nextTurn = turnCount + 1; // set its first move to next turn
            turnOrder.push(tmp);
            Entity::bootCamp->pop_front();
            
        }
        // regenerate pathfinding map each turn
        Entity::mapData->generateVectorMap();
    }
    
    
    enum UI{
        MAP,
        CONTROL,
        INFO,
        UNKNOWN
    };
    
    //rendering the scene and play area
    int panewidth = resolutionY / 5;
    olc::vi2d tlMapView = olc::vi2d(0,0);
    olc::vi2d brMapView = olc::vi2d(resolutionX - panewidth,resolutionY - panewidth);
    olc::vi2d tlControl = olc::vi2d(0,resolutionY - panewidth);
    olc::vi2d brControl = olc::vi2d(resolutionX - panewidth,panewidth);
    olc::vi2d tlInfo = olc::vi2d(resolutionX - panewidth,0);
    olc::vi2d brInfo = olc::vi2d(panewidth,resolutionY);
    
    olc::vi2d mapMouse;
    bool focused = false;
    olc::vi2d mapFocus;
    UI mouseLoc = UNKNOWN;
    Button attackUpgrade;
    Button armyUpgrade;
    
    /// Take and process input from player
    void userInput(){
        mouseLoc = UNKNOWN;
        //define Map view display area
        if (GetMousePos() == GetMousePos().clamp(tlMapView,brMapView) ){
            mouseLoc = MAP;
        }
        
        //Define control Panel display area
        if (GetMousePos() == GetMousePos().clamp(tlControl, tlControl + brControl) ){
            mouseLoc = CONTROL;
        }
        if (GetMousePos() == GetMousePos().clamp(tlInfo, tlInfo + brInfo) ){
            mouseLoc = INFO;
        }

        // Map Controls
        if (mouseLoc == MAP){
            tv.HandlePanAndZoom();
            mapMouse = tv.ScreenToWorld(GetMousePos());
            olc::vf2d tvViewerBR = tv.ScreenToWorld(brMapView);
            //clamp view to the gameworld
            // 0,0 in top left, to 170,84 in bottom right.
            olc::vf2d tlClamp = olc::vf2d(-0.5f,-0.5f);
            olc::vf2d brClamp = (Entity::mapData->br + olc::vf2d(0.5f,1.0f)) -(tv.ScreenToWorld(brMapView) - tv.GetWorldOffset()) ;
            olc::vf2d tvOffset = tv.GetWorldOffset().clamp(tlClamp,brClamp); //TODO :calculate zoom to backset ratio
            tv.SetWorldOffset(tvOffset);
            if(GetMouse(1).bReleased)
                focused = false;
            if(GetMouse(0).bReleased){
                focused = true;
                mapFocus = mapMouse;
            }
            
        }
        playerControls.update(turnCount);
        
    }
    
    void draw(){
        
        // Game area terrain background
        tv.DrawDecal({0,0},Entity::mapData->terrainD,{10.0f/Entity::mapData->graphicalDetail,10.0f/Entity::mapData->graphicalDetail});
        
        //draw all units from the mapData
        for(auto const &[local,cell] : Entity::mapData->unitLocations){
            tv.FillRectDecal(local+olc::vf2d(0.1f,0.1f),olc::vf2d(0.8,0.8),Entity::mapData->factionColours[cell.faction]);
        }
        
        for(auto const &[local,amount] : Entity::mapData->resourceNodes){
            tv.DrawStringDecal(local,"R",olc::MAGENTA);
        }
        
        //draw vertical and horizontal lines for the grid overlay
        for(int y = 0;y <= Entity::mapData->br.y;y++){
            tv.DrawLineDecal(olc::vi2d(0,y),olc::vi2d(Entity::mapData->br.x,y),olc::VERY_DARK_GREY);
        }
        for(int x = 0;x <= Entity::mapData->br.x;x++){
            tv.DrawLineDecal(olc::vi2d(x,0),olc::vi2d(x,Entity::mapData->br.y),olc::VERY_DARK_GREY);
        }
        
        // draw faction bases
        for( auto [faction,location] : Entity::mapData->baseLocations){
            tv.DrawRectDecal(location + olc::vi2d(-1,-1),olc::vi2d(3,3),Entity::mapData->factionColours[faction]);
        }
        
        
       // some test buttons
       playerControls.draw();

        
        
        //Draw info panel and info inputs
        FillRectDecal(tlInfo,brInfo,olc::DARK_GREEN);
        
        FillRectDecal(tlInfo + olc::vi2d(10,10), panewidth - olc::vi2d(20,20),olc::BLACK);
        DrawRectDecal(tlInfo + olc::vi2d(10,10), panewidth - olc::vi2d(20,20),olc::GREEN);
        olc::vi2d line = tlInfo + olc::vi2d(12,12);
        olc::vi2d dataCell = mapMouse;
        if(focused)
            dataCell = mapFocus;
        
        if( (focused || mouseLoc == MAP) && Entity::mapData->unitLocations.find(dataCell) != Entity::mapData->unitLocations.end()){
            DrawStringDecal( line += olc::vi2d(0,10) ,"Units",olc::WHITE);
            for(auto unit : Entity::mapData->unitLocations[dataCell].units){
                    DrawStringDecal(line += olc::vi2d(0,10)
                      , unit->getName() + " " + std::to_string(unit->getHP()) + " " + std::to_string(unit->checkStat(actionEffect))
                      , unit->factionColour());
            }
        }
        
        tv.DrawRectDecal(mapMouse,{1,1},olc::CYAN);
        if(focused)
            tv.DrawRectDecal(mapFocus,{1,1},olc::DARK_YELLOW);
        
        /// debug test display
        //olc::vi2d debugText = olc::vi2d(10,10);
        //olc::vi2d debugIncrement = olc::vi2d (0,10);
        //DrawStringDecal(debugText += debugIncrement,tv.GetWorldOffset().str(),olc::CYAN);
        //DrawStringDecal(debugText += debugIncrement,tv.GetWorldVisibleArea().str(),olc::CYAN);
        //DrawStringDecal(debugText += debugIncrement,tv.GetWorldScale().str(),olc::CYAN);tv.ScreenToWorld(brMapView);
        //DrawStringDecal(debugText += debugIncrement,tv.ScreenToWorld(brMapView).str(),olc::CYAN);
        //olc::vf2d test = Entity::mapData->br -(tv.ScreenToWorld(brMapView) - tv.GetWorldOffset()) ;
        //DrawStringDecal(debugText += debugIncrement,test.str(),olc::CYAN);
        
        return;
    }
public:
    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        
        Entity::setMap(170,85);
        Entity::mapData->generateResources(10);
        Entity::bootCamp = std::make_shared<std::list<std::shared_ptr<Entity>>>();
        tv.Initialise(GetScreenSize(),olc::vi2d(10,10));
        tv.SetScaleExtents(olc::vi2d(10,10),olc::vi2d(100,100));
        tv.EnableScaleClamp(true);
        
        factions.emplace_back(std::make_shared<Faction>(Entity::mapData->br - olc::vi2d(20,20),olc::GREEN));
        playerControls = Player(factions[0], tlControl,brControl ,this);
        std::shared_ptr<Entity> home = factions[0]->makeBase(CommandCenter,Marine);
        home->nextTurn = turnCount + 1;
        turnOrder.push(home);
        playerControls.setup();
        
        factions.emplace_back( std::make_shared<Faction>(olc::vi2d(20,20),olc::RED));
        std::shared_ptr<Entity> enemy = factions[1]->makeBase(CommandCenter,Marine);
        enemy->nextTurn = turnCount + 1;
        turnOrder.push(enemy);
        
        
        return true;
    }
    
    double curTime = 0.0f;
    bool OnUserUpdate(float fElapsedTime) override
    {
        if(GetKey(olc::Key::ESCAPE).bReleased)
        { // close on escape key
            return false;
        }
        
        
        // Accumulate time and run a game tick when enough time has passed
        curTime += double(fElapsedTime) * turnsPerSecond;
        if (curTime > 1.0)
        {
            tick();
            curTime -= 1.0;
            turnCount++;
        }
        // TODO:: build UI and input function
        userInput();
        //render
        draw();
        
        return true;
    }
    
    /// clean up memory in correct order when game closes
    bool OnUserDestroy()
    {
        // reset shared pointers of factions.
        for(int i = 0; i < factions.size();i++)
            factions[i].reset();
        
        // reset shared_ptr of any units still in the bootCamp list
        for(auto &ptr : (*Entity::bootCamp)){
            ptr.reset();
        }
        Entity::bootCamp->clear();
        
        return true;
    }
};


int main()
{
    IdleGame game;
    if (game.Construct(game.resolutionX, game.resolutionY, 1, 1))
        game.Start();
    return 0;
}
