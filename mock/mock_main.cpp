#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

// Minimal host-side stubs that mimic ScreenLayout and managers enough to exercise flow
struct TextLine {
    std::string mText;
    int mValue;
};
struct ScreenLayout {
    std::string mTitle;
    const void* mBitmap;
    std::vector<TextLine> mTextLines;
    std::vector<TextLine> mOptions;
    int mSelection = 0;
};

class DisplayManagerMock {
public:
    void setup() {
        std::cout << "[Display] setup() -> Booting\n";
        ScreenLayout s;
        s.mTitle = "Booting";
        setDesiredLayout(s);
    }
    void setDesiredLayout(const ScreenLayout& layout) {
        mDesired = layout;
        repaint();
    }
    void repaint() { printLayout(mDesired); }
    void update() { /* no-op for mock */ }
    void setSelection(int idx) { mDesired.mSelection = idx; }
    int getSelection() { return mDesired.mSelection; }
    int getOptionCount() { return (int)mDesired.mOptions.size(); }
    ScreenLayout mDesired;
private:
    void printLayout(const ScreenLayout& s) {
        std::cout << "\n--- DISPLAY ---\n";
        std::cout << "Title: " << s.mTitle << "\n";
        for (auto &t : s.mTextLines) {
            std::cout << "  " << t.mText << "\n";
        }
        if (!s.mOptions.empty()) {
            std::cout << "Options:\n";
            for (size_t i=0;i<s.mOptions.size();++i) {
                std::cout << (i==s.mSelection?"> ":"  ") << s.mOptions[i].mText << "\n";
            }
        }
        std::cout << "---------------\n";
    }
};

class SoundManagerMock {
public:
    void setup(int pin) { (void)pin; std::cout << "[Sound] setup() pin=" << pin << "\n"; }
    void play(const std::string &name, bool waitForSong) {
        std::cout << "[Sound] play(" << name << ", wait=" << waitForSong << ")\n";
        if (waitForSong) {
            // simulate blocking-ish play by sleeping in a separate thread
            std::thread t([name](){
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                std::cout << "[Sound] finished: " << name << "\n";
            });
            t.detach();
        }
    }
    bool isPlaying() { return false; }
    bool isWaitingForSong() { return false; }
};

int main(){
    DisplayManagerMock display;
    SoundManagerMock sound;

    display.setup();
    sound.setup(0);

    // Simulate a simple screen flow similar to game_screens: Boot -> Title -> Menu
    ScreenLayout menu;
    menu.mTitle = "MAIN MENU";
    menu.mTextLines.push_back({"Welcome to DarkishTower", 0});
    menu.mOptions.push_back({"Start Game", 0});
    menu.mOptions.push_back({"Options", 0});
    menu.mOptions.push_back({"Exit", 0});
    menu.mSelection = 0;

    display.setDesiredLayout(menu);

    // emulate user pressing down, down, select
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    display.setSelection(1);
    display.repaint();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    display.setSelection(2);
    display.repaint();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // play a beep
    sound.play("beep_snd", false);

    // simulate selecting "Exit"
    if (display.getOptionCount() > 0) {
        std::cout << "User selected: " << menu.mOptions[display.getSelection()].mText << "\n";
    }

    // keep process alive briefly to allow detached sound thread to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Mock run complete.\n";
    return 0;
}
