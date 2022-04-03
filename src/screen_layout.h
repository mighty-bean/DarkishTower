#include <Arduino.h>

struct TextLine {
    TextLine(){};

    TextLine(const String& str, uint16_t value) {
        mText= str;
        mValue = value;
    }
    TextLine(const TextLine& rha) {
        mText = rha.mText;
        mValue = rha.mValue;
    }

    bool operator==(const TextLine& rha) const {
        return mText == rha.mText && mValue == rha.mValue;
    }
    bool operator!=(const TextLine& rha) const {
        return mText != rha.mText || mValue != rha.mValue;
    }
    void operator=(const TextLine& rha) {
        mText = rha.mText;
        mValue = rha.mValue;
    }
    String mText;
    uint16_t mValue;
};

struct ScreenLayout {
    
    String mTitle = "";
    const uint16_t* mBitmap = nullptr; 
    std::vector<TextLine> mTextLines;
    std::vector<TextLine> mOptions;
    int mSelection = 0;

    bool operator==(const ScreenLayout& rha) const {
        return mTitle == rha.mTitle 
            && mBitmap == rha.mBitmap
            && mTextLines == rha.mTextLines
            && mOptions == rha.mOptions
            && mSelection == rha.mSelection;
    }

    bool operator!=(const ScreenLayout& rha) const {
        return mTitle != rha.mTitle 
            || mBitmap != rha.mBitmap
            || mTextLines != rha.mTextLines
            || mOptions != rha.mOptions
            || mSelection != rha.mSelection;
    }

    void operator=(const ScreenLayout& rha) {
        mTitle = rha.mTitle; 
        mBitmap = rha.mBitmap;
        mTextLines = rha.mTextLines;
        mOptions = rha.mOptions;
        mSelection = rha.mSelection;
    }

    ScreenLayout() {};

    ScreenLayout( const String& title,
        const uint16_t* bitmap, 
        const std::vector<TextLine>& textLines,
        const std::vector<TextLine>& options,
        int selection = 0) 
    {
        mTitle = title;
        mBitmap = bitmap; 
        mTextLines = textLines;
        mOptions = options;
        mSelection = selection;
    };

    void addInfo(const String& text, uint16_t value) {
        mTextLines.push_back({text, value});
    }

    void clearInfo() {
        mTextLines.clear();
    }

    void addOption(const String& text, uint16_t value) {
        mOptions.push_back({text, value});
    }
    void clearOptions() {
        mOptions.clear();
    }  
    
};
