
#include "display_manager.h"
#include "assets/SerifGothicStd_Bold12pt7b.h"
#include "assets/SerifGothicStd_Bold20pt7b.h"

#define SMALL_LINE_HEIGHT 23

void DisplayManager::setup() {
    // use this initializer (uncomment) if using a 2.0" 320x240 TFT:
    tft.init(240, 320);           // Init ST7789 320x240
    //	tft.setRotation(1);

    // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
    // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
    // may end up with a black screen some times, or all the time.
    tft.setSPISpeed(40000000);

    // we'll use the same font everywhere
    tft.setFont(&SerifGothicStd_Bold12pt7b);
    tft.setTextWrap(false);

    // clear the screen
    tft.fillScreen(ST77XX_BLACK);      

    setTitle("Booting");
}

void DisplayManager::drawRGBBitmap(
    int16_t x, int16_t y, 
    const uint16_t *pcolors) 
{
    tft.drawRGBBitmap(x, y, pcolors, 120, 90);
}

void DisplayManager::drawRGBBitmap2X(
    int16_t x, int16_t y, 
    const uint16_t *pcolors) 
{
    int16_t w = 120;
    int16_t h = 90;
    int16_t _width = tft.width() >> 1;
    int16_t _height = tft.height() >> 1;
    static uint16_t scanline[320];
    int16_t x2, y2;                 // Lower-right coord
    if ((x >= _width) ||            // Off-edge right
        (y >= _height) ||           // " top
        ((x2 = (x + w - 1)) < 0) || // " left
        ((y2 = (y + h - 1)) < 0))
        return; // " bottom

    int16_t bx1 = 0, by1 = 0, // Clipped top-left within bitmap
        saveW = w;            // Save original bitmap width value
    if (x < 0) {              // Clip left
        w += x;
        bx1 = -x;
        x = 0;
    }
    if (y < 0) { // Clip top
        h += y;
        by1 = -y;
        y = 0;
    }
    if (x2 >= _width)
        w = _width - x; // Clip right
    if (y2 >= _height)
        h = _height - y; // Clip bottom

    pcolors += (by1 * saveW + bx1 + 1); // Offset bitmap ptr to clipped top-left
    tft.startWrite();
    tft.setAddrWindow(x, y, w*2, h*2); // Clipped area

    while (h--) {              // For each (clipped) scanline...
        const uint16_t *srcColors = pcolors;
        for (int src=0; src<w ; ++src) {
            int d= src*2;
            scanline[d] = *srcColors;
            scanline[d+1] = *srcColors;
            ++srcColors;
        }
        tft.writePixels(scanline, w*2); // Push one (clipped) row
        tft.writePixels(scanline, w*2); // Push one (clipped) row
        pcolors += saveW;        // Advance pointer by one full (unclipped) line
    }
    tft.endWrite();
} 

void DisplayManager::setTitle(const String& title) {
    mDesiredLayout.mTitle = title;
}
String DisplayManager::getTitle() const {
    return mDesiredLayout.mTitle;
}

void DisplayManager::setBitmapsAndValues(const uint16_t* primaryBitmap, const uint16_t* secondaryBitmap) {
    mDesiredLayout.mBitmap = primaryBitmap;
}
void DisplayManager::setBitmapAndValue(const uint16_t* primaryBitmap, int value) {
    mDesiredLayout.mBitmap = primaryBitmap;
}
void DisplayManager::clearBitmaps() {
    mDesiredLayout.mBitmap = nullptr;
}

void DisplayManager::addInfo(const String& text, uint16_t color) {
    mDesiredLayout.mTextLines.emplace_back(text, color);
}
void DisplayManager::clearInfo() {
    mDesiredLayout.mTextLines.clear();
}

void DisplayManager::addOption(const String& text, uint16_t value) {
    mDesiredLayout.mOptions.emplace_back(text, value);
}
void DisplayManager::clearOptions() {
    mDesiredLayout.mOptions.clear();
}
int32_t DisplayManager::getOptionCount() {
    return mDesiredLayout.mOptions.size();
}

void DisplayManager::pinSelection() {
    int lastOption = mDesiredLayout.mOptions.size()-1;
    mDesiredLayout.mSelection = std::min(mDesiredLayout.mSelection, lastOption);
    mDesiredLayout.mSelection = std::max(mDesiredLayout.mSelection, 0);    
}

void DisplayManager::setSelection(int index) {
    mDesiredLayout.mSelection = index; 
    pinSelection();
}

void DisplayManager::forceRepaint() {
    mForceRepaint = true;
}

int16_t DisplayManager::drawTextCentered(const String& text, int yPos, uint16_t color) {
    tft.setCursor(0, yPos);
    tft.setTextColor(ST77XX_BLACK);
    tft.print(text);
    int16_t endX= tft.getCursorX();
    int16_t inset = 120 - (endX >> 1);
    if (inset < 0) {
        inset = 0;
    }
    tft.setCursor(inset, yPos);
    tft.setTextColor(color);
    tft.print(text);

    return inset;
}

