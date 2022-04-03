#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H


#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "FreeRTOS.h"
#include "semphr.h"

struct SoundFile
{
  uint32_t size;
  const uint8_t * data;
};

// play a sound as a blocking operation
void playSound(int pinSpk, const SoundFile& sound);

// a sound amanger to play async sounds using a hardware timer
class SoundManager {
	int mOutputPin;
	SemaphoreHandle_t mMutex;
	esp_timer_handle_t mAudioTimer;
	
	// internal, active state
	const SoundFile* mActiveSound = nullptr;
	uint32_t mPlayPosition = 0;
	bool mPlaying = false;
	
	// user requested state changes
	const SoundFile* mRequest = nullptr;
	bool mHaltPlayback = false;

	bool mWaitForSong = false;
public:	
	void setup(int pin);	
	void play(const SoundFile& sound, bool waitForSong= true);	
	void stop();	
	bool isPlaying();
	bool isWaitingForSong();
	void update();
	void onTimer();
	static void audio_timer_callback(void* arg);	
};

#endif