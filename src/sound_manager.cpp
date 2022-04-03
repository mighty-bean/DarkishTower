#include <Arduino.h>
#include "sound_manager.h"
//#include <driver/dac.h>

void playSound(int pinSpk, const SoundFile& sound) {
   int64_t  lastTime = 0;
   for (int j = 0; j < sound.size; ++j ) {
      dacWrite(pinSpk, sound.data[j]);
      int64_t  currentTime = esp_timer_get_time();
      if (currentTime >= lastTime) {
         int64_t  elapsedTime = currentTime - lastTime;
         if (elapsedTime < 125) {
            int64_t waitTime = 125 - elapsedTime;
			if (j % 1024 == 0) {
				Serial.println(F("WaitTimeSample: "));
				Serial.println(waitTime, DEC);
			}
            usleep(waitTime);
         }
      }
      lastTime = currentTime;     
   }
}

void SoundManager::setup(int pin) {
    mOutputPin = pin;
    mMutex = xSemaphoreCreateMutex();

    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &audio_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .arg = (void*)this,
            .name = "audioPlayer",
            
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &mAudioTimer));
    
}

void SoundManager::play(const SoundFile& sound, bool waitForSong) {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    mRequest = &sound;
    mHaltPlayback = false;
    mWaitForSong = waitForSong;
    xSemaphoreGive(mMutex);
}

void SoundManager::stop() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    mRequest = nullptr;
    mHaltPlayback = true;
    xSemaphoreGive(mMutex);
}

bool SoundManager::isPlaying() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    bool playing = !mHaltPlayback && (mPlaying || mRequest != nullptr);
    xSemaphoreGive(mMutex);
    return playing;
}

bool SoundManager::isWaitingForSong() {
    return isPlaying() && mWaitForSong;
}

void SoundManager::update() {
    xSemaphoreTake(mMutex, portMAX_DELAY);
    bool playing = mPlaying;
    const SoundFile* request = mRequest;
    xSemaphoreGive(mMutex);
    
    if (!playing) {
        bool timerActive = esp_timer_is_active(mAudioTimer);
        if (request != nullptr && !timerActive) {
            // timer needs to start
            ESP_ERROR_CHECK(esp_timer_start_periodic(mAudioTimer, 126));
        }
        else if (mRequest == nullptr && timerActive) {
            // timer may stop
            ESP_ERROR_CHECK(esp_timer_stop(mAudioTimer));
        }
    }
}

void SoundManager::audio_timer_callback(void* arg) {
    SoundManager* sm = (SoundManager*)arg;
    sm->onTimer();
}

void SoundManager::onTimer() {
    // exchange data with the outside world
    xSemaphoreTake(mMutex, portMAX_DELAY);
    const SoundFile* request = mRequest;
    bool halt = mHaltPlayback;
    mRequest = nullptr;
    mHaltPlayback = false;
    mPlaying = !halt && (mRequest != nullptr || mActiveSound != nullptr);
    xSemaphoreGive(mMutex);
    
    if (request) {
        mActiveSound = request;
        mPlayPosition = 0;
        Serial.println("Starting playback!");
    }
    else if (halt) {
        mActiveSound = nullptr;
        mPlayPosition = 0;
        Serial.println("Ending playback!");
    }

    if (mActiveSound) {
        mPlayPosition++;
        if (mPlayPosition < mActiveSound->size) {
            dacWrite(mOutputPin, mActiveSound->data[mPlayPosition]);
        }
        else {
            dacWrite(mOutputPin, 0);
            Serial.println("Finished playback!");
            mActiveSound = nullptr;
            mPlayPosition = 0;
        }
    }
}