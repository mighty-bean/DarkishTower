#ifndef GAME_SCREENS_H
#define GAME_SCREENS_H

struct GameState;

struct GameScreen {
    virtual void begin() {}
    virtual void update(int32_t elapsedMS) {}
    virtual void onOptionChanged() {}
    virtual void onSelection() {}
    virtual void confirm() {}
    virtual void deny() {}
};

GameScreen* getStartupScreen();
GameScreen* getConfirmOrDenyScreen();
GameScreen* setupConfirmOrDenyScreen(const String& title, const String& info);

void playBeep();
void playErrorSound();

#endif