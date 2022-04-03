#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "sound_manager.h"
#include "input_manager.h"
#include "world_state.h"
#include "display_manager.h"
#include <vector>

struct GameScreen;

struct GameState {
    InputManager inputManager;
    SoundManager soundManager;
    DisplayManager displayManager;
    WorldState worldState; 

    std::vector<GameScreen*> mActiveScreens;
    GameScreen* mConfirmScreen = nullptr;
    int32_t mScreenStartTime = 0;

    void setup();
    void reset();
    
    void update();
    void setActiveScreen(GameScreen* screen);
    void confirmOrDeny(const String& title, const String& info);

    GameScreen* getActiveScreen() const;
    void swapScreen(GameScreen* screen);
    void pushScreen(GameScreen* screen);
    void popScreen();

    GameState() {
        mActiveScreens.reserve(4);  
    }

    private:
    void internalStartScreen(GameScreen* screen);
};

extern GameState gGameState;

#endif