void DisplayManager::update() {
    int VertPos= 0;
    
    // Title
    const bool hadTitle = mCurrentLayout.mTitle.length() > 0;
    const bool hasTitle = mDesiredLayout.mTitle.length() > 0;
    if (hadTitle != hasTitle) {
        mForceRepaint= true;   
    }
    if (mForceRepaint || mCurrentLayout.mTitle != mDesiredLayout.mTitle) {
        tft.fillRect(0, VertPos, 240, VertPos+32, ST77XX_BLACK);
        tft.setFont(&SerifGothicStd_Bold20pt7b);
        drawTextCentered(mDesiredLayout.mTitle, VertPos + 27, ST77XX_WHITE);	

        if (mDesiredLayout.mTitle.length() != 0) {
            VertPos += 32;  
        }
        mCurrentLayout.mTitle = mDesiredLayout.mTitle;
        mPostTitleVert = VertPos;
    }
    else {
        VertPos = mPostTitleVert;
    }

    // bitmaps
    bool hadBitmaps = (mCurrentLayout.mBitmap != nullptr);
    bool hasBitmaps = (mDesiredLayout.mBitmap != nullptr);
    if (hasBitmaps != hadBitmaps) {
        mForceRepaint = true;    
    }
    if (mForceRepaint || mCurrentLayout.mBitmap != mDesiredLayout.mBitmap) {
        tft.fillRect(0, VertPos, 240, VertPos + (hadBitmaps * 90), ST77XX_BLACK);
        if (hasBitmaps) {
            drawRGBBitmap2X(0, VertPos, mDesiredLayout.mBitmap);
            VertPos += 180;
        }
		else {
			VertPos += 16;
		}
        mCurrentLayout.mBitmap = mDesiredLayout.mBitmap;
        mPostBitmapVert = VertPos;
    }
    else {
        VertPos = mPostBitmapVert;
    }

    // information
    const int hadTextLines = mCurrentLayout.mTextLines.size();
    const int hasTextLines = mDesiredLayout.mTextLines.size();
    if (hadTextLines != hasTextLines) {
        mForceRepaint= true;   
    }
    if (mForceRepaint || mCurrentLayout.mTextLines != mDesiredLayout.mTextLines) {
        // clear the remaining space
        if (VertPos < 320) {
            tft.fillRect(0, VertPos, 240, 320, ST77XX_BLACK);
        }
        mForceRepaint = true;

        tft.setFont(&SerifGothicStd_Bold12pt7b);	
        for (int i=0; i<mDesiredLayout.mTextLines.size(); ++i) {
            if (VertPos < 320 - SMALL_LINE_HEIGHT) {
                const TextLine& line = mDesiredLayout.mTextLines[i]; 
                drawTextCentered(line.mText, VertPos + 21, line.mValue);	
                VertPos += SMALL_LINE_HEIGHT;
            }
        }
		VertPos += 4; // add some buffer for decendant font elements on the last line

        mCurrentLayout.mTextLines = mDesiredLayout.mTextLines;
        mPostTextVert = VertPos;
    }
    else {
        VertPos = mPostTextVert;
    }

    // options
    const int hadOptions = mCurrentLayout.mOptions.size();
    const int hasOptions = mDesiredLayout.mOptions.size();
    if (hadOptions != hasOptions) {
        mForceRepaint= true;   
        mStartLine = 0;
    }
    pinSelection();
    if (mForceRepaint || mCurrentLayout.mOptions != mDesiredLayout.mOptions || mCurrentLayout.mSelection != mDesiredLayout.mSelection) {
        
        // clear the remaining space
        if (VertPos < 320) {
            tft.fillRect(0, VertPos, 240, 320, ST77XX_BLACK);
        }
        mForceRepaint = true;

        if (hasOptions > 0) {
            const int linesAvailable = (320 - VertPos) / SMALL_LINE_HEIGHT;
            const int lastLine = hasOptions - 1;
            const int buffer= linesAvailable > 2 ? 1 : 0;

            int minVisible = mDesiredLayout.mSelection - buffer;
            if (minVisible < 0) {
                minVisible = 0;
            }
            if (minVisible < mStartLine) {
                mStartLine = minVisible;
            }
            
            int maxVisible = mDesiredLayout.mSelection + buffer;
            if (maxVisible > lastLine) {
                maxVisible = lastLine;
            }
            int proposedStart = maxVisible - (linesAvailable-1);
            if (proposedStart > mStartLine) {
                mStartLine = proposedStart;
            }
            if (mStartLine < 0)  {
                mStartLine = 0;
            }

            if (linesAvailable > hasOptions) {
                int delta = linesAvailable - hasOptions; 
                VertPos += (delta * SMALL_LINE_HEIGHT) >> 1; 
            }
            
            tft.setFont(&SerifGothicStd_Bold12pt7b);
            for (int i=mStartLine; i<mDesiredLayout.mOptions.size(); ++i) {
                if (VertPos < 320 - SMALL_LINE_HEIGHT) {
                    const String& line = mDesiredLayout.mOptions[i].mText;
                    if (mDesiredLayout.mSelection == i && line.length() > 0) {
                        int16_t inset= drawTextCentered(line, VertPos + 21, ST77XX_YELLOW);	
                        inset -= 24;
                        if (inset > 0) {
                            tft.setCursor(inset, VertPos + 21);
                            tft.print(">");     
                        } 
                    } 
                    else {
                        drawTextCentered(line, VertPos + 21, ST77XX_WHITE);
                    }
                    VertPos += SMALL_LINE_HEIGHT;
                }
            }
        }

        mCurrentLayout.mOptions = mDesiredLayout.mOptions;
        mCurrentLayout.mSelection = mDesiredLayout.mSelection;
        mPostOptionsVert = VertPos;
    }
    else {
        VertPos = mPostOptionsVert;
    }

    mForceRepaint = false;
}

void DisplayManager::setDesiredLayout(const ScreenLayout& layout) {
    mDesiredLayout = layout;
}

void DisplayManager::repaint() {
    update();
}