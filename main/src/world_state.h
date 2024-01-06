#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <array>
#include <vector>

enum class Kingdom : uint8_t {
    Arisilon = 0,
    Brynthia,
    Durnin,
    Zenon,
    COUNT,
    UNKNOWN= 255,
};


enum class Location : uint8_t {
    Territory,
    Citadel,
    Sanctuary,
    Ruin,
    Bazaar,
    Frontier,
    DarkTower
};

enum class Inventory : uint8_t {
    Warriors,
    Gold,
    Food,
    Beast,
    Scout,
    Healer,
    Pegasus,
    Sword,
    BrassKey,
    SilverKey,
    GoldKey,
    COUNT
};

enum class KeyOrder : uint8_t {
    Brass,
    Silver,
    Gold,
    COUNT
};

struct Player {
    uint8_t mIndex;
    uint8_t mHomeKingdom;
    uint8_t mCurrentKingdom;
    uint8_t mKingdomCount;
    uint8_t mLocation;
    uint8_t mLastBuilding;
    std::array<uint8_t, (size_t)Inventory::COUNT> mInventory;
    bool mTurnCompleted;
    bool mWasCursed;
    bool mIsSolo;
	bool mRiddleSolved;
    uint8_t mCursedGold;
    uint8_t mCursedWarriors;

	uint8_t mFirstKey = (uint8_t)KeyOrder::Brass;
	uint8_t mSecondKey = (uint8_t)KeyOrder::Silver;

    int adjustWarriors(int add) {
        int oldCount= mInventory[(uint8_t)Inventory::Warriors];
        int newCount = oldCount + add;
        newCount = std::max(0, newCount);
        newCount = std::min(99, newCount);
        if (!mIsSolo) {
            newCount = std::max(1, newCount);   
        }
        mInventory[(uint8_t)Inventory::Warriors] = newCount; 

        if (add > 0) {
            return std::max(0, newCount - oldCount);
        }
        return std::max(0, oldCount - newCount);
    }

    int adjustFood(int add) {
        int oldCount= mInventory[(uint8_t)Inventory::Food];
        int newCount = oldCount + add;
        newCount = std::max(0, newCount);
        newCount = std::min(99, newCount);
        mInventory[(uint8_t)Inventory::Food] = newCount; 
        
        if (add > 0) {
            return std::max(0, newCount - oldCount);
        }
        return std::max(0, oldCount - newCount);
    }

    int adjustGold(int add) {
        int oldCount= mInventory[(uint8_t)Inventory::Gold];
        int newCount = oldCount + add;
        int max = mInventory[(uint8_t)Inventory::Warriors] * 6 + (mInventory[(uint8_t)Inventory::Beast] * 50);  
        newCount = std::max(0, newCount);
        newCount = std::min(99, newCount);
        newCount = std::min(max, newCount);
        mInventory[(uint8_t)Inventory::Gold] = newCount; 
        
        if (add > 0) {
            return std::max(0, newCount - oldCount);
        }
        return std::max(0, oldCount - newCount);
    }    

    void Curse(int& lostWarriors, int& lostGold) {
        mWasCursed = true;
        uint8_t CursedGold = mInventory[(uint8_t)Inventory::Gold] >> 2;
        uint8_t CursedWarriors = mInventory[(uint8_t)Inventory::Warriors] >> 2;
        lostGold = adjustGold(-CursedGold);
        lostWarriors = adjustWarriors(-CursedWarriors);
        mCursedGold += lostGold;
        mCursedWarriors += lostWarriors;
    }

    void ClearCurse() {
        mWasCursed = false;
        mCursedGold = 0;
        mCursedWarriors = 0;
    }

    int consumeFood() {
        const int warriors = mInventory[(uint8_t)Inventory::Warriors];
        int foodEaten = -1;
        if (warriors > 15 && warriors <= 30) {
            foodEaten = -2;
        }
        else if (warriors > 30 && warriors <= 45) {
            foodEaten = -3;
        }
        else if (warriors > 46 && warriors <= 60) {
            foodEaten = -4;
        }
        else if (warriors > 60 && warriors <= 75) {
            foodEaten = -5;
        }
        else if (warriors > 75 && warriors <= 90) {
            foodEaten = -6;
        }
        else if (warriors > 90) {
            foodEaten = -7;
        }
        adjustFood(foodEaten); 

        return mInventory[(uint8_t)Inventory::Food];  
    }

};

struct WorldState {
    std::vector<Player> mPlayers;
    uint8_t mCurrentPlayer = 0;
    int mDragonWarriors = 0;
    int mDragonGold = 0;
    int mDifficultyLevel = 0;

    void setup() {
        mPlayers.clear();  
        mCurrentPlayer = 0;
        mDragonWarriors = 0;
        mDragonGold = 0;
    }

    void CreatePlayers(uint8_t count) {
        count = std::min(count, (uint8_t)4);
        count = std::max(count, (uint8_t)1);
        mPlayers.clear();
        for (int i=0; i<count; ++i) {
            Player newPlayer;
            newPlayer.mIndex = mPlayers.size();
            newPlayer.mHomeKingdom = (uint8_t)Kingdom::UNKNOWN;
            newPlayer.mCurrentKingdom = (uint8_t)Kingdom::UNKNOWN;
            newPlayer.mKingdomCount = 0;
            newPlayer.mLocation = (uint8_t)Location::Citadel;
            newPlayer.mInventory.fill(0);
            newPlayer.mInventory[(uint8_t)Inventory::Warriors] = 10;
            newPlayer.mInventory[(uint8_t)Inventory::Gold] = 30;
            newPlayer.mInventory[(uint8_t)Inventory::Food] = 25;
            newPlayer.mTurnCompleted= false;
            newPlayer.mWasCursed = false;
            newPlayer.mIsSolo = false;
            newPlayer.mCursedGold = 0;
            newPlayer.mCursedWarriors = 0;
            newPlayer.mLastBuilding = (uint8_t)Location::Citadel;
			newPlayer.mRiddleSolved = false;

			// chose the random key order for this player
			newPlayer.mFirstKey = (uint8_t)random((int)KeyOrder::COUNT);
			int secondIndex= ((int)newPlayer.mFirstKey + (random(2)==0 ? -1 : 1));
			if (secondIndex < 0) 
			{
				secondIndex += (int)KeyOrder::COUNT;
			}
			else if (secondIndex >= (int)KeyOrder::COUNT)
			{
				secondIndex -= (int)KeyOrder::COUNT;
			}
			newPlayer.mSecondKey = (uint8_t)secondIndex;

            mPlayers.push_back(newPlayer);
        }

        // adjust for single player
        if (count == 1) {
            mPlayers[0].mKingdomCount = 0;
            mPlayers[0].mIsSolo = true;

/*			// TESTING
			//newPlayer.mInventory[(uint8_t)Inventory::Warriors] = 10;
			mPlayers[0].mInventory[(uint8_t)Inventory::Pegasus] = 1;
			mPlayers[0].mInventory[(uint8_t)Inventory::BrassKey] = 1;
			mPlayers[0].mInventory[(uint8_t)Inventory::SilverKey] = 1;
			mPlayers[0].mInventory[(uint8_t)Inventory::GoldKey] = 1;
*/
        }
    }
};

#endif