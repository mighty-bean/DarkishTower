#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <array>
#include <vector>

enum Button {
    Up,
    Down,
    Select,
    COUNT
};

const static std::array<int,3> Pressed= {1, 1<<1, 1<<2};

struct ButtonState {
    int mPin;
    int mLastReading;
    int mNewReadingCounter;
    bool mPressDetected;
};

struct InputState {
    int state;
    bool isPressed(Button button) {
        return (state & Pressed[button]) != 0;  
    }
    bool isAnyPressed() {
        return (state != 0);  
    }
};

struct InputManager {
    std::array<ButtonState, Button::COUNT> mButton;
    int mCountRequired;

    void setup(int upPin, int downPin, int selectPin, int countRequired) {
        mCountRequired = std::max(countRequired, 1);
        mCountRequired = std::min(countRequired, 254);

        mButton[Button::Up].mPin = upPin;  
        mButton[Button::Down].mPin = downPin;
        mButton[Button::Select].mPin = selectPin;  

        mButton[Button::Up].mLastReading = HIGH;  
        mButton[Button::Down].mLastReading = HIGH;
        mButton[Button::Select].mLastReading = HIGH;  

        mButton[Button::Up].mNewReadingCounter = 0;  
        mButton[Button::Down].mNewReadingCounter = 0;
        mButton[Button::Select].mNewReadingCounter = 0;  

        mButton[Button::Up].mPressDetected = false;  
        mButton[Button::Down].mPressDetected = false;
        mButton[Button::Select].mPressDetected = false;  

        for(int i=0; i<mButton.size(); ++i) {
            pinMode(mButton[i].mPin, INPUT_PULLUP);         
        }
        for(int i=0; i<mButton.size(); ++i) {
            mButton[i].mLastReading = digitalRead(mButton[i].mPin);     
        }
    };


    void clear() {
        for(int i=0; i<mButton.size(); ++i) {
            mButton[i].mNewReadingCounter = 0; 
            mButton[i].mPressDetected = false;          
        }
    };

    void update() {
         for(int i=0; i<mButton.size(); ++i) {
            ButtonState& button = mButton[i];
            int Reading = digitalRead(button.mPin); 
            if (Reading == button.mLastReading) {
                button.mNewReadingCounter = 0;  
            }
            else {
                if (button.mNewReadingCounter < 255) {
                    ++button.mNewReadingCounter;  
                } 
                if (button.mNewReadingCounter > mCountRequired) {
                    button.mLastReading = Reading; 
                    button.mNewReadingCounter = 0;  
                    button.mPressDetected = (Reading == LOW); 
                }
            }    
        }       
    }

    InputState ReadInputState() {
        InputState input;
        input.state= 0;
        for(int i=0; i<mButton.size(); ++i) {
            if (mButton[i].mPressDetected) {
                input.state |= Pressed[i];
                mButton[i].mPressDetected = false; // consume the press
            }
        }  

        return input;  
    }
};

#endif

