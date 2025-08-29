#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include "idlestrategy.h"

#include <queue>
#include <vector>

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
    
    //map stuff
    std::shared_ptr<Map> mapData;
    
    //unit stuff
    //std::vector<std::weak_ptr<Unit>> army;
    class MyCompare {
        public:
            bool operator()(std::shared_ptr<Unit> a, std::shared_ptr<Unit> b) {
                return (*a) < (*b);
            }
    };
    std::priority_queue<std::shared_ptr<Unit>,std::vector<std::shared_ptr<Unit>>,MyCompare> turnOrder;
    Base home;
    Base enemy;

    // Physics operations and ingame time
    void tick(){
        
        std::vector<std::weak_ptr<Unit>> navy;
        
        //update on home and enemy bases
        home.update(navy);
        enemy.update(navy);
        
        // update any units whos turn has come up
        bool stillthisturn = true;
        while(!turnOrder.empty() && stillthisturn)
        {
            std::shared_ptr<Unit> unit = turnOrder.top();
            if(unit->nextTurn <= turnCount)
            {
                int waitTime = unit->update();
                unit->nextTurn = turnCount + waitTime;
                if(unit->getHP() > 0)
                    turnOrder.push(unit);
                turnOrder.pop();
            } else {
                stillthisturn = false;
            }
        }
        
        // add all new units to the turnTracker
        for(auto soldier = navy.begin(); soldier != navy.end();soldier++)
        {
            if(std::shared_ptr<Unit> unit = soldier->lock())
            {
                unit->nextTurn = turnCount + 1;
                turnOrder.push(unit);
            }
        }
    }
    
    //rendering the scene and play area
    void draw(){
        int panewidth = resolutionY / 5;

        // Game area terrain background
        tv.DrawDecal({0,0},mapData->terrainD,{10,10});
        
        //draw all units from the mapData
        for(auto const &[local,in_map] : mapData->unitLocations)
            for(auto const &[faction,count] : in_map)
            {
                tv.FillRectDecal(local,olc::vi2d(1,1),mapData->factionColours[faction]);
        }
        
        //draw vertical and horizontal lines for the grid overlay
        for(int y = 0;y <= mapData->br.y;y++)
        {
            tv.DrawLineDecal(olc::vi2d(0,y),olc::vi2d(mapData->br.x,y),olc::BLUE);
        }
        for(int x = 0;x <= mapData->br.x;x++)
        {
            tv.DrawLineDecal(olc::vi2d(x,0),olc::vi2d(x,mapData->br.y),olc::BLUE);
        }
        
        // draw home bases
        tv.FillRectDecal(home.location(),olc::vi2d(2,2),home.getFactionColour());
        
        tv.FillRectDecal(enemy.location(),olc::vi2d(2,2),enemy.getFactionColour());
        
            
        
        
        //Future control Panel area reservation
        olc::vi2d tlControl = olc::vi2d(0,resolutionY - panewidth);
        olc::vi2d brControl = olc::vi2d(resolutionX - panewidth,panewidth);
        FillRectDecal(tlControl,brControl,olc::DARK_GREEN);

        //Future infromation Display Panel reservation
        olc::vi2d tlInfo = olc::vi2d(resolutionX - panewidth,0);
        olc::vi2d brInfo = olc::vi2d(panewidth,resolutionY);
        FillRectDecal(tlInfo,brInfo,olc::DARK_YELLOW);
        return;
    }
public:
    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        mapData = std::make_shared<Map>(170,85);
        Unit::setMap(mapData);
        tv.Initialise(GetScreenSize(),olc::vi2d(10,10));
        
        home = Base(olc::vi2d(20,20),1);
        mapData->factionColours[1] = olc::BLUE;
        enemy = Base(olc::vi2d(150,65),2);
        mapData->factionColours[2] = olc::RED;
        
        home.setOpponent(enemy.location());
        enemy.setOpponent(home.location());
        
        return true;
    }
    
    double curTime = 0.0f;
    bool OnUserUpdate(float fElapsedTime) override
    {
        // called once per frame
        curTime += fElapsedTime * turnsPerSecond;
        // TODO:: build UI and input function
        if(GetKey(olc::Key::ESCAPE).bReleased)
        {
            return false;
        }
        tv.HandlePanAndZoom();
        
        // run game tick when appropriate
        if (curTime > 1.0f)
        {
            tick();
            curTime -= 1.0f;
            turnCount++;
        }
        
        //render
        draw();

        

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
