#include "arduino_mocks.h"
#include <iostream>
#include <thread>
#include <chrono>

// include the project's ScreenLayout header
#include "../src/screen_layout.h"

int main() {
    // Build a ScreenLayout using real types from src/screen_layout.h
    ScreenLayout s;
    s.mTitle = "MOCK: Real ScreenLayout";
    s.addInfo("This uses the real ScreenLayout type", 0);
    s.addOption("Start", 0);
    s.addOption("Exit", 0);
    s.mSelection = 0;

    // print it like DisplayManager would
    std::cout << "Title: " << s.mTitle << "\n";
    for (auto &t : s.mTextLines) {
        std::cout << "  Info: " << t.mText << "\n";
    }
    std::cout << "Options:\n";
    for (size_t i=0;i<s.mOptions.size();++i) {
        std::cout << (i==s.mSelection?"> ":"  ") << s.mOptions[i].mText << "\n";
    }

    // emulate a selection change
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    s.mSelection = 1;
    std::cout << "\nAfter changing selection:\n";
    for (size_t i=0;i<s.mOptions.size();++i) {
        std::cout << (i==s.mSelection?"> ":"  ") << s.mOptions[i].mText << "\n";
    }

    std::cout << "mock_full complete.\n";
    return 0;
}
