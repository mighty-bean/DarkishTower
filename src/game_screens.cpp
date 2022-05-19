#include <Arduino.h>
#include "game_state.h"
#include "game_screens.h"
#include "assets/sounds.h"
#include "assets/images.h"
#include <deque>


void playBeep() {
    if (!gGameState.soundManager.isPlaying()) {
        gGameState.soundManager.play(beep_snd, false); 
    } 
}

void playErrorSound() {
    if (!gGameState.soundManager.isPlaying()) {
        gGameState.soundManager.play(beep_snd, false); 
    } 
}

struct ScreenList {
    std::deque<ScreenLayout> mScreenList;
    int32_t mStartTime;

    int count() {
        return mScreenList.size();
    }

    void addScreen(const ScreenLayout& screen) {
       mScreenList.push_back(screen); 
    }

    bool nextScreen() {
        mStartTime = millis();
        if (mScreenList.size()) {
            gGameState.displayManager.setDesiredLayout(mScreenList.front());
            mScreenList.pop_front();
            return true;
        }
        return false;
    }

    void clear() {
      mScreenList.clear();  
    }

    int32_t timeInScreen() {
        return millis() - mStartTime;
    }
};

const String& getKingdomName(int i) {
    static String Arisilon("Arisilon");
    static String Brynthia("Brynthia");
    static String Durnin("Durnin");
    static String Zenon("Zenon");
    static String UNKNOWN("UNKNOWN");
    switch(i) {
        case (int)Kingdom::Arisilon:
            return Arisilon;
            break;
        
        case (int)Kingdom::Brynthia:
            return Brynthia;
            break;
            
        case (int)Kingdom::Durnin:
            return Durnin;
            break;
            
        case (int)Kingdom::Zenon:
            return Zenon;
            break;
            
        default:
            break;
    };
    return UNKNOWN;
}

bool isValidKingdom(int i) {
    return i>=0 && i<(int)Kingdom::COUNT;
}

static String InventoryNames[(int)Inventory::COUNT] = {
    "Warriors",
    "Gold",
    "Food",
    "Beast",
    "Scout",
    "Healer",
    "Pegasus",
    "Sword",
    "BrassKey",
    "SilverKey",
    "GoldKey"
};

const String&  getInventoryName(int i) {
    static String UNKNOWN("UNKNOWN");
    if (i>=0 && i<(int)Inventory::COUNT)  {
        return InventoryNames[i];
    }   
    return UNKNOWN;
}

static const uint16_t* InventoryPictures[(int)Inventory::COUNT] = {
    tile_bitmap_warriors,
    tile_bitmap_gold,
    tile_bitmap_food,
    tile_bitmap_beast,
    tile_bitmap_scout,
    tile_bitmap_healer,
    tile_bitmap_pegasus,
    tile_bitmap_sword,
    tile_bitmap_brasskey,
    tile_bitmap_silverkey,
    tile_bitmap_goldkey
};

const uint16_t* getInventoryPicture(int i) {
    static String UNKNOWN("UNKNOWN");
    if (i>=0 && i<(int)Inventory::COUNT)  
    {
        return InventoryPictures[i];
    }  
    return nullptr; 
}

void playerStartTurn(int playerIndex);

//
// Base for screens that auto-confirm the selection
//
struct AutoConfirmScreen : public GameScreen {
    TextLine mSelectedOption;
    virtual void begin() override {};

    virtual void onSelection() override {
        mSelectedOption = gGameState.displayManager.getSelectedOption();
        gGameState.confirmOrDeny(gGameState.displayManager.getTitle(), mSelectedOption.mText);
    }

    virtual void confirm() override {};
    virtual void deny() override {};
};

//
// ************
// GAME SCREENS
// ************
//

