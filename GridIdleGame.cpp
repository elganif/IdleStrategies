
#include "olcPixelGameEngine.h"

#include "olcPGEX_TransformedView.h"

#include <queue>
#include <vector>
#include <set>

#include "engine.h"
#include "factions.h"
#include "maping.h"
#include "entity.h"
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
    olc::TransformedView tv;
    
    
    std::vector<std::shared_ptr<Faction>> factions;
    
    std::shared_ptr<Player> playerControls;

    /// Physics operations and ingame time
    void tick(){
        TurnQueue::queue.runTurn();
        
        Map::mapData->generateVectorMap();
    }
    
    
    /// UI Implementation
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
            mapMouse = tv.ScreenToWorld(GetMousePos() );
            olc::vf2d tvViewerBR = tv.ScreenToWorld(brMapView);
            //clamp view to the gameworld
            // 0,0 in top left, to 170,84 in bottom right.
            olc::vf2d tlClamp = olc::vf2d(-0.5f,-0.5f);
            olc::vf2d brClamp = (Map::mapData->br + olc::vf2d(0.5f,1.0f) ) -(tv.ScreenToWorld(brMapView) - tv.GetWorldOffset() ) ;
            olc::vf2d tvOffset = tv.GetWorldOffset().clamp(tlClamp,brClamp); //TODO :calculate zoom to backset ratio
            tv.SetWorldOffset(tvOffset);
            if(GetMouse(1).bReleased)
                focused = false;
            if(GetMouse(0).bReleased){
                focused = true;
                mapFocus = mapMouse;
            }
            
        }
        playerControls->update();
        
    }
    
    void draw(){
        
        // Game area terrain background
        tv.DrawDecal({0,0},Map::mapData->terrain.Decal(),{10.0f/Map::mapData->graphicalDetail,10.0f/Map::mapData->graphicalDetail});
        
        //draw all units from the mapData
        for(auto const &[local,cell] : Map::mapData->unitLocations){
            tv.FillRectDecal(local+olc::vf2d(0.1f,0.1f),olc::vf2d(0.8,0.8),Map::mapData->factionColours[cell.faction]);
        }
        
        for(auto const &[local,amount] : Map::mapData->resourceNodes){
            tv.DrawStringDecal(local,"R",olc::MAGENTA);
        }
        
        //draw vertical and horizontal lines for the grid overlay
        for(int y = 0;y <= Map::mapData->br.y;y++){
            tv.DrawLineDecal(olc::vi2d(0,y),olc::vi2d(Map::mapData->br.x,y),olc::VERY_DARK_GREY);
        }
        for(int x = 0;x <= Map::mapData->br.x;x++){
            tv.DrawLineDecal(olc::vi2d(x,0),olc::vi2d(x,Map::mapData->br.y),olc::VERY_DARK_GREY);
        }
        
        // draw faction bases
        for(auto faction : factions){
            tv.DrawDecal( (faction->location() - olc::vf2d(1,1) ),
                        faction->drawing(),
                        olc::vf2d(Map::mapData->graphicalDetail,Map::mapData->graphicalDetail) );
            tv.DrawRectDecal( (faction->location() - olc::vf2d(1,1) ),olc::vi2d(3,3),faction->colour() );
        }
        
        
        //Draw info panel and info inputs
        FillRectDecal(tlInfo,brInfo,olc::DARK_GREEN);
        
        FillRectDecal(tlInfo + olc::vi2d(10,10), panewidth - olc::vi2d(20,20),olc::BLACK);
        DrawRectDecal(tlInfo + olc::vi2d(10,10), panewidth - olc::vi2d(20,20),olc::GREEN);
        olc::vi2d line = tlInfo + olc::vi2d(12,12);
        olc::vi2d dataCell = mapMouse;
        if(focused)
            dataCell = mapFocus;
        
        if( (focused || mouseLoc == MAP) ){
            for(auto faction : factions){
                if(dataCell == faction->location() ){ //<- can be adapted to check specific square with faction?
                    DrawStringDecal(line += olc::vi2d(0,10),"Faction",faction->colour() );
                    DrawStringDecal(line += olc::vi2d(0,10),std::to_string(faction->resource() ),faction->colour() );
                }
            }
            if(Map::mapData->unitLocations.find(dataCell) != Map::mapData->unitLocations.end() ){
                DrawStringDecal( line += olc::vi2d(0,10) ,"Units",olc::WHITE);
                for(auto wptr : Map::mapData->unitLocations[dataCell].units){
                    if( std::shared_ptr<in_map> mUnit = wptr.lock() ){
                        std::shared_ptr<Entity> unit = std::dynamic_pointer_cast<Entity>(mUnit);
                            DrawStringDecal(line += olc::vi2d(0,10)
                              , unit->getName() + " " + std::to_string(unit->getHP() ) + " " + std::to_string(unit->checkStat(actionEffect) )
                              , unit->factionColour() );
                    }
                }
            }
            if(Map::mapData->resourceNodes.find(dataCell) != Map::mapData->resourceNodes.end() ){
                DrawStringDecal(line += olc::vi2d(0,10)
                          , "Resources " + std::to_string(Map::mapData->resourceNodes[dataCell]._turnReserve) + " "
                          , olc::WHITE );
            }
        }
        
        tv.DrawRectDecal(mapMouse,{1,1},olc::CYAN);
        if(focused)
            tv.DrawRectDecal(mapFocus,{1,1},olc::DARK_YELLOW);
        
        
        // some test buttons
        playerControls->draw();
        
        /// debug test display
        
        return;
    }
    
public:
    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        SetPixelMode(olc::Pixel::MASK);
        Map::createMap(170,85);
        Map::mapData->generateResources(10);
        tv.Initialise(GetScreenSize(),olc::vi2d(10,10) );
        tv.SetScaleExtents(olc::vi2d(10,10),olc::vi2d(100,100) );
        tv.EnableScaleClamp(true);
        
        factions.emplace_back(std::make_shared<Faction>(Map::mapData->br - olc::vi2d(20,20),olc::GREEN) );
        playerControls = factions[0]->setAsPlayer(tlControl,brControl,this);
        playerControls->initialize();
        
        factions.emplace_back( std::make_shared<Faction>(olc::vi2d(20,20),olc::RED) );
        factions[1]->setAsComputer();
        
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
        
        return true;
    }
};


int main()
{
    IdleGame game;
    if (game.Construct(game.resolutionX, game.resolutionY, 1, 1) )
        game.Start();
    return 0;
}
