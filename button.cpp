#include "olcPixelGameEngine.h"

#include <string>
#include <functional>

#include "factions.h"
#include "button.h"

Button::Button(){};

Button::Button(std::function<void()> function) : _executable(function){

}

Button::~Button(){}

void Button::onClick(){
    _executable();
}

Button& Button::setText(std::string text){
    _text = text;
    return *this;
}

std::string Button::getText(){
    return _text;
}

Button& Button::setLocation(olc::vi2d location, olc::vi2d size){
    _tl = location;
    _br = size;
    return *this;
}

Button& Button::setLocation(int x, int y, int w, int h){
    setLocation(olc::vi2d(x,y), olc::vi2d(w,h) );
    return *this;
}

olc::vi2d Button::getTopLeft(){
    return _tl;
}

olc::vi2d Button::getArea(){
    return _br;
}

bool Button::isInButton(olc::vf2d location){
    olc::vf2d offset = location - _tl;
    
    // check if location is outside bounds as that is more likely to fail quickly. 
    // only when all 4 checks pass is mouse inside button 
    if(offset.x < 0 || offset.y < 0 || offset.x > _br.x  || offset.y > _br.y)
        return false;
    return true;
}

/// Controller Class
Controller::Controller(std::shared_ptr<Faction> faction) : _faction(faction) {
    
}
Controller::~Controller(){
    _faction.reset();
}

Player::Player(std::shared_ptr<Faction> faction, olc::vi2d tlControl, olc::vi2d brControl, olc::PixelGameEngine* game) : 
                Controller(faction), _tl(tlControl),_br(brControl), _game(game){
    _uiImage.Create(_br.x,_br.y);
}

void Player::initialize(){
    _faction->makeBase( 4,CommandCenter,Gatherer);
}

void Player::setup(){
    // set draw target to be interface layer
    olc::Sprite *originalDrawTarget = _game->GetDrawTarget();
    _game->SetDrawTarget(_uiImage.Sprite());
    _game->FillRect({0,0},_uiImage.Sprite()->Size(),olc::DARK_BLUE );
    // setup 3x3 grid for buildings to fill
    int space = 5;
    int size = ( _br.y / 3);
    olc::vi2d anchor = _tl;
    for(int i = 0;i < 9; i++){
        int x1 = ( (i % 3) * size) + space;
        int y1 = ( (i / 3) * size) + space;
        olc::vi2d buttonTL = olc::vi2d(x1,y1) + _tl;
        int w1 = size - (space*2);
        int h1 = size - (space*2);
        _buttons.push_back(Button([this,i](){_activeMenu = i;_menuReady = false; } ) );
        _buttons[i].setLocation(buttonTL,olc::vi2d(w1,h1) )
        .setText(std::to_string(i) );
    }
    _game->DrawRect(olc::vi2d(0,0),olc::vi2d(_br.y-1,_br.y-1),olc::DARK_CYAN);
    olc::Pixel border = olc::GREEN;
    olc::Pixel fill = olc::BLACK;
    for(auto button : _buttons){
        _game->FillRect(button.getTopLeft(), button.getArea(),fill);
        _game->DrawRect(button.getTopLeft(), button.getArea(),border);
    }
    // setup building specific menu based on active structure
    std::shared_ptr<Entity> basePtr = _faction->getBase(_activeMenu);
    if(basePtr == nullptr){
        // base constuction list
        
    _game->SetDrawTarget(originalDrawTarget);
    _uiImage.Decal()->Update();
        return;
    }
    // else use basePtr to determine buttons to generate here
    // return draw target to original pointer
    _game->SetDrawTarget(originalDrawTarget);
    _uiImage.Decal()->Update();
}

void Player::update(){
    if(!_menuReady)
        setup();
    // get player input and preform assigned actions
    if(_game->GetMouse(0).bReleased){
        for(auto button : _buttons){
            if(button.isInButton(_game->GetMousePos() ) )
                button.onClick();
        }
    }
}

void Player::draw(){
    // draw the interface for the buttons
    olc::Pixel background = olc::BLUE;
    olc::Pixel border = olc::GREEN;
    olc::Pixel textColour = olc::WHITE;
    olc::Pixel fill = olc::BLACK;
    olc::Pixel highlight = olc::DARK_YELLOW;
    _game->FillRectDecal(_tl,_br,background);
    _game->DrawDecal(_tl,_uiImage.Decal() );
    for(auto button : _buttons){
        if(button.isInButton(_game->GetMousePos() ) )
            _game->FillRectDecal(button.getTopLeft(), button.getArea(),highlight);
        std::string lable = button.getText();
        int lableWidth = lable.length() * 8;
        if (lableWidth == 0)
            continue; // empty string dodge div by 0
        int lableHeight = 8;
        float widthScale = button.getArea().x / lableWidth;
        float heightScale = button.getArea().y / lableHeight;
        float scale = (widthScale < heightScale) ? widthScale : heightScale;
        olc::vi2d stringStart = olc::vi2d(
            (button.getArea().x - lableWidth * scale) * 0.5f ,
            (button.getArea().y - lableHeight * scale) * 0.5f );
        
        
        _game->DrawStringDecal( button.getTopLeft() + stringStart,button.getText(),textColour,olc::vf2d(scale,scale) );
    }
    
}

Computer::Computer(std::shared_ptr<Faction> faction){
    _faction = faction;
}

void Computer::initialize(){
    // create inital conditions of play
    _faction->makeBase(4,CommandCenter,Gatherer);
    _faction->makeBase(1,Barracks,Marine);
}

void Computer::update(){
    // some auto play code for the computer player to perform
    
    // items like increasing stats on regular tick counts
    
}
