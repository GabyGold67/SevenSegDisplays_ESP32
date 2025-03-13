/**
 * @file SevenSegDisplays.h
 * @brief Header file for the SevenSegDisplays_ESP32 library 
 * 
 * 
 * 
 * @author Gabriel D. Goldman
 * 
 * @version 3.0.0
 * 
 * @date First release: 20/12/2023 
 *       Last update:   13/03/2025 13:40 (GMT+0200)
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
  * @attention	This library was originally developed as part of the refactoring
  * process for an industrial machines security enforcement and productivity control
  * (hardware & firmware update). As such every class included complies **AT LEAST**
  * with the provision of the attributes and methods to make the hardware & firmware
  * replacement transparent to the controlled machines. Generic use attributes and
  * methods were added to extend the usability to other projects and application
  * environments, but no fitness nor completeness of those are given but for the
  * intended refactoring project, and for the author's projects requirements.  
  * 
  * @warning **Use of this library is under your own responsibility**
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 Games People play:   
  * Games people play, you take it or you leave it
  * Things that they say aren't alright
  * If I promised you the moon and the stars, would you believe it?
  * Games people play in the middle of the night
 *******************************************************************************
 */


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
    virtual bool begin();
    bool getCommAnode();
    uint8_t* getDspBuffPtr();
    uint8_t getDspDigits();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
    virtual bool stop();
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