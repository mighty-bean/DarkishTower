#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <array>
#include <vector>
#include "screen_layout.h"

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        6//10
  #define TFT_RST        -1//9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         5//3//8
#endif

class DisplayManager {

public:
    void setup();
    
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t *pcolors);
    void drawRGBBitmap2X( int16_t x, int16_t y, const uint16_t *pcolors);

    void repaint();

    const ScreenLayout& getState() const {
        return mDesiredLayout;
    }
    int getSelection() const {
        return mDesiredLayout.mSelection;
    }
    TextLine getSelectedOption() {
        return mDesiredLayout.mOptions[getSelection()];
    }

    void setTitle(const String& title);
    String getTitle() const;

    void setBitmapsAndValues(const uint16_t* primaryBitmap, const uint16_t* secondaryBitmap);
    void setBitmapAndValue(const uint16_t* primaryBitmap, int value);
    void clearBitmaps();

    void setValues(int left, int right, uint16_t leftColor, uint16_t rightColor);
    void clearValues();

    void addInfo(const String& text, uint16_t color);
    void clearInfo();

    void addOption(const String& text, uint16_t value);
    void clearOptions();
    int32_t getOptionCount();
     
    void pinSelection();

    void setSelection(int index);
    
    void forceRepaint();

    int16_t drawTextCentered(const String& text, int yPos, uint16_t color);

    void setDesiredLayout(const ScreenLayout& layout);
    void update();
   
private:
    Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    ScreenLayout mCurrentLayout;
    ScreenLayout mDesiredLayout;
    bool mForceRepaint= true;
    int mStartLine= 0;

    int mPostTitleVert = 0;
    int mPostBitmapVert = 0;
    int mPostNumbersVert = 0;
    int mPostTextVert = 0;
    int mPostOptionsVert = 0;
};

#endif // DISPLAY_MANAGER_H