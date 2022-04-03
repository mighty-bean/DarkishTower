#include <Arduino.h>
#include "game_state.h"
#include "game_screens.h"

GameState gGameState;

void GameState::setup() {
    inputManager.setup(1,3,7,5);
    soundManager.setup(DAC1);
    displayManager.setup();
    
    reset();
}  

void GameState::reset() {
    worldState.setup();

    mActiveScreens.clear();
    mConfirmScreen = nullptr;
    mScreenStartTime = 0;

    setActiveScreen(getStartupScreen());
}  

void GameState::update() {
    displayManager.update();   
    soundManager.update();
    inputManager.update();
    InputState inputs= inputManager.ReadInputState();

    GameScreen* activeScreen = getActiveScreen();

    const int32_t elapsedTime = millis() - mScreenStartTime;
    if (mConfirmScreen)
    {
        mConfirmScreen->update(elapsedTime);
    }
    else if (activeScreen) {
        activeScreen->update(elapsedTime);
    }

    if (inputs.isPressed(Button::Up))
    {
        playBeep(); 

        displayManager.setSelection(displayManager.getSelection() - 1);
        if (mConfirmScreen)
        {
            mConfirmScreen->onOptionChanged();
        }
        else if (activeScreen) {
            activeScreen->onOptionChanged();
        }
    }
    else if (inputs.isPressed(Button::Down))
    {
        playBeep();     
        
        displayManager.setSelection(displayManager.getSelection() + 1);
        if (mConfirmScreen)
        {
            mConfirmScreen->onOptionChanged();
        }
        else if (activeScreen) {
            activeScreen->onOptionChanged();
        }
    }
    else if (inputs.isPressed(Button::Select))
    {
        if (soundManager.isWaitingForSong()) {
            // ignore the input;
            return;
        }

        if (displayManager.getOptionCount() == 0)
        {
            return;
        }

        playBeep(); 
/*
        if (mConfirmScreen)
        {
            mConfirmScreen->onSelection();
        }
        else if (activeScreen) {
            activeScreen->onSelection();
        }
*/
        if (mConfirmScreen && activeScreen) {
            mConfirmScreen = nullptr;
            
            if (displayManager.getSelection() == 0) {
                activeScreen->confirm();   
            }
            else {
                activeScreen->deny();   
            }
        }
        else if (activeScreen) {
            activeScreen->onSelection();
        }
    }
}

void GameState::setActiveScreen(GameScreen* screen) {
    swapScreen(screen);
} 

void GameState::confirmOrDeny(const String& title, const String& info) {
    mConfirmScreen = setupConfirmOrDenyScreen(title, info);
    mScreenStartTime = millis();
    mConfirmScreen->begin();
}

GameScreen* GameState::getActiveScreen() const {
    return mActiveScreens.size() > 0 ? mActiveScreens.back() : nullptr;
}

void GameState::internalStartScreen(GameScreen* screen) {
    if (screen) {
        mConfirmScreen = nullptr;
        mScreenStartTime = millis();
        screen->begin();
    }
}

void GameState::swapScreen(GameScreen* screen) {
    if (screen) {
        if (mActiveScreens.size() > 0) {
            mActiveScreens.pop_back();
        }
        mActiveScreens.push_back(screen);
        internalStartScreen(screen);
    }
}

void GameState::pushScreen(GameScreen* screen) {
    if (screen) {
        mActiveScreens.push_back(screen);
        internalStartScreen(screen);
    }
}

void GameState::popScreen() {
    if (mActiveScreens.size() > 0) {
        mActiveScreens.pop_back();
    }
    if (mActiveScreens.size() > 0) {
        GameScreen* screen = mActiveScreens.back();    
        internalStartScreen(screen);
    }
}


