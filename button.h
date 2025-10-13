#ifndef BUTTON_DEFINED
#define BUTTON_DEFINED
class Faction;

class Button{
    public:
    Button();
    Button(std::function<void()> function);
    ~Button();
    
    void onClick();
    
    // setters
    Button& setText(std::string text);
    std::string getText();
    Button& setLocation(olc::vi2d location, olc::vi2d size);
    Button& setLocation(int x,int y, int w, int h);
    olc::vi2d getTopLeft();
    olc::vi2d getArea();
    
    bool isInButton(olc::vf2d location);
    
    private:
    std::function<void()> _executable;
    std::string _text;
    olc::vi2d _tl;
    olc::vi2d _br;
};

/// Controller Class. 
class Controller{
    public:
    Controller():_faction(nullptr){};
    Controller(std::shared_ptr<Faction> faction);
    virtual ~Controller();
    
    virtual void setup() = 0;
    virtual void update(int tickCounter) = 0;
    
    
    protected:
    std::shared_ptr<Faction> _faction;
};

class Player : public Controller {
    public:
    Player() = default;
    
    Player(std::shared_ptr<Faction> faction,olc::vi2d tlControl, olc::vi2d brControl, olc::PixelGameEngine* game);
    void draw();
    void setup() override;
    void update(int tickCounter) override;
    private:
    olc::PixelGameEngine *_game;
    olc::vi2d _tl;
    olc::vi2d _br;
    std::vector<Button> _buttons;
};

class Computer : public Controller {
    public:
    Computer() = default;
    void setup() override;
    void update(int tickCounter) override;
    
};
#endif
