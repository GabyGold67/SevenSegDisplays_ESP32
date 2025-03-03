#ifndef sevenSegDispHw_H
#define sevenSegDispHw_H

#include "Arduino.h"
//============================================================> Class declarations separator

class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    bool _commAnode {true}; //SevenSegDisplays objects will retrieve this info to build the right segments for each character
    uint8_t* _digitPosPtr{nullptr};
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspDigitsQty{}; //Display size in digits    
    uint8_t _dspHwInstNbr{0};
    uint8_t* _ioPins{};

    // virtual void send(uint8_t* digitsBuffer);  //===================>> To be implemented
    // virtual void send(const uint8_t &segments, const uint8_t &port);  //===================>> To be implemented
public:
    SevenSegDispHw();
    SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDispHw();    
    bool getCommAnode();
    uint8_t* getDspBuffPtr();
    uint8_t getDspDigits();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
};

//============================================================> Class declarations separator

class SevenSegDynamic: public SevenSegDispHw{    
    static void tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg);  //Will easily fail in subclasses calls, check it!!
protected:
    TimerHandle_t _dspRfrshTmrHndl{nullptr};
    uint8_t _firstRefreshed{0};
    // void fastRefresh();  //===================>> To be implemented
    void refresh();
    // void send(uint8_t content);
    // void send(const uint8_t &segments, const uint8_t &port);
    TimerHandle_t _svnSgDynTmrHndl{NULL};
public:
    SevenSegDynamic();
    ~SevenSegDynamic();
    bool begin();
    bool stop();
};

//============================================================> Class declarations separator

class SevenSegDynHC595: public SevenSegDynamic{
    static void tmrCbRefreshHC595(TimerHandle_t rfrshTmrCbArg);  //Will easily fail in subclasses calls, check it!!
private:
    const uint8_t _sclk {0};
    const uint8_t _rclk {1};
    const uint8_t _dio {2};
protected:
    void refresh();
    void send(uint8_t content);
    void send(const uint8_t &segments, const uint8_t &port);
public:
    SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegDynHC595();
    bool begin();
    bool stop();
};

//============================================================> Class declarations separator

class SevenSegDynDummy: public SevenSegDynamic{
public:
    SevenSegDynDummy(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDynDummy();
};

//============================================================> Class declarations separator

class SevenSegStatic: public SevenSegDispHw{

public:
    // SevenSegStatic();    //No differentiated default constructor for this class yet!!
    ~SevenSegStatic();
};

//============================================================> Class declarations separator

class SevenSegTM1637: public SevenSegStatic{
    const uint8_t maxBrightLvl{0b0111};
    const uint8_t minBrightLvl{0b0000};
protected:
    uint8_t _brightLvl{};
    void send();
public:
    // SevenSegTM1637();
    bool setBrightness(uint8_t &newBrightLevel);
    bool turnOff();
    bool turnOn();
    ~SevenSegTM1637();
};

//============================================================> Class declarations separator

class SevenSegStatHC595: public SevenSegStatic{
// protected:
public:
    SevenSegStatHC595();
    ~SevenSegStatHC595();
};

//============================================================> Class declarations separator

class SevenSegStatDummy: public SevenSegStatic{
public:
    SevenSegStatDummy(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegStatDummy();
};

//============================================================> Class declarations separator

// Classes for the TM1638, Max7219, HT16K33 under implementation need analysis


#endif