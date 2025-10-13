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

}

void Player::draw(){
    // draw the interface for the buttons
    olc::Pixel background = olc::BLUE;
    olc::Pixel border = olc::GREEN;
    olc::Pixel fill = olc::DARK_BLUE;
    olc::Pixel highlight = olc::DARK_YELLOW;
    _game->FillRectDecal(_tl,_br,background);
    for(auto button : _buttons){
        _game->FillRectDecal(button.getTopLeft(), button.getArea(),button.isInButton(_game->GetMousePos()) ? highlight : fill);
        _game->DrawRectDecal(button.getTopLeft(), button.getArea(),border);
        olc::vi2d center = (button.getArea() * 0.5f) + button.getTopLeft();
        float charWidth = float(button.getArea().x) / (button.getText().size() + 2);
        olc::vi2d stringStart = olc::vi2d(button.getTopLeft().x + charWidth, center.y+(charWidth*0.5f));
        float stringScale = charWidth / 8.0f;
        _game->DrawStringDecal( stringStart,button.getText(),olc::BLACK,olc::vf2d(stringScale,stringScale));
    }
}

void Player::setup(){
    auto attackUp = [faction = _faction](){faction->improveStat(Marine,actionEffect);};
    auto unitsUp = [faction = _faction](){faction->improveStat(CommandCenter,actionEffect); };  
    _buttons.push_back(Button(attackUp).setText("Attack Increase") );
    _buttons.push_back(Button(unitsUp).setText("Add Unit") );
    olc::vi2d buttonLocation = _tl + olc::vi2d(20,20);
    olc::vi2d buttonSize = olc::vi2d(100,100);
    olc::vi2d buttonSpaceing = olc::vi2d(120,0);
    for(int i = 0;i < _buttons.size();i++){
        _buttons[i].setLocation(buttonLocation,buttonSize);
        buttonLocation += buttonSpaceing;
    }
}

void Player::update(int tickCount){
    // get player input and preform assigned actions
    if(_game->GetMouse(0).bReleased){
        for(auto button : _buttons){
            if(button.isInButton(_game->GetMousePos()))
                button.onClick();
        }
    }
}

void Computer::setup(){
    
    
}

void Computer::update(int tickCount){
    // some auto play code for the computer player to perform
    
    // items like increasing stats on regular tick counts
    
}