struct GameOver : public GameScreen {
     virtual void begin() override {
 /*
        if (gGameState.worldState.mPlayers.size() > 1) {
            // impossible!
            gGameState.popScreen();
        }
*/
        gGameState.inputManager.clear();
        gGameState.soundManager.play(plague_snd, true);
 
        gGameState.displayManager.setDesiredLayout({
            "GAME OVER", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"You are out", ST77XX_RED},
                {"of Warriors", ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "Play Again", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        
    }

    virtual void onSelection() override {
        gGameState.reset();
        return;
    }

} gGameOver;

struct Victory : public GameScreen {
     virtual void begin() override {

        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        String Title = "PLAYER ";
        Title += String(playerIdx+1);

        gGameState.inputManager.clear();
        gGameState.soundManager.play(darktower_snd, true);
 
        gGameState.displayManager.setDesiredLayout({
            Title, // title
            tile_bitmap_victory, //const uint16_t* bitmap ; 
            {
                {"WINNER!", ST77XX_GREEN}
            }, //std::vector<TextLine> textLines;
            {
                { "Play Again", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        
    }

    virtual void onSelection() override {
        gGameState.reset();
        return;
    }

} gVictory;

bool checkForEndGame() {
    if (gGameState.worldState.mPlayers.size() == 1) {
        int player= gGameState.worldState.mCurrentPlayer;
        if (gGameState.worldState.mPlayers[player].mInventory[(int)Inventory::Warriors] <= 0) {
            gGameState.pushScreen(&gGameOver);
            return true;
        }
    }
    return false;
}

struct PlayerCursed : public GameScreen {
    ScreenList mScreenList;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        gGameState.soundManager.play(plague_snd, true);

        String cursedWarriors = "1 Warrior";
        if (player.mCursedWarriors != 1) {
            cursedWarriors = String(player.mCursedWarriors) + String(" Warriors");           
        }
        String cursedGold = String(player.mCursedGold) + String(" Gold");

        player.mTurnCompleted = true;
        player.ClearCurse();

        String Title = "PLAYER ";
        Title += String(playerIdx+1);

        mScreenList.addScreen({
            Title, // title
            tile_bitmap_cursed, //const uint16_t* bitmap ; 
            {
                {"A Wizards Curse", ST77XX_RED}, 
                {"takes effect!", ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        mScreenList.addScreen({
            "CURSED!", // title
            tile_bitmap_warriors, //const uint16_t* bitmap ; 
            {
                {"You Lost", ST77XX_WHITE}, 
                {cursedWarriors, ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        mScreenList.addScreen({
            "CURSED!", // title
            tile_bitmap_gold, //const uint16_t* bitmap ; 
            {
                {"You Lost", ST77XX_WHITE}, 
                {cursedGold, ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
 
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            gGameState.popScreen();    
        }
    }

} gPlayerCursed;

struct CitadelScreen : public GameScreen {
    ScreenList mScreenList;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        gGameState.soundManager.play(sanctuary_snd, true);
        mScreenList.clear();

        const int lastBuilding = player.mLastBuilding;
        player.mLastBuilding = (int)Location::Citadel;
        player.mLocation = (int)Location::Citadel;
		player.mTurnCompleted = true;

        // got all three keys and ready to double?
        if (player.mInventory[(int)Inventory::BrassKey] > 0
            && player.mInventory[(int)Inventory::SilverKey] > 0
            && player.mInventory[(int)Inventory::GoldKey] > 0
            && player.mInventory[(int)Inventory::Warriors] > 4
            && player.mInventory[(int)Inventory::Warriors] < 25
            && lastBuilding != (int)Location::Citadel) 
        {
            const int giftCount = player.adjustWarriors(player.mInventory[(int)Inventory::Warriors]);
            const String giftString = String(giftCount) + String(" Warriors!");    

            mScreenList.addScreen({
                "CITADEL", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        } 
        else if (player.mInventory[(int)Inventory::Warriors] <= 4)
        {
            const int giftCount = player.adjustWarriors(random(4)+5);
            const String giftString = String(giftCount) + String(" Warriors!");    
            
            mScreenList.addScreen({
                "CITADEL", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
            
        if (player.mInventory[(int)Inventory::Food] <= 5)
        {
            const int giftCount = player.adjustFood(random(6)+10);
            const String giftString = String(giftCount) + String(" Food!"); 

            mScreenList.addScreen({
                "CITADEL", // title
                tile_bitmap_food, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        
        if (player.mInventory[(int)Inventory::Gold] <= 7)
        {
            const int giftCount = player.adjustGold(random(6)+10);
            const String giftString = String(giftCount) + String(" Gold!");

            mScreenList.addScreen({
                "CITADEL", // title
                tile_bitmap_gold, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }

        if (mScreenList.count() == 0) {
            mScreenList.addScreen({
                "CITADEL", // title
                nullptr, //const uint16_t* bitmap ; 
                {
                    {"No help is", ST77XX_WHITE}, 
                    {"needed now", ST77XX_WHITE}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            gGameState.popScreen();    
        }
    }

} gCitadelScreen;


struct PlayerInventory : public GameScreen {
    ScreenList mScreenList;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        gGameState.soundManager.play(rotate_snd, true);
        mScreenList.clear();
        player.mTurnCompleted = true;

        for (int slot=0; slot < player.mInventory.size(); ++slot)
        {
            if (player.mInventory[slot] > 0) 
            {
                mScreenList.addScreen({
                    "INVENTORY", // title
                    getInventoryPicture(slot), //const uint16_t* bitmap ; 
                    {
                        {String(player.mInventory[slot]), ST77XX_GREEN}
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
        }

        if (!mScreenList.nextScreen()) {
            gGameState.displayManager.setDesiredLayout({
                "INVENTORY", // title
                nullptr, //const uint16_t* bitmap ; 
                {
                    {"Error!", ST77XX_WHITE}, 
                    {"Try later?", ST77XX_WHITE}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
            
            return;
        }
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            gGameState.popScreen();    
        }
    }

} gPlayerInventory;

struct DragonScreen : public GameScreen {
    ScreenList mScreenList;

    virtual void begin() override 
    {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        mScreenList.clear();

       	mScreenList.addScreen({
            "ATTACKED!", // title
            tile_bitmap_dragon, //const uint16_t* bitmap ; 
            {
                {"The Dragon attacks!", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        if (player.mInventory[(int)Inventory::Sword] <= 0) 
        {
        	gGameState.soundManager.play(dragon_snd, true);
            const int lostWarriors= player.adjustWarriors(-(player.mInventory[(int)Inventory::Warriors] >> 2));
            const int lostGold= player.adjustGold(-(player.mInventory[(int)Inventory::Gold] >> 2));

            gGameState.worldState.mDragonWarriors += lostWarriors;
            gGameState.worldState.mDragonGold += lostGold;
  
            String cursedWarriors = "1 Warrior";
            if (lostWarriors != 1) {
                cursedWarriors = String(lostWarriors) + String(" Warriors");           
            }
            String cursedGold = String(lostGold) + String(" Gold");

            mScreenList.addScreen({
                "ATTACKED!", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"The Dragon takes", ST77XX_WHITE},
                    {cursedWarriors, ST77XX_RED}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });

            mScreenList.addScreen({
                "ATTACKED!", // title
                tile_bitmap_gold, //const uint16_t* bitmap ; 
                {
                    {"The Dragon takes", ST77XX_WHITE},
                    {cursedGold, ST77XX_RED}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        else 
        {
			gGameState.soundManager.play(dragon_kill_snd, true);
		    const int newWarriors= player.adjustWarriors(gGameState.worldState.mDragonWarriors);
            const int newGold= player.adjustGold(gGameState.worldState.mDragonGold);

            player.mInventory[(int)Inventory::Sword] = 0;
            gGameState.worldState.mDragonWarriors = 0;
            gGameState.worldState.mDragonGold = 0;
           
            String gainedWarriors = "1 Warrior";
            if (newWarriors != 1) {
                gainedWarriors = String(newWarriors) + String(" Warriors");           
            }
            String gainedGold = String(newGold) + String(" Gold");

            mScreenList.addScreen({
                "DRAGON", // title
                tile_bitmap_sword, //const uint16_t* bitmap ; 
                {
                    {"You defeated", ST77XX_WHITE},
                    {"the Dragon!", ST77XX_WHITE}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
            
            mScreenList.addScreen({
                "DRAGON", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {gainedWarriors, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });

            mScreenList.addScreen({
                "DRAGON", // title
                tile_bitmap_gold, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {gainedGold, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }

        mScreenList.addScreen({
            "DRAGON", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"Move Dragon", ST77XX_WHITE}, 
                {"to any open", ST77XX_WHITE},
                {"Territory space", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
    
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) 
        {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else 
        {
            gGameState.popScreen();    
        }
    }

} gDragonScreen;

struct MoveBack : public GameScreen {
     virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        //player.mLastBuilding = (int)Location::Citadel;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(rotate_snd, true);
 
        gGameState.displayManager.setDesiredLayout({
            "GO BACK", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"You must return", ST77XX_WHITE},
                {"to your previous", ST77XX_WHITE},
                {"location", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        
    }

    virtual void onSelection() override {
        gGameState.popScreen();   
    }

} gMoveBack;

struct Frontier : public GameScreen {
     bool mGoBack= false;

	 virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        player.mTurnCompleted = true;

		// is this a valid move?
		if ((player.mKingdomCount == 1 && player.mInventory[(int)Inventory::BrassKey] == 0)
			|| (player.mKingdomCount == 2 && player.mInventory[(int)Inventory::SilverKey] == 0)
			|| (player.mKingdomCount == 3 && player.mInventory[(int)Inventory::GoldKey] == 0))
		{
			// key missing, go back
			mGoBack = true;
			gGameState.soundManager.play(player_hit_snd, true);
			gGameState.displayManager.setDesiredLayout({
				"NO EXIT", // title
				tile_bitmap_keymissing, //const uint16_t* bitmap ; 
				{
					{"You must find", ST77XX_WHITE},
					{"a key first", ST77XX_WHITE}
				}, //std::vector<TextLine> textLines;
				{
					{ "OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});
		}
		else if (player.mKingdomCount > 3) 
		{
			// no more Frontiers, go back
			mGoBack = true;
			gGameState.soundManager.play(player_hit_snd, true);
 			gGameState.displayManager.setDesiredLayout({
				"NO EXIT", // title
				nullptr, //const uint16_t* bitmap ; 
				{
					{"You may not", ST77XX_WHITE},
					{"exit this", ST77XX_WHITE},
					{"kingdom", ST77XX_WHITE}
				}, //std::vector<TextLine> textLines;
				{
					{ "OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});
 		}
		else {
			// go for it
			mGoBack = false;
			player.mLocation = (int)Location::Frontier;
			gGameState.soundManager.play(frontier_snd, true);

			player.mKingdomCount++;

			gGameState.displayManager.setDesiredLayout({
				"SAFE TRAVELS", // title
				nullptr, //const uint16_t* bitmap ; 
				{
					{"You travel", ST77XX_WHITE},
					{"into the next", ST77XX_WHITE},
					{"kingdom", ST77XX_WHITE}
				}, //std::vector<TextLine> textLines;
				{
					{ "OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});

		}        
    }

    virtual void onSelection() override {
        if (mGoBack) {
			gGameState.swapScreen(&gMoveBack);
			return;
		}
		gGameState.popScreen();   
    }

} gFrontier;


struct LostScreen : public GameScreen {
    bool mMoveBack= true;
    ScreenList mScreenList;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        //player.mLastBuilding = (int)Location::Citadel;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(lost_snd, true);
        mMoveBack = (player.mInventory[(int)Inventory::Scout] == 0);

        mScreenList.addScreen({
            "TERRITORY", // title
            tile_bitmap_lost, //const uint16_t* bitmap ; 
            {
                {"You become lost", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        if (!mMoveBack) {
            mScreenList.addScreen({
                "SAVED!", // title
                tile_bitmap_scout, //const uint16_t* bitmap ; 
                {
                    {"Your Scout shows", ST77XX_GREEN},
                    {"the way", ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
           
        }
        
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            if (mMoveBack) {
                gGameState.swapScreen(&gMoveBack);
            }
            else {
                gGameState.popScreen(); 
            }               
        }
    }

} gLostScreen;

struct PlagueScreen : public GameScreen {
    ScreenList mScreenList;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        //player.mLastBuilding = (int)Location::Citadel;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(plague_snd, true);

        mScreenList.addScreen({
            "TERRITORY", // title
            tile_bitmap_plague, //const uint16_t* bitmap ; 
            {
                {"You find a", ST77XX_RED},
                {"village afflicted", ST77XX_RED},
                {"with illness", ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        if (player.mInventory[(int)Inventory::Healer] > 0) {
            const int newWarriors= player.adjustWarriors(2);
           
            String gainedWarriors = "1 Warrior";
            if (newWarriors != 1) {
                gainedWarriors = String(newWarriors) + String(" Warriors");           
            }

            mScreenList.addScreen({
                "SAVED!", // title
                tile_bitmap_healer, //const uint16_t* bitmap ; 
                {
                    {"Your Healer", ST77XX_GREEN},
                    {"cured the sick", ST77XX_GREEN},
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });

            mScreenList.addScreen({
                "SAVED!", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You gain", ST77XX_GREEN},
                    {gainedWarriors, ST77XX_GREEN},
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
    
        }
        else
        {
            const int lostWarriors= player.adjustWarriors(-2);
            const int lostGold= player.adjustGold(0);

            String changedWarriors = "1 Warrior";
            if (lostWarriors != 1) {
                changedWarriors = String(lostWarriors) + String(" Warriors");           
            }
            String cursedGold = String(lostGold) + String(" Gold");

            mScreenList.addScreen({
                "PLAGUE!", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You lost", ST77XX_RED},
                    {changedWarriors, ST77XX_RED},
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });

            if (lostGold) {
                mScreenList.addScreen({
                    "PLAGUE!", // title
                    tile_bitmap_gold, //const uint16_t* bitmap ; 
                    {
                        {"You lost", ST77XX_RED},
                        {cursedGold, ST77XX_RED},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
        }

        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            gGameState.popScreen();               
        }
    }

} gPlagueScreen;

struct CurseGainScreen : public GameScreen {
    ScreenList mScreenList;
	int mGainedWarriors = 0;
	int mGainedGold = 0;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        mScreenList.clear();

        {
            const int giftCount = player.adjustWarriors(mGainedWarriors);
            const String giftString = String(mGainedWarriors) + String(" Warriors!");    
            
            mScreenList.addScreen({
                "CAST CURSE", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        {
            const int giftCount = player.adjustGold(mGainedGold);
            const String giftString = String(giftCount) + String(" Gold!");

            mScreenList.addScreen({
                "CAST CURSE", // title
                tile_bitmap_gold, //const uint16_t* bitmap ; 
                {
                    {"You Gained", ST77XX_WHITE}, 
                    {giftString, ST77XX_GREEN}
                }, //std::vector<TextLine> textLines;
                {
                    { "OK", 0}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
     
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            gGameState.popScreen();    
        }
    }

} gCurseGainScreen;

struct CursePlayer : public GameScreen {
    virtual void begin() override 
	{
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];
        gGameState.inputManager.clear();

		if (gGameState.worldState.mPlayers.size()>1) 
		{
			ScreenLayout screen(
				"CAST CURSE", // title
				tile_bitmap_wizard, //const uint16_t* bitmap ; 
				{
					{"Select player", ST77XX_YELLOW},
					{"to Curse!", ST77XX_YELLOW}
				}, //std::vector<TextLine> textLines;
				{},//std::vector<TextLine> options;
				0 //int selection = 0)      
			);

			screen.clearOptions();
			for (int i=0; i<gGameState.worldState.mPlayers.size(); ++i) 
			{
				if (i != playerIdx)
				{
					String PlayerOption = "PLAYER ";
					PlayerOption += String(i+1);

					screen.addOption(PlayerOption, i);
				}
			}

			gGameState.displayManager.setDesiredLayout(screen);
		}
    }

    virtual void onSelection() override {
		// curse selected player
		TextLine mSelectedOption = gGameState.displayManager.getSelectedOption();
		int cursedIdx = mSelectedOption.mValue;
        auto& cursed = gGameState.worldState.mPlayers[cursedIdx];
		int lostWarriors = 0;
		int lostGold = 0;

		cursed.Curse(lostWarriors, lostGold);
		gCurseGainScreen.mGainedGold = lostGold;
		gCurseGainScreen.mGainedWarriors = lostWarriors;
        gGameState.swapScreen(&gCurseGainScreen); 
        return;
    }

} gCursePlayer;


struct TreasureScreen : public GameScreen {
    ScreenList mScreenList;
    bool mWizardCurse= false;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
        mWizardCurse= false;

		mScreenList.clear();

        int rand = random(10);

        if (rand >=3 && rand < 5) 
        {
            if (player.mInventory[(int)Inventory::Sword] == 0)
			{
				player.mInventory[(int)Inventory::Sword] = 1;
                mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_sword, //const uint16_t* bitmap ; 
                    {
                        {"You found the", ST77XX_GREEN},
                        {"Dragon Sword", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
        }
        else if (rand < 6) 
        {
            if (gGameState.worldState.mPlayers.size() > 1)
			{
				mWizardCurse = true;
				mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_wizard, //const uint16_t* bitmap ; 
                    {
                        {"You may cast", ST77XX_GREEN},
                        {"a Curse", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
        }
        else if (rand < 7) 
        {
            if (player.mInventory[(int)Inventory::Pegasus] == 0)
			{
				player.mInventory[(int)Inventory::Pegasus] = 1;
                mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_pegasus, //const uint16_t* bitmap ; 
                    {
                        {"You found the", ST77XX_GREEN},
                        {"Pegasus", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
        }
		else if (rand >= 7) {
			// do we deserve and need a key?
			if (player.mInventory[(int)Inventory::BrassKey] == 0 && player.mKingdomCount > 0)
			{
				player.mInventory[(int)Inventory::BrassKey] = 1;
                mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_brasskey, //const uint16_t* bitmap ; 
                    {
                        {"You found a", ST77XX_GREEN},
                        {"Tower Key", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
			else if (player.mInventory[(int)Inventory::SilverKey] == 0 && player.mKingdomCount > 1)
			{
				player.mInventory[(int)Inventory::SilverKey] = 1;
                mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_silverkey, //const uint16_t* bitmap ; 
                    {
                        {"You found a", ST77XX_GREEN},
                        {"Tower Key", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }
			else if (player.mInventory[(int)Inventory::GoldKey] == 0 && player.mKingdomCount > 2)
			{
				player.mInventory[(int)Inventory::GoldKey] = 1;
                mScreenList.addScreen({
                    "REWARD", // title
                    tile_bitmap_goldkey, //const uint16_t* bitmap ; 
                    {
                        {"You found a", ST77XX_GREEN},
                        {"Tower Key", ST77XX_GREEN},
                    }, //std::vector<TextLine> textLines;
                    {
                        { "OK", 0}
                    },//std::vector<TextLine> options;
                    0 //int selection = 0)      
                });
            }

		}

		if (mScreenList.count() == 0) {
			int gold = player.adjustGold(random(11) + 10);
			if (gold > 0) 
			{
				String gifteddGold = String(gold) + String(" Gold");
				mScreenList.addScreen({
					"REWARD", // title
					tile_bitmap_gold, //const uint16_t* bitmap ; 
					{
						{"You found", ST77XX_GREEN},
						{gifteddGold, ST77XX_GREEN},
					}, //std::vector<TextLine> textLines;
					{
						{ "OK", 0}
					},//std::vector<TextLine> options;
					0 //int selection = 0)      
				});
			}
		}
        
        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
			if (mWizardCurse) {
				mWizardCurse = false;
				// goto curse screen
				gGameState.swapScreen(&gCursePlayer);
			}
			else {
				gGameState.popScreen();   
			}
        }
    }

} gTreasureScreen;

struct RunAway : public GameScreen {
     virtual void begin() override {

        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

		int killed = player.adjustWarriors(-1);
		player.adjustFood(0);

        String Title = "PLAYER ";
        Title += String(playerIdx+1);

        gGameState.inputManager.clear();
 
		if (killed) {
			gGameState.soundManager.play(player_hit_snd, true);
			gGameState.displayManager.setDesiredLayout({
				"RUN AWAY!", // title
				tile_bitmap_warrior, //const uint16_t* bitmap ; 
				{
					{"You lost one", ST77XX_RED},
					{"more Warrior!", ST77XX_RED}
				}, //std::vector<TextLine> textLines;
				{
					{ "OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});
		}
        else 
		{
			gGameState.soundManager.play(pegasus_snd, true);
			gGameState.displayManager.setDesiredLayout({
				"RUN AWAY!", // title
				nullptr, //const uint16_t* bitmap ; 
				{
					{"You Escaped!", ST77XX_YELLOW},
				}, //std::vector<TextLine> textLines;
				{
					{ "OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});
		}
    }

    virtual void onSelection() override {
        gGameState.popScreen(); 
        return;
    }

} gRunAway;


struct BattleScreen : public GameScreen {
    ScreenList mScreenList;
    int32_t mBrigands = 1;
    bool mFinalBattle= false;

    int32_t oddsOfWinning() {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
        int32_t brigs = mBrigands * 100;
        int32_t wars = player.mInventory[(int)Inventory::Warriors] * 100;

        if (wars > brigs) {
            return 75 - ( brigs / ( 4 * wars) );
        }

        return 25 + ( wars / ( 4 * brigs) );
    }

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
        mScreenList.clear();

        int minWarriors = 1;
        if (gGameState.worldState.mPlayers.size() == 1) {
            minWarriors = 0;
        }

        int odds = oddsOfWinning();
        String title = "SKIRMISH!";
        uint16_t brigColor = ST77XX_YELLOW;
        uint16_t warColor = ST77XX_YELLOW;

        if (random(100) < odds ) {
            gGameState.soundManager.play(pegasus_snd, true);
            title = "ROUND WON!";
            if (mBrigands > 1) {
                mBrigands = mBrigands>>1;
            }
            else {
                mBrigands = 0;
            }
            brigColor = ST77XX_RED;
        }
        else {
            gGameState.soundManager.play(player_hit_snd, true);
            title = "ROUND LOST!";
            player.adjustWarriors(-1);
            warColor = ST77XX_RED;
        }

        int warriors = player.mInventory[(int)Inventory::Warriors];

        mScreenList.addScreen({
            title, // title
            tile_bitmap_brigands, //const uint16_t* bitmap ; 
            {
                {String(mBrigands), brigColor}
            }, //std::vector<TextLine> textLines;
            {
                {}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        if (warriors == minWarriors) {
            // they win
             mScreenList.addScreen({
                "BATTLE LOST", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {String(warriors), warColor}
                }, //std::vector<TextLine> textLines;
                {
                    {"OK", 2}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        else if (mBrigands == 0) {
            // you wim
            mScreenList.addScreen({
                "BATTLE WON", // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {String(warriors), warColor}
                }, //std::vector<TextLine> textLines;
                {
                    {"OK", 3}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }
        else {
            // battle continues
            mScreenList.addScreen({
                title, // title
                tile_bitmap_warriors, //const uint16_t* bitmap ; 
                {
                    {String(warriors), warColor}
                }, //std::vector<TextLine> textLines;
                {
                    {"Fight", 0},
                    {"Run!", 1}
                },//std::vector<TextLine> options;
                0 //int selection = 0)      
            });
        }

        // display the screen list
        mScreenList.nextScreen();
    }

    virtual void update(int32_t elapsedMS) {
        if (mScreenList.count() > 0 && elapsedMS > 2500) {
            // display the screen list
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen();
        }
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            gGameState.soundManager.play(rotate_snd, true);
            mScreenList.nextScreen(); 
        }
        else {
            TextLine mSelectedOption = gGameState.displayManager.getSelectedOption();
            if (mSelectedOption.mValue == 2) // Battle ended in loss
            {
                gGameState.popScreen(); 
            }
            else if (mSelectedOption.mValue == 3) // Battle ended in victory
            {            
                if (mFinalBattle) {
                    // Victory screen!
                	gGameState.swapScreen(&gVictory); 
					return;
                }  
                // Treasure
                gGameState.swapScreen(&gTreasureScreen); 
				return;
            }
            else if (mSelectedOption.mValue == 1) // Run Away!
            {              
                // Run screen
                gGameState.swapScreen(&gRunAway); 
            }
            else {
                // continue fight!
                gGameState.swapScreen(this); 
            }
        }
    }

} gBattleScreen;

struct BattleStart : public GameScreen {
     bool mFinalBattle= false;

     int getBrigands() {
         int brigands = 16;

         if (mFinalBattle) {
            if (gGameState.worldState.mDifficultyLevel == 0) {
                brigands = random(16) + 17;
            }
            else if (gGameState.worldState.mDifficultyLevel == 1) {
                brigands = random(32) + 33;
            }
            else {
                brigands = random(48) + 17;
            }
         }
         else {
            const int playerIdx= gGameState.worldState.mCurrentPlayer;
            Player& player = gGameState.worldState.mPlayers[playerIdx];
            int warriors = player.mInventory[(int)Inventory::Warriors];

            brigands = warriors;

            if (gGameState.worldState.mDifficultyLevel == 0) {
                brigands = (warriors - 3) + random(7);
            }
            else if (gGameState.worldState.mDifficultyLevel == 1) {
                brigands = warriors + random(6);
            }
            else {
                brigands = warriors + (random(11) + 5);
            }

            if ( brigands < 3 )
            {
                brigands = (random(4) + 3);
            }
            if ( brigands > 99 )
            {
                brigands = 99;
            }
        }

         return brigands;
     }

     virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        //player.mLastBuilding = (int)Location::Citadel;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(battle_snd, true);
 
        int brigands = getBrigands();
        gBattleScreen.mBrigands = brigands;
        gBattleScreen.mFinalBattle = mFinalBattle;

        gGameState.displayManager.setDesiredLayout({
            "ATTACKED!", // title
            tile_bitmap_brigands, //const uint16_t* bitmap ; 
            {
                {String(brigands), ST77XX_RED}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        
    }

    virtual void onSelection() override {
        gGameState.swapScreen(&gBattleScreen); 
    }

} gBattleStart;

struct TerritoryMove : public GameScreen {
    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       

		int randomRoll = random(10);

        const int lastLocation = player.mLocation;
        player.mLocation = (int)Location::Territory;
        player.mTurnCompleted = true;

        if (randomRoll == 5) {
            gGameState.swapScreen(&gDragonScreen);
            return;
        }
        if (randomRoll == 6) {
			player.mLocation = lastLocation;
            gGameState.swapScreen(&gLostScreen);
            return;
        }
        if (randomRoll == 7) {
            gGameState.swapScreen(&gPlagueScreen);
            return;
        }
        if (randomRoll >= 8) {
            gBattleStart.mFinalBattle = false;
            gGameState.swapScreen(&gBattleStart);
            return;
        }

        gGameState.displayManager.setDesiredLayout({
            "TERRITORY", // title
            nullptr, //const uint16_t* bitmap ; 
            {   
                {"Safe move.", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });
        
    }

    virtual void onSelection() override {
        gGameState.popScreen();   
    }

} gTerritoryMove;


struct TombRuin : public GameScreen {
    ScreenList mScreenList;
	enum Outcome {
		Empty=0,
		Reward,
		Battle
	};
	Outcome mOutcome = Outcome::Empty;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
       
        int randomRoll = random(10);

        player.mLastBuilding = (int)Location::Ruin;
        player.mLocation = (int)Location::Ruin;
		player.mTurnCompleted = true;

        //gGameState.soundManager.play(rotate_snd, true);
		mScreenList.clear();

        mScreenList.addScreen({
            "EXPLORING", // title
            nullptr, //const uint16_t* bitmap ; 
            {   
                {"You look inside", ST77XX_WHITE},
				{"the darkness...", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {},//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        if (randomRoll < 2) {
            mOutcome = Outcome::Empty;
			gGameState.soundManager.play(tomb_nothing_snd, true);
			
			mScreenList.addScreen({
				"EXPLORING", // title
				nullptr, //const uint16_t* bitmap ; 
				{   
					{"You find", ST77XX_WHITE},
					{"nothing", ST77XX_WHITE}
				}, //std::vector<TextLine> textLines;
				{
					{"OK", 0}
				},//std::vector<TextLine> options;
				0 //int selection = 0)      
			});

        }
        else if (randomRoll == 2) {
            mOutcome = Outcome::Reward;
			gGameState.soundManager.play(tomb_snd, true);
        }
        else {
            mOutcome = Outcome::Battle;
			gGameState.soundManager.play(tomb_battle_snd, true);
        }

		mScreenList.nextScreen();
    }

    virtual void update(int32_t elapsedMS) {
        if (elapsedMS > 3000) 
		{
			if (mScreenList.count() > 0) 
			{
            	// display the screen list
				gGameState.inputManager.clear();

            	mScreenList.nextScreen();
			}
			else if (gGameState.displayManager.getOptionCount() == 0)
			{
				if (mOutcome == Outcome::Battle) {
					gBattleStart.mFinalBattle = false;
					gGameState.swapScreen(&gBattleStart); 
					return;
				}
				else if (mOutcome == Outcome::Reward) {
					gGameState.swapScreen(&gTreasureScreen); 
					return;
				}

				gGameState.popScreen();    
			}
		}
    }

    virtual void onSelection() override {
        if (mScreenList.count() > 0) {
            mScreenList.nextScreen(); 
        }
        else {
			if (mOutcome == Outcome::Battle) {
				gBattleStart.mFinalBattle = false;
				gGameState.swapScreen(&gBattleStart); 
				return;
			}
			else if (mOutcome == Outcome::Reward) {
				gGameState.swapScreen(&gTreasureScreen); 
				return;
			}

            gGameState.popScreen();    
        }
    }

} gTombRuin;

struct UsePegasus : public GameScreen {
     virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		player.mInventory[(int)Inventory::Pegasus] = 0;

		// pretend we are in a territory and our turn is not yet over
        player.mLocation = (int)Location::Territory;
        player.mTurnCompleted = false;

        gGameState.soundManager.play(rotate_snd, true);
 
        gGameState.displayManager.setDesiredLayout({
            "PEGASUS", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"Fly to any", ST77XX_WHITE},
                {"space in this", ST77XX_WHITE},
                {"Kingdom, then", ST77XX_WHITE},
				{"select action", ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });        
    }

    virtual void onSelection() override {
		// return to the turn menu to choose the landing
        gGameState.popScreen();   
    }

} gUsePegasus;

struct WrongKey : public GameScreen {
    virtual void begin() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		player.mLocation = (int)Location::DarkTower;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(bazaar_closed_snd, true);

        gGameState.displayManager.setDesiredLayout({
            "KEY RIDDLE", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"Wrong Answer", ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });        
     }

    virtual void onSelection() override {
        gGameState.popScreen();   
    }

} gWrongKey;

struct SecondKey : public GameScreen {
    virtual void begin() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		player.mLocation = (int)Location::DarkTower;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(pegasus_snd, true);

        ScreenLayout screen(
            "KEY RIDDLE", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"Which Key", ST77XX_WHITE},
                {"goes second?", ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {},//std::vector<TextLine> options;
            0 //int selection = 0)      
        );   

		screen.clearOptions();
		if (player.mFirstKey != 0) {
			screen.addOption("Brass", 0);
		}
		if (player.mFirstKey != 1) {
			screen.addOption("Silver", 1);
		}
		if (player.mFirstKey != 2) {
			screen.addOption("Gold", 2);
		}

		gGameState.displayManager.setDesiredLayout(screen);     
     }

    virtual void onSelection() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];

		int keyIndex= gGameState.displayManager.getSelectedOption().mValue;
		if ((int)player.mSecondKey == keyIndex)
		{
			// TOWER ATTACK!
			player.mRiddleSolved= true;
			gBattleStart.mFinalBattle = true;
			gGameState.swapScreen(&gBattleStart); 
		}
		else
		{
			gGameState.swapScreen(&gWrongKey); 
		}
    }

} gSecondKey;

struct FirstKey : public GameScreen {
    virtual void begin() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		player.mLocation = (int)Location::DarkTower;
        player.mTurnCompleted = true;

        gGameState.soundManager.play(darktower_snd, true);

        gGameState.displayManager.setDesiredLayout({
            "KEY RIDDLE", // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"Which Key", ST77XX_WHITE},
                {"goes first?", ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {
                { "Brass", 0},
				{ "Silver", 1},
				{ "Gold", 2},
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });        
     }

    virtual void onSelection() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];

		int keyIndex= gGameState.displayManager.getSelectedOption().mValue;
		if ((int)player.mFirstKey == keyIndex)
		{
			gGameState.swapScreen(&gSecondKey); 
		}
		else
		{
			gGameState.swapScreen(&gWrongKey); 
		}
    }

} gFirstKey;

struct BazaarClosed : public GameScreen {
    virtual void begin() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
        player.mTurnCompleted = true;

        gGameState.soundManager.play(bazaar_closed_snd, true);

        gGameState.displayManager.setDesiredLayout({
            "GO AWAY", // title
            tile_bitmap_bazaar, //const uint16_t* bitmap ; 
            {
                {"No Deal", ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {
                { "OK", 0}
            },//std::vector<TextLine> options;
            0 //int selection = 0)      
        });        
     }

    virtual void onSelection() override {
        gGameState.popScreen();   
    }

} gBazaarClosed;

struct Bazaar : public GameScreen {
	struct StoreItem {
		const uint16_t* bitmap;
		int inventorySlot;
		int minPrice;
		int limit;
		int price;

		StoreItem(
			const uint16_t* _bitmap,
			int _inventorySlot,
			int _minPrice,
			int _limit,
			int _price) 
		{
			bitmap = _bitmap;
			inventorySlot = _inventorySlot;
			minPrice = _minPrice;
			limit = _limit;
			price = _price;
		}
	};
	std::vector<StoreItem> mItems;
	int mItemIdx = 0;

	void setup() {
		mItemIdx = 0;
		mItems.clear();

		mItems.emplace_back(tile_bitmap_warrior, (int)Inventory::Warriors, 4, 99, random(7) + 4);
		mItems.emplace_back(tile_bitmap_food, (int)Inventory::Food, 1, 99, 1);
		mItems.emplace_back(tile_bitmap_beast, (int)Inventory::Beast, 4, 1, random(11) + 15);
		mItems.emplace_back(tile_bitmap_scout, (int)Inventory::Scout, 4, 1, random(11) + 15);
		mItems.emplace_back(tile_bitmap_healer, (int)Inventory::Healer, 4, 1, random(11) + 15);
        gGameState.soundManager.play(bazaar_snd, true);
	}

    virtual void begin() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		player.mLocation = (int)Location::Bazaar;
		player.mLastBuilding = (int)Location::Bazaar;
        player.mTurnCompleted = true;

		const int playerGold = player.mInventory[(int)Inventory::Gold];
		StoreItem& item= mItems[mItemIdx];

        ScreenLayout screen(
            "BAZAAR", // title
            item.bitmap, //const uint16_t* bitmap ; 
            {
                {String("Cost: ") + String(item.price) + String(" Gold: ") + String(playerGold), ST77XX_WHITE},
            }, //std::vector<TextLine> textLines;
            {},//std::vector<TextLine> options;
            0 //int selection = 0)      
        );   

		screen.clearOptions();
		if (player.mInventory[item.inventorySlot] >= item.limit) {
			screen.clearInfo();
			screen.addInfo("out of stock", ST77XX_RED);
		}
		else 
		{
			if (item.price <= playerGold) {
				screen.addOption("Buy", 0);
			}
			if (item.inventorySlot != (int)Inventory::Food) 
			{
				screen.addOption("Haggle", 1);
			}
		}

		screen.addOption("Next", 2);
		screen.addOption("Exit", 3);

		gGameState.displayManager.setDesiredLayout(screen);     
     }

    virtual void onSelection() override {
		const int playerIdx= gGameState.worldState.mCurrentPlayer;
        Player& player = gGameState.worldState.mPlayers[playerIdx];
		int mSelectedOption = gGameState.displayManager.getSelectedOption().mValue;
		StoreItem& item= mItems[mItemIdx];

		if (mSelectedOption == 0) {
			if (player.mInventory[item.inventorySlot] < item.limit) {
				player.mInventory[item.inventorySlot]++;
			}
			// reset the screen
			player.adjustGold(-item.price);
			gGameState.soundManager.play(pegasus_snd, true);
			begin();
			return;
		}
		else if (mSelectedOption == 1) {
			bool haggleSuccess = random(10) > 5;
			if (haggleSuccess) {
				if (item.price > 0) {
					item.price--;
				}
			}
			if (!haggleSuccess || item.price < item.minPrice) {
				gGameState.swapScreen(&gBazaarClosed);
			}
			else {
				// reset the screen
				gGameState.soundManager.play(enemy_hit_snd, true);
				begin();
			}
			return;
		}
		else if (mSelectedOption == 2) {
			mItemIdx++;
			if (mItemIdx >= mItems.size()) {
				mItemIdx = 0;
			}
			// reset the screen
	        gGameState.soundManager.play(rotate_snd, true);
			begin(); 
			return;
		}
		
		// exit
		gGameState.popScreen();   
    }

} gBazaar;

//
// Player Turn Menu
//

struct PlayerTurn : public GameScreen {
    enum Action {
        UsePegasus = 0,
        Territory,
        Frontier,
        Citadel,
        Sanctuary,
        TombRuin,
        Bazaar,
        DarkTower,
        ViewInventory,
        EndTurn
    };

	bool mPegasusLanding= false;
    TextLine mSelectedOption;

    virtual void begin() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        if (checkForEndGame()) {
            return;
        }

        String Title = "PLAYER ";
        Title += String(playerIdx+1);

        gGameState.displayManager.setTitle(Title);
        gGameState.displayManager.clearBitmaps();
        gGameState.displayManager.clearInfo();

        if (player.mInventory[(int)Inventory::Food] == 0) 
        {
            gGameState.displayManager.addInfo("A Warrior Starved!", ST77XX_RED);
        }
        else if (player.mInventory[(int)Inventory::Food] < 5) 
        {
            gGameState.displayManager.addInfo("Food is low!", ST77XX_YELLOW);
        }
        
		if (!mPegasusLanding) {
			if (player.mTurnCompleted) 
			{
				gGameState.displayManager.addInfo("End Your Turn", ST77XX_WHITE);
			}
			else 
			{
				gGameState.displayManager.addInfo("Select Action:", ST77XX_WHITE);
			}
		}
		else {
			gGameState.displayManager.addInfo("Choose Landing:", ST77XX_WHITE);
		}
        gGameState.displayManager.clearOptions();
        
        if (!player.mTurnCompleted) 
        {
            if (!mPegasusLanding && player.mInventory[(int)Inventory::Pegasus] > 0)
            {
                gGameState.displayManager.addOption("Use Pegasus", (int)Action::UsePegasus);     
            }

            gGameState.displayManager.addOption("Territory", (int)Action::Territory);

            if (player.mLocation == (int)Location::Territory) 
            {
                gGameState.displayManager.addOption("Frontier", (int)Action::Frontier);     
            }

            if (player.mLocation == (int)Location::Citadel || 
                (player.mLocation == (int)Location::Territory && (player.mKingdomCount == 0 || player.mKingdomCount > 3))) 
            {
                gGameState.displayManager.addOption("Citadel", (int)Action::Citadel);     
            }

            if (player.mLocation == (int)Location::Sanctuary || player.mLocation == (int)Location::Territory) 
            {
                gGameState.displayManager.addOption("Sanctuary", (int)Action::Sanctuary);     
            }

            if (player.mLocation == (int)Location::Ruin || player.mLocation == (int)Location::Territory) 
            {
                gGameState.displayManager.addOption("Tomb/Ruin", (int)Action::TombRuin);     
            }
            
            if (player.mLocation == (int)Location::Bazaar || player.mLocation == (int)Location::Territory) 
            {
                gGameState.displayManager.addOption("Bazaar", (int)Action::Bazaar);     
            }

            if (player.mLocation == (int)Location::DarkTower || player.mLocation == (int)Location::Territory) 
            {
				if (player.mInventory[(int)Inventory::BrassKey] > 0
					&& player.mInventory[(int)Inventory::SilverKey] > 0
					&& player.mInventory[(int)Inventory::GoldKey] > 0
					&& player.mKingdomCount > 3)
				{
                	gGameState.displayManager.addOption("DarkTower", (int)Action::DarkTower);     
				}
            }
        }
        
		gGameState.displayManager.setSelection(0);

		if (!mPegasusLanding) {
            if (!player.mTurnCompleted)
            {
        	gGameState.displayManager.addOption("Inventory", (int)Action::ViewInventory);
            }

			if (player.mTurnCompleted)
			{
        		gGameState.displayManager.addOption("End Turn", (int)Action::EndTurn);
				gGameState.displayManager.setSelection(gGameState.displayManager.getOptionCount()-1);
			}
		}

		mPegasusLanding= false;
    };

    virtual void onSelection() override 
    {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];

        // if cursed, jump to cursed screen no matter what was selected
        if (player.mWasCursed) 
        {
            gGameState.pushScreen(&gPlayerCursed);
            return;
        }  

        mSelectedOption = gGameState.displayManager.getSelectedOption();
        if (mSelectedOption.mValue == (int)Action::EndTurn) // End Turn
        {
            if (gGameState.worldState.mPlayers.size() > 1) 
            {
                //gGameState.confirmOrDeny(gGameState.displayManager.getTitle(), mSelectedOption.mText);
				// advance to the next player
            	playerStartTurn(gGameState.worldState.mCurrentPlayer + 1);
            }
            else 
            {
                playerStartTurn(gGameState.worldState.mCurrentPlayer);
            }
            return;
        }
        else if (mSelectedOption.mValue == (int)Action::ViewInventory) // Inventory
        {
            // show the inventory screen, then return
            gGameState.pushScreen(&gPlayerInventory);
            return;
        }

        // everything else is verified
        gGameState.confirmOrDeny(gGameState.displayManager.getTitle(), mSelectedOption.mText);
    };

    virtual void confirm() override {
        const int playerIdx= gGameState.worldState.mCurrentPlayer;
        auto& player = gGameState.worldState.mPlayers[playerIdx];
		mPegasusLanding= false;

        if (mSelectedOption.mValue == (int)Action::EndTurn) // End Turn
        {
            // advance to the next player
            playerStartTurn(gGameState.worldState.mCurrentPlayer + 1);
            return;
        }
        if (mSelectedOption.mValue == (int)Action::Citadel) // End Turn
        {
             gGameState.pushScreen(&gCitadelScreen);
            return;
        }
        if (mSelectedOption.mValue == (int)Action::Sanctuary) // End Turn
        {
            gGameState.pushScreen(&gCitadelScreen);
            return;
        }
        if (mSelectedOption.mValue == (int)Action::Territory) // End Turn
        {
             gGameState.pushScreen(&gTerritoryMove);
            return;
        }
		if (mSelectedOption.mValue == (int)Action::TombRuin)
		{
            gGameState.pushScreen(&gTombRuin);
            return;			
		}
		if (mSelectedOption.mValue == (int)Action::UsePegasus)
		{
			mPegasusLanding = true;
            gGameState.pushScreen(&gUsePegasus);
            return;			
		}
		if (mSelectedOption.mValue == (int)Action::Frontier)
		{
            gGameState.pushScreen(&gFrontier);
            return;			
		}	
		if (mSelectedOption.mValue == (int)Action::DarkTower)
		{
			if (player.mRiddleSolved) {
				// TOWER ATTACK!
				player.mRiddleSolved= true;
				gBattleStart.mFinalBattle = true;
				gGameState.pushScreen(&gBattleStart); 
			}
			else
			{
				// TOWER RIDDLE
				gGameState.pushScreen(&gFirstKey); 
			}
			return;	
		}	
		if (mSelectedOption.mValue == (int)Action::Bazaar) {
			gBazaar.setup();
			gGameState.pushScreen(&gBazaar); 
			return;
		}
        // redisplay the menu
        begin();
    };

    virtual void deny() override 
    {
        // redisplay the menu
        begin();
    };
 
} gPlayerTurnScreen;

struct HomeKingdom : public AutoConfirmScreen {
    virtual void begin() override {
        const int playerIdx = gGameState.worldState.mCurrentPlayer;
        String Title = "PLAYER ";
        Title += String(playerIdx+1);

        std::vector<int> openKingdoms;
        openKingdoms.reserve((int)Kingdom::COUNT);
        for (int i=0; i<(int)Kingdom::COUNT; ++i) {
            bool open = true;
            for (Player& player: gGameState.worldState.mPlayers) {
                if (player.mHomeKingdom == i) {
                    open = false;
                }
            }    
            if (open) {
                openKingdoms.push_back(i);    
            }
        } 

        if (openKingdoms.size() == 1) {
            // just one left. no need for a menu
            gGameState.worldState.mPlayers[playerIdx].mHomeKingdom = openKingdoms[0];
            gGameState.popScreen();
            return;
        }

        ScreenLayout screenLayout({
            Title, // title
            nullptr, //const uint16_t* bitmap ; 
            {
                {"What is your", ST77XX_WHITE}, 
                {"home kingdom?", ST77XX_WHITE}
            }, //std::vector<TextLine> textLines;
            {},//std::vector<TextLine> options;
            0 //int selection = 0)      
        });

        for (int i : openKingdoms) {
            screenLayout.addOption(getKingdomName(i), i);   
        }
        gGameState.displayManager.setDesiredLayout(screenLayout);

    }

    virtual void confirm() override {
        int player= gGameState.worldState.mCurrentPlayer;
        gGameState.worldState.mPlayers[player].mHomeKingdom = mSelectedOption.mValue;
        gGameState.popScreen();
        gGameState.setActiveScreen(&gPlayerTurnScreen); 
    };

    virtual void deny() override {
        begin(); 
    };
} gHomeKingdom;


void playerStartTurn(int playerIndex) 
{
    if (playerIndex < 0 || playerIndex >= gGameState.worldState.mPlayers.size()) 
    {
        playerIndex = 0;
    }
 
	gPlayerTurnScreen.mPegasusLanding= false;

    gGameState.soundManager.play(end_turn_snd, true);
    while(gGameState.soundManager.isPlaying()) {
        delay(500);
        gGameState.soundManager.update();
    }
    gGameState.worldState.mCurrentPlayer = playerIndex;
    auto& player = gGameState.worldState.mPlayers[playerIndex];
    
    player.mTurnCompleted = false;
    if (!isValidKingdom(player.mHomeKingdom)) {
        gGameState.pushScreen(&gHomeKingdom);
        return;
    }

    int foodRemaining = gGameState.worldState.mPlayers[playerIndex].consumeFood();
    if (foodRemaining == 0 && gGameState.worldState.mPlayers[playerIndex].adjustWarriors(-1) != 0) {
        gGameState.soundManager.play(plague_snd, true);
    }
    else if (foodRemaining < 5) {
        gGameState.soundManager.play(starving_snd, true);
    }
    gGameState.setActiveScreen(&gPlayerTurnScreen);    

};


//
// DiffcultyLevel
//
struct DiffcultyLevel : public AutoConfirmScreen {
    virtual void begin() override {
        gGameState.displayManager.setTitle("OPTIONS");
        gGameState.displayManager.clearBitmaps();
        gGameState.displayManager.clearInfo();
        gGameState.displayManager.addInfo("Difficulty?", ST77XX_WHITE);
        gGameState.displayManager.clearOptions();
        gGameState.displayManager.addOption("Easy", 0);
        gGameState.displayManager.addOption("Medium", 1);
        gGameState.displayManager.addOption("Hard", 2);
        gGameState.displayManager.setSelection(0);
    }

    virtual void confirm() override {
        gGameState.worldState.mDifficultyLevel = mSelectedOption.mValue;
        playerStartTurn(0);
    };
    virtual void deny() override {
       begin(); 
    };

} gDiffcultyLevel;


//
// PlayerCount
//
struct PlayerCount : public AutoConfirmScreen {
    virtual void begin() override {
        gGameState.displayManager.setTitle("PLAYERS");
        gGameState.displayManager.clearBitmaps();
        gGameState.displayManager.clearInfo();
        gGameState.displayManager.addInfo("How Many Players?", ST77XX_WHITE);
        gGameState.displayManager.clearOptions();
        gGameState.displayManager.addOption("Solo Game", 1);
        gGameState.displayManager.addOption("Two Players", 2);
        gGameState.displayManager.addOption("Three Players", 3);
        gGameState.displayManager.addOption("Four Players", 4);
        gGameState.displayManager.setSelection(0);
    }

    virtual void confirm() override {
        gGameState.worldState.CreatePlayers(mSelectedOption.mValue);
        gGameState.setActiveScreen(&gDiffcultyLevel);
    };
    virtual void deny() override {
       gGameState.setActiveScreen(this);
    };

} gPlayerCount;

//
// confirm or deny screen
//
struct ConfirmOrDeny : public GameScreen {
    String mTitle= "";
    String mInfo= "";

    virtual void begin() override {
        gGameState.displayManager.setTitle(mTitle);
        gGameState.displayManager.clearBitmaps();
        gGameState.displayManager.clearInfo();
        gGameState.displayManager.addInfo(mInfo, ST77XX_WHITE);
        gGameState.displayManager.addInfo("Are you sure?", ST77XX_WHITE);
        gGameState.displayManager.clearOptions();
        gGameState.displayManager.addOption("Yes", 0);
        gGameState.displayManager.addOption("Go back", 1);
        gGameState.displayManager.setSelection(0);
    }

    virtual void onSelection() override {
    }
  
} gConfirmOrDeny;


GameScreen* setupConfirmOrDenyScreen(const String& title, const String& info) {
    gConfirmOrDeny.mTitle = title;
    gConfirmOrDeny.mInfo = info;
    return &gConfirmOrDeny;
}

//
// StartupScreen
//
struct StartupScreen : public GameScreen {

    virtual void begin() override {
        gGameState.inputManager.clear();
    
        gGameState.displayManager.setTitle("Welcome");
        gGameState.displayManager.setBitmapAndValue(tile_bitmap_logo, -1);
        gGameState.displayManager.clearInfo();
        gGameState.displayManager.addInfo("Mini Tower Version", ST77XX_WHITE);
        gGameState.displayManager.addInfo("Are you ready?", ST77XX_WHITE);
        gGameState.displayManager.clearOptions();
        gGameState.displayManager.addOption("Begin", 0);
        gGameState.displayManager.setSelection(0);
        gGameState.soundManager.play(intro_snd, true);
    }

    virtual void onSelection() override {
        gGameState.setActiveScreen(&gPlayerCount);
    }

} gStartupScreen;


GameScreen* getStartupScreen() {
    return &gStartupScreen;
}

GameScreen* getConfirmOrDenyScreen() {
    return &gConfirmOrDeny;
}
