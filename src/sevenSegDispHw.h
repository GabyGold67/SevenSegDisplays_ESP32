/**
 * @file SevenSegDispHw.h
 * @brief Header file for the SevenSegDisplays_ESP32 library, SevenSegDispHw class and subclasses 
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
  * "Games people play, you take it or you leave it
  * Things that they say aren't alright
  * If I promised you the moon and the stars, would you believe it?"
 *******************************************************************************
 */
#ifndef sevenSegDispHw_H
#define sevenSegDispHw_H

#include "Arduino.h"
#include <stdint.h>

//============================================================> Class declarations separator
class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    bool _commAnode {true}; // SevenSegDisplays objects need this info to build the right segments to represent each character
    uint8_t* _digitPosPtr{nullptr};
    uint8_t* _dspBuffPtr{nullptr};  //FFDR Shouldn't the display buffer shared with the display better be placed in the HEAP?
    uint8_t _dspDigitsQty{}; // Display size in digits    
    uint8_t _dspHwInstNbr{0};
    uint8_t* _ioPins{};

    virtual void send(uint8_t* digitsBuffer);
    virtual void send(const uint8_t &segments, const uint8_t &port);
public:
    SevenSegDispHw();
    SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDispHw();    
   /**
    * @brief Sets up the hardware display to work.  
    * 
    * Depending on the display technology and the resources it needs to start working, this method takes care of those preparation. That means that each specific subclass of display will have to provide it's version of `begin()` that will take care of:  
    * - Configuring timmers or interrupts.  
    * - Setup communications parameters.  
    * - Establish communications with the display.  
    * - Other specific services configuration and starting.  
    * 
    * @retval true The specific configurations and startups could be successfuly made
    * @return false One or more of the specific configurations or startups failed.  
    */
    virtual bool begin();
    /**
     * @brief Returns a value indicating if the hardware has the led display wired as common anode or common cathode
     * 
     * The SevenSegDisplays instantiated objects will compose the values corresponding to each character it can display according to the SevenSegDispHw attribute _commAnode. Each SevenSegDispHw instantiable subclass will have that constant attribute set by the subclass developer to correspond to the technical specifications of the display hardware. 
     * 
     * @retval true The display is built with Common Annode seven segment display modules
     * @retval false The display is built with Common Cathode seven segment display modules
     */
    bool getCommAnode();
    uint8_t* getDspBuffPtr();
    uint8_t getHwDspDigitsQty();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    /**
     * @brief Returns the pointer to the Display Buffer
     * 
     * When a SevenSegDisplays object is instantiated it's constructor sets a display buffer memory area to store the contents ready to be displayed. Part of the constructor execution includes passing to the SevenSegDispHw subclass component that pointer, as the underlying hardware display object will be taking the information to display from that memory buffer. The resource to set the pointer is this method. 
     * 
     * @return uint8_t* The pointer to the **Display Buffer Memory Area**  
     * 
     * @attention Using this method is a resource to generate "animations" by changing the memory area from with the hardware displays takes it's contents, to some other area with ready to display information
     * 
     * @warning Setting the display buffer pointer to an address not coinciding with the one configured in the SevenSegDisplays will **disable** the possibility for it to get new generated content displayed!! Handle with extreme care!!
     */
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
    virtual bool stop();
};

//============================================================> Class declarations separator

class SevenSegDynamic: public SevenSegDispHw{    
    static void tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg);

protected:
    static TimerHandle_t _dynDspRfrshTmrHndl;
    uint8_t _firstRefreshed{0};
    void refresh();
    virtual void send(uint8_t content);
    virtual void send(const uint8_t &segments, const uint8_t &port);
    TimerHandle_t _svnSgDynTmrHndl{NULL};
public:
    SevenSegDynamic();
    SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegDynamic();
    virtual bool begin();
    virtual bool stop();
};

//============================================================> Class declarations separator

class SevenSegDynHC595: public SevenSegDynamic{
    static void tmrCbRfrshDynHC595(TimerHandle_t rfrshTmrCbArg);  //Will easily fail in subclasses calls, check it!!

private:
    const uint8_t _sclkIndx {0};
    const uint8_t _rclkIndx {1};
    const uint8_t _dioIndx {2};
    uint8_t _sclk {};
    uint8_t _rclk {};
    uint8_t _dio {};
protected:
    static TimerHandle_t _dynHC595DspRfrshTmrHndl;
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

/*
class SevenSegDynDummy: public SevenSegDynamic{
public:
    SevenSegDynDummy(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDynDummy();
};
*/

//============================================================> Class declarations separator

/*
class SevenSegStatic: public SevenSegDispHw{

public:
    // SevenSegStatic();    //No differentiated default constructor for this class yet!!
    ~SevenSegStatic();
};
*/

//============================================================> Class declarations separator

/*
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
*/

//============================================================> Class declarations separator

/*
class SevenSegStatHC595: public SevenSegStatic{
public:
    SevenSegStatHC595();
    ~SevenSegStatHC595();
};
*/

//============================================================> Class declarations separator

/*
class SevenSegStatDummy: public SevenSegStatic{
public:
    SevenSegStatDummy(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegStatDummy();
};
*/

//============================================================> Class declarations separator

// Classes for the TM1638, Max7219, HT16K33 under implementation need analysis


#endif