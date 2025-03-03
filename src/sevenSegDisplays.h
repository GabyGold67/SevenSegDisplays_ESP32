#ifndef sevenSegDisplays_H
#define sevenSegDisplays_H

#include <Arduino.h>
#include <SevenSegDispHw.h>

const int MAX_DIGITS_PER_DISPLAY{8};
const int MAX_DISPLAYS_QTY{10};

class SevenSegDisplays {
    static uint8_t _displaysCount;
    static uint16_t _dspSerialNum;
    static uint8_t _dspPtrArrLngth;
    static SevenSegDisplays** _instancesLstPtr;

    static TimerHandle_t _blinkTmrHndl;
    static TimerHandle_t _waitTmrHndl;

    static void tmrCbBlink(TimerHandle_t blinkTmrCbArg);
    static void tmrCbWait(TimerHandle_t waitTmrCbArg);
    
private:
    uint8_t _waitChar {0xBF};
    uint8_t _waitCount {0};
    bool _waiting {false};
    unsigned long _waitRate {250};
    unsigned long _waitTimer {0};
protected:
    const unsigned long _minBlinkRate{100};
    const unsigned long _maxBlinkRate{2000};

    bool* _blinkMaskPtr{nullptr};
    uint8_t* _dspAuxBuffPtr{nullptr};
    bool _dspBuffChng{false};
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspDigitsQty{};
    SevenSegDispHw _dspUndrlHw{};
    SevenSegDisplays* _dspInstance;
    uint16_t _dspInstNbr{0};
    int32_t _dspValMax{};
    int32_t _dspValMin{};
    
    bool _blinking{false};
    bool _blinkShowOn{false};
    unsigned long _blinkTimer{0};
    unsigned long _blinkOffRate{500};
    unsigned long _blinkOnRate{500};
    unsigned long _blinkRatesGCD{500};  //Holds the value for the minimum timer checking the change ON/OFF of the blinking, 
                                        //saving unneeded timer interruptions, and without the need of the std::gcd function
    String _charSet{"0123456789AabCcdEeFGHhIiJLlnOoPqrStUuY-_=~* ."}; // for using indexOf() method
    uint8_t _charLeds[45] = {   //Values valid for a Common Anode display. For a Common Cathode display values must be logically bit negated
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90, // 9
        0x88, // A
        0xA0, // a
        0x83, // b
        0xC6, // C
        0xA7, // c
        0xA1, // d
        0x86, // E
        0x84, // e
        0x8E, // F
        0xC2, // G
        0x89, // H
        0x8B, // h
        0xF9, // I
        0xFB, // i
        0xF1, // J
        0xC7, // L
        0xCF, // l
        0xAB, // n
        0xC0, // O
        0xA3, // o
        0x8C, // P
        0x98, // q
        0xAF, // r
        0x92, // S
        0x87, // t
        0xC1, // U
        0xE3, // u
        0x91, // Y
        0xBF, // Minus -
        0xF7, // Underscore _
        0xB7, // Low =
        0xB6, //~ for Equivalent symbol
        0x9C, // °
        0xFF, // Space
        0x7F  //.
    };    
    
    uint8_t _space {0xFF};
    uint8_t _dot {0x7F};
    String _zeroPadding{""};
    String _spacePadding{""};

    unsigned long blinkTmrGCD(unsigned long blnkOnTm, unsigned long blnkOffTm);
    void restoreDspBuff();
    void saveDspBuff();
    void setAttrbts();
    void updBlinkState();
    void updWaitState();
public:
    SevenSegDisplays();
    SevenSegDisplays(SevenSegDispHw dspUndrlHw);
    ~SevenSegDisplays();
    bool blink();
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    void clear();
    bool doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft = ' ', char labelRight = ' ');
    bool gauge(const int &level, char label = ' ');
    bool gauge(const double &level, char label = ' ');
    uint8_t getDigitsQty();
    uint32_t getDspValMax();
    uint32_t getDspValMin();
    uint16_t getInstanceNbr();
    unsigned long getMaxBlinkRate();
    unsigned long getMinBlinkRate();
    bool isBlank();
    bool isBlinking();
    bool isWaiting();
    bool noBlink();
    bool noWait();
    bool print(String text);
    bool print(const int32_t &value, bool rgtAlgn = false, bool zeroPad = false);
    bool print(const double &value, const unsigned int &decPlaces, bool rgtAlgn = false, bool zeroPad = false);
    void resetBlinkMask();
    void setBlinkMask(const bool* newBlnkMsk);
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    bool setWaitChar (const char &newChar);
    bool setWaitRate(const unsigned long &newWaitRate);
    bool wait();
    bool wait(const unsigned long &newWaitRate);
    bool write(const uint8_t &segments, const uint8_t &port);
    bool write(const String &character, const uint8_t &port);
};

//============================================================> Class declarations separator

class ClickCounter{
private:
    SevenSegDisplays _display;
    int _count{0};
    int _beginStartVal{0};
    bool _countRgthAlgn{true};
    bool _countZeroPad{false};
public:
    ClickCounter(uint8_t ccSclk, uint8_t ccRclk, uint8_t ccDio, bool rgthAlgn = true, bool zeroPad = false, bool commAnode = true, const uint8_t dspDigits = 4);
    ~ClickCounter();
    bool blink();
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    void clear();
    bool countBegin(int32_t startVal = 0);  //To be analyzed it's current need
    bool countDown(int32_t qty = 1);
    bool countReset();
    bool countRestart(int32_t restartValue = 0);
    bool countStop();   //To be analyzed it's current need
    bool countToZero(int32_t qty = 1);
    bool countUp(int32_t qty = 1);
    int32_t getCount();
    int32_t getStartVal();
    bool noBlink();
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    bool updDisplay();  //To be analyzed it's current need
};

#endif