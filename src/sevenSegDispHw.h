/**
 ******************************************************************************
 * @file SevenSegDispHw.h
 * @brief Header file for the SevenSegDisplays_ESP32 library, SevenSegDispHw class and subclasses 
 * 
 * Repository: https://github.com/GabyGold67/SevenSegDisplays_ESP32  
 * 
 * Framework: Arduino  
 * Platform: ESP32  
 * 
 * @author Gabriel D. Goldman  
 * mail <gdgoldman67@hotmail.com>  
 * Github <https://github.com/GabyGold67>  
 * 
 * @version 3.0.0
 * 
 * @date First release: 20/12/2023  
 *       Last update:   08/04/2025 09:00 (GMT+0200) DST  
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
  * Parsons Project (c) 1980 Games People play disclaimer:   
  * Games people play, you take it or you leave it  
  * Things that they say aren't alright  
  * If I promised you the moon and the stars, would you believe it?  
 *******************************************************************************
 */
#ifndef sevenSegDispHw_ESP32_H
#define sevenSegDispHw_ESP32_H

#include "Arduino.h"
#include <stdint.h>
#include <ShiftRegGPIOXpander_ESP32.h>

//------- Generic Functions prototypes BEGIN
template<typename T>
void pushElmnt(T* &elmntLstPtr, T ssdToPush, uint8_t &elmntQty);
//--------- Generic Functions prototypes END

//============================================================> Class declarations separator

/**
 * @class SevenSegDispHw
 * 
 * @brief Base abstract class models a generic Seven Segment display hardware
 * 
 * @note This library, from it's base class to the last of it's subclasses, adhere to the following concepts: 
 * **Seven Segment display hardware**: is a hardware construction capable of receiving data in 8-bits units, each unit corresponding to a segment or decimal point of a seven segment display digit, as it is already normalized across the electronic industry. The Seven Segment display is composed of two basic elements:  
 * - The **display controller component**: Is conformed by the electronics receiving the data to be displayed and send the needed signals to turn on the corresponding segments to show the data at the display module. The display controller component might be as autonomous and complex as a specific "display controller chip", as simple as a "SIPO Shift Register" or as "non physical existent" as a "cable connector to the MCU", so that the MCU will have to take care of the data update and display logic, making the MCU the "display controller component". 
 * - The **display module component**: Is conformed by the seven segments led display unit or units if more than one is required. This units include the leds to be lit as segments and decimal point, and some include colons to represent time values, or special icons in very specific use models. Each display module component will have it's internal wiring, the seven segment part pretty standard, the other components wired differently. As for this library the only display module characteristic considered essential is if the digit leds are connected in common anode or common cathode way. This required information is provided by each display module datasheet, the corresponding display module code is usually found printed in one side or at the bottom of the module.  
 * 
 */
class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    bool _commAnode {true}; // SevenSegDisplays objects need this info to build the right segments to represent each character
    uint8_t* _digitPosPtr{nullptr};
    uint8_t* _dspBuffPtr{nullptr};  
    uint8_t _dspDigitsQty{}; // Display size in digits    
    uint8_t _dspHwInstNbr{0};
    SevenSegDispHw* _dspHwInstance{nullptr};
    uint8_t* _ioPins{};

    virtual void _unAbstract() = 0; // Makes this an Abstract class. For the subclasses to be instantiable they'll have to implement the _unAbstract() method.  
    virtual void send(uint8_t* digitsBuffer);
    virtual void send(const uint8_t &segments, const uint8_t &port);
public:
    /**
     * @brief Default class constructor
     * 
     */
    SevenSegDispHw();
    /**
     * @brief Class constructor
     * 
     * @param ioPins A pointer to an array holding the identifieres for the MCU GPIO pins required to send the data to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants for each subclass.  
     * @param dspDigits Quantity of digits/ports of the display. This value is directly related to the **display module component** quantity of ports and characteristics.  
     * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).  
     *      
     * @attention The dspDigits parameter indicating the quantity of digits of the display module is a **basic fundamental** parameter passed at instantiation time. This information provides the value to be used to generate the data buffer, the digits order table, auxiliar buffers and even the required information to generate a valid left or right aligned display. Obviously the quantity is bigger than 0, and must be less than or equal to the hardware maximum display digits capability.  
     */
    SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class destructor
     */
    virtual ~SevenSegDispHw();    
   /**
    * @brief Sets up the hardware display to work, and starts the display activities.  
    * 
    * Depending on the display technology and the resources it needs to work with, this method takes care of those preparations. That means that each specific subclass of display will provide it's own version of `begin()` that will take care of:  
    * - Configuring timers or interrupts.  
    * - Setup tasks and unblocking procedures to get new contents from the SevenSegDisplays object.  
    * - Setup communications parameters.  
    * - Establish communications with the display.  
    * - Other specific services configuration and starting.  
    * 
    * @param updtLps Time lapse between updating activities required, specially by dynamic subclasses. The parameter will be used according to each subclass needs, which will be described in each class begin() method.  
    * 
    * @retval true The specific configurations and startups could be successfully completed.  
    * @return false One or more of the specific configurations or startups failed.  
    */
    virtual bool begin(uint32_t updtLps = 0);
    /**
     * @brief Reverts the begin(uint32_t) actions, stopping the display activities and freeing the resources used by the Seven Segment display hardware object.  
     * 
     * @return true The display activities could be stopped and resources freed with no problems.  
     * @return false The display activities couldn't be stopped. The method failed.  
     */
    virtual bool end();
    /**
     * @brief Returns a value indicating if the display module component uses a common anode or a common cathode led wiring.  
     * 
     * The SevenSegDisplays instantiated objects will compose the values corresponding to each character it can display according to the SevenSegDispHw _commAnode attribute. Each SevenSegDispHw instantiable subclass will have that constant attribute set by the subclass developer to correspond to the technical specifications of the display module component. 
     * 
     * @retval true The display is built with a Common Anode display module component.  
     * @retval false The display is built with a Common Cathode  display module component.  
     */
    bool getCommAnode();
    /**
     * @brief Returns a pointer to the display buffer.  
     * 
     * The display buffer is a memory area set by the SevenSegDisplays class object instantiated to hold the data to be displayed, and so it's shared by that object and the SevenSegDispHw subclass object component. When the SevenSegDisplays object is constructed it sets the SevenSegDispHw subclass object buffer pointer to the memory area it set for that purpose. This method retrieves that memory pointer.  
     * 
     * @return uint8_t* The pointer to the buffer area used by the SevenSegDispHw to get the data to display.  
     * 
     * @warning If getDspBuffPtr() returns a nullptr it means the setDspBuffPtr(uint8_t*) invoked by the SevenSegDisplays constructor failed and the SevenSegDispHw object has no source for the data to be displayed.  
     */
    uint8_t* getDspBuffPtr();
    /**
     * @brief Returns the quantity of digits of the display module.  
     * 
     * The value returned correspond to the dspDigits parameter passed at instantiation time.  
     * 
     * @return uint8_t The **display module component** digits quantity.  
     */
    uint8_t getHwDspDigitsQty();
    /**
     * @brief Notifies the object of a change of content available in the display buffer, the object must update the display.  
     * 
     * While the dynamic displays must periodically refresh the data displayed, and so they get the most updated content from the display buffer without the need to send them any notification, the data displayed by the static displays remains constant even if it's data buffer or registers are loaded with new data. This method notifies the display that the data buffer content has changed and it must load the new contents of it's registers or buffers to display the new information.  
     */
   virtual void ntfyUpdDsply();
   virtual bool setBrghtnssLvl(const uint8_t &newBrghtnssLvl){return true;}; 
   /**
    * @brief Sets a mapping to relate the display buffer positions to the display port asigned to exhibit it's contents.  
    * 
     * As different **Seven Segment display hardware** implement different wiring schemes between the **display controller component** and the **display module component**, some implement the leftmost display port as it's lowest memory position of it's buffer, while some implement the rightmost position to it. When more than one **display module components** are used, it adds a new level of hardware implementation that differs from one supplier to the other. The library implements a mechanism to provide the instantiated object to relate the positions of the display ports to the display buffer positions through an array. The array has the size of the display buffer, and each array elment is meant to hold the number of the corresponding port that is wired to display the data in that display buffer position The array is default defined in the constructor as (0, 1, 2,...) that is the most usual implementation found. If the order needs to be changed the setDigitsOrder() method is the way to set a new mapping.  
     * 
     * @param newOrderPtr Pointer to an uint8_t array of _dspDigits lenght containing the position of the port in the **display module components** wired to display that buffer position content. Each value will be checked against the _dspDigits value to ensure that they are all in the range acceptable, 0 <= value <= _dspDigits - 1. If one of the values is out of the valid range no change will be done. Please note that no checking will be done to ensure all of the array values are different. A repeated value will be accepted, ending in an undetermined non-critic, display behavior.  
     * @return true All of the elements of the array were in the accepted range. The change was performed.  
     * @return false At least one of the values of the array passed were out of range. The change wasn't performed.  
     * 
   * Use example:  
   * @code {.cpp}
   * uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4}; //Builds an array with the port order of the "DIY MORE 8-bit LED Display".
   * myLedDisp.setDigitsOrder(diyMore8Bits); //Changes the display bit to port mapping according to the display characteristics.  
   * @endcode
*/
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
     * @note The dspBuffPtr attribute might be changed to use a temporary different source of data to be displayed. 
     * 
     * @warning Setting the display buffer pointer to an address not coinciding with the one configured in the SevenSegDisplays will **disable** the possibility for it to get new SevenSegDisplays generated content displayed!! Handle with extreme care, first saving the original provided pointer, changing it as needed, and reseting the _dspBuffPtr to the saved value!!
     */
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
    virtual void turnOff(){};
    virtual void turnOn(){};
    virtual void turnOn(const uint8_t &newBrghtnssLvl){};
};

//============================================================> Class declarations separator

/**
 * @class SevenSegDynamic
 * 
 * @brief Abstract class models a generic dynamically updated **Seven Segment display hardware**
 */
class SevenSegDynamic: public SevenSegDispHw{    
    static void tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg);

protected:
    // static TimerHandle_t _dynDspRfrshTmrHndl;
    TimerHandle_t _dynDspRfrshTmrHndl{NULL};
    uint8_t _firstRefreshed{0};
    void _refresh();
    virtual void send(uint8_t content);
    virtual void send(const uint8_t &segments, const uint8_t &port);
    TimerHandle_t _svnSgDynTmrHndl{NULL};
public:
    SevenSegDynamic();
    SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegDynamic();
   /**
    * @brief Sets up the hardware display to work.  
    * 
    * Depending on the display technology and the resources it needs to start working, this method takes care of those preparation. That means that each specific subclass of display will have to provide it's version of `begin()`.  
    * The SevenSegDynamic abstract class and it's subclasses model displays that need regular refreshing of its contents, and for this to happen their `begin()` method must implement and start timers and/or periodic interrupts to handle it's refreshing routines.  
    *   
    * @retval true The specific configurations and startups could be successfully made
    * @return false One or more of the specific configurations or startups failed.  
    * 
    * @note For each SevenSegDynamic instantiable subclass a short description of their respective `begin()` actions will be added if they are relevant to the developer using the library.  
    */
    virtual bool begin(uint32_t updtLps=0);
    /**
     * @brief Reverts the begin(uint32_t) actions, stopping the display activities, stopping and deleting the timer created for periodic refreshing and freeing the resources used by the Seven Segment display hardware object.  
     * 
     * @return true The display activities could be stopped and resources freed with no problems.  
     * @return false The display activities couldn't be stopped. The method failed.  
     */
    virtual bool end();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegDynHC595
 * 
 * @brief Models seven segment displays driven by two 74HC595 shift registers
 * 
 * The display is wired so that one shift register holds the active segments of the display digit, so it is connected in parallel to every digit segment, to the activation pin of the same segment in each one, and the second shift register holds the active digit enabled, so it's pins are connected independently to each digit, selecting which will be active at any given moment.
 * As detailed in the **SevenSegDynamic** abstract class, this wiring arrange requires the display to be refreshed periodically to generate a cinematic effect or animation showing the full contents of all the digits at the same time, while the hardware is capable of lightning on just one at a time. This cinematic effect is also managed by the library.  
 */
class SevenSegDynHC595: public SevenSegDynamic{
    static void tmrCbRfrshDynHC595(TimerHandle_t rfrshTmrCbArg);

private:
    const uint8_t _sclkIndx {0};
    const uint8_t _rclkIndx {1};
    const uint8_t _dioIndx {2};

    ShiftRegGPIOXpander* _drvrShftRegPtr{nullptr};
    uint8_t* _drvrShftRegSndPtr{nullptr};
    uint8_t _sclk {};
    uint8_t _rclk {};
    uint8_t _dio {};
    virtual void _unAbstract();

protected:
    TimerHandle_t _dynHC595DspRfrshTmrHndl{NULL};  

    void _refresh();
    void send(uint8_t content){};
    void send(const uint8_t &segments, const uint8_t &port){};
public:
    /**
     * @brief Class constructor
     * 
     * @param ioPins A pointer to an array holding the identifieres for the 3 GPIO pins required to send the data to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->sclk, 1->rclk, 2->dio
     * @param dspDigits Quantity of digits/ports of the display. This class supports the wiring scheme allowing a maximum of 8 digits.  
     * @param commAnode Boolean indicating if the hardware uses **display module component** wired as common anode (true) or common cathode (false).  
     */
    SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    /**
     * @brief Class destructor
     * 
     */
    ~SevenSegDynHC595();
    /**
     * @brief Sets up the required resources for the hardware display to work
     * 
     * For the Seven Segments Dynamic 74HC595 displays to work several procedures must be completed by this method.  
     * 
     * Attaches the display to the O.S. software timer service, which takes care of refreshing the display regularly. An unlimited amount of displays might be attached to the timer theoretically, as long as there's enough resources available for them, but in practice the refreshing work takes time, and the time taken is proportional to the quantity of displays attached. And as Timers consume time for other tasks done by the microcontroller, the time taken by the timers must be kept to minimal or the stability of the whole system will be compromised. As the time available to execute the refreshing time without risking the stability of the system depends on various factors, the number of supported displays has to be tested in each development environment at development time.
     * @retval true The display could be attached to the software timer service, or if the display was already attached to it. This not ensures system stability.  
     * @retval false the display couldn't be attached to the software timer service, due to O.S. failure.  
     * 
     * Use example:  
     * 
     * @code {.cpp}
     * myLedDisp.begin();
     * @endcode
     */
    virtual bool begin(uint32_t updtLps = 0);
    /**
     * @brief Stops the active display updating.  
     * 
     * Detaches the display from the Software Timer Service which takes care of refreshing the display regularly. To restart de display update timer a new begin() method must be executed.  
     * 
     * @return true The instance of the display was found and detached from the STS.  
     * @return false The instance of the display wasn't found attached to the STS, no detach was carried as it wasn't needed.  
     * 
     * Use example
     * 
     * @code {.cpp}
     * myLedDisp.end();
     * @endcode
     * 
     */
    bool end();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegDynDummy
 * 
 * @brief Models a dynamic display with no screen, for tests or remote display of the data.  
 * 
 * The objects instantiated are usefull for development of code expecting to use a dynamic display while the precise hardware to be used in production is not defined. This is done by sending the data through the MCU UART port, making possible the implementation without depending of a physical display. The refresh rate is a parameter of the `begin(uint32_t)` method, so that it can be adjusted to a reasonable speed, either to reading it in real-time, either to send it to storage.  
 * For each periodic "display refresh" event a message will be transmited through the UART, including: 
 * - A time stamp
 * - The content of each port indicating:  
 *      - The port position as a **decimal**  
 *      - The content for that position as a **hexadecimal** value.  
 */
class SevenSegDynDummy: public SevenSegDynamic{
    static void tmrCbRfrshDynDummy(TimerHandle_t rfrshTmrCbArg);
    
protected:
    // static TimerHandle_t _dynDummyDspRfrshTmrHndl;
    TimerHandle_t _dynDummyDspRfrshTmrHndl{NULL};

    virtual void _unAbstract();
    void _refresh();
    void send(const uint8_t &segments, const uint8_t &port);
public:
    /**
     * @brief Class constructor, instantiates a SevenSegDynDummy object
     * 
     * @param dspDigits Quantity of digits/ports of the display. As this is a software only "display" the value is just limited by the parameter type to 256 digits.  
     * @param commAnode Boolean indicating if the hardware display/s module/s being simulated are supposed to be wired as common anode (true) or common cathode (false).  
     */
    SevenSegDynDummy(uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class destructor  
     */
    ~SevenSegDynDummy();
    /**
     * @brief Sets up the required resources for the "dummy hardware display" to work
     * 
     * For the Seven Segments Dynamic Dummy displays to work several procedures must be completed by this method, specially to ensure a behaviour as close to a hardware dynamic display.  
     * 
     * Attaches the display to the O.S. software timer service, which takes care of refreshing the display regularly. An unlimited amount of displays might be attached to the timer theoretically, as long as there's enough resources available for them, but in practice the refreshing work takes time, and the time taken is proportional to the quantity of displays attached. And as Timers consume time for other tasks done by the microcontroller, the time taken by the timers must be kept to minimal or the stability of the whole system will be compromised. As the time available to execute the refreshing time without risking the stability of the system depends on various factors, the number of supported displays has to be tested in each development environment at development time.
     * @retval true The display could be attached to the software timer service, or if the display was already attached to it. This not ensures system stability.  
     * @retval false the display couldn't be attached to the software timer service, due to O.S. failure.  
     * 
     * @param updtLps (Optional) Time lapse given in milliseconds for the "display refresh" task to be executed. If no value is provided a time of 2000 milliseconds (2 seconds) will be used. 
     * 
     * @return true 
     * @return false 
     */
    bool begin(uint32_t updtLps = 0);
    /**
     * @brief Stops the active display updating.  
     * 
     * See SevenSegDynHC595::end() for details.  
     */
    bool end();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegStatic
 * 
 * @brief Abstract class that models displays that don't need permanent MCU data updating to keep the data correctly displayed. 
 * 
 * The lack of need of periodic intervention from the MCU implies that a "display driver" chip (or chipset) takes care of the display update and refreshing. So this is a base class for display through those display drivers. The MCU sends the data to be displayed and the chip required commands through diverse channels and protocols depending on the specific chip.  
 * The chip then connects to the display module through their output pins. The hardware developer might decide to implement the chip/display module wiring according to the project requirements, but all the chips modeled by this class and it's subclasses share the same characteristics:  
 * - They provide 8 pins for the seven segments + DP ports.  
 * - They provide 1 pin per digit/port supported by the specific chip.  
 * 
 * @note **NONE** of the **driver chips** controlling the displays modeled by the SevenSegStatic subclasses includes **COLONS**, **ICONS** or any other amenity some hardware display modules include. The activation of those **colons** and/or **icons** is provided by the use of some of the existing described chip pins in a display module propietary exclusive way, and are described in those display modules' datasheets. Some of those mechanisms are:  
 * - Have less display ports than the maximum supported by the chip, and use one or more segments of the exceeding ports wired to the colon/s or icon/s of the display.  
 * - Use an external source to activate the colon/s or icon/s independently from the driving chip.  
 * - Wire **one or more** of the visible display ports DP segments to the colon/s or icon/s of the display.  
 * 
 * @warning Using displays that implement the colon/s and/or icon/s through the DP segments of the active ports make the display unfit to display decimal non integer values, as no DP might be used. Verify the display module characteristics to setup the corresponding class with the right parameters.  
 */
class SevenSegStatic: public SevenSegDispHw{
    
public:
    SevenSegStatic();
    SevenSegStatic(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegStatic();
    virtual void ntfyUpdDsply();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegStatHC595
 * 
 * @brief Models displays driven by 74HC595 or similar shift registers, one shift register per display port, wired so that the eight output pins of each shift register is connected to the 8 segment pins of the display module.  
 * 
 * For more than one digit displays, the shift registers driving each port is connected to the next in the traditional **daisy-chain** fashion, using the cascading pins to connect the shift register to the next in the chain.  
 */
class SevenSegStatHC595: public SevenSegStatic{
private:
    const uint8_t _sclkIndx {0};
    const uint8_t _rclkIndx {1};
    const uint8_t _dioIndx {2};

    ShiftRegGPIOXpander* _dsplyHwShftRegPtr{nullptr};
    uint8_t* _lclDspBuffPtr{nullptr};
    uint8_t _sclk {};
    uint8_t _rclk {};
    uint8_t _dio {};

    virtual void _unAbstract();
    void _updDsplyCntnt();
public:
    /**
     * @brief Class default constructor.  
     */
    SevenSegStatHC595();
    /**
     * @brief Class constructor.  
     * 
     * @param ioPins A pointer to an array holding the identifieres for the 3 GPIO pins required to send the data to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->sclk, 1->rclk, 2->dio
     * @param dspDigits Quantity of digits/ports of the display. This class supports the wiring scheme allowing a maximum of 8 digits.  
     * @param commAnode Boolean indicating if the hardware uses a display/s module/s wired as common anode (true) or common cathode (false).  
     */
    SevenSegStatHC595(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class destructor
     */
    ~SevenSegStatHC595();
    /**
     * @brief See SevenSegDispHw::ntfyUpdDsply() for description
     */
    virtual void ntfyUpdDsply();
};

//============================================================> Class declarations separator

/**
 * @brief Models Seven Segments LEDs static displays hardware controlled by Titan Micro TM163X series chips
 *
 * As TM163X series chips have some differences among them, this class implements the base common characteristics, the differences are implemented in corresponding subclasses.
 *
 * Common attributes include:
 * - Communications protocol (a non standard variation of the I2C protocol).
 * - Commands structure.
 * - Read/Write commands.
 * - Brightness commands, capabilities and values.
 * - Start and Stop commands.
 *
 * Different attributes include:
 * - Maximum number of ports addressable.
 * - Keyscanning services
 *
 * @note As the communications protocol doesn't completely comply with the I2C protocol, the communications must be implemented in software. For that reason, for resources saving sake, the CLK speed will be reduced from the data sheet **Maximum clock frequency** stated as 500KHz to a less demanding 100KHz(10 microseconds) time slices, managed by `delayMicroseconds()` function keyword, or a timer interrupt set at 100KHz, enabling the timer interrupt service only while transmitting data, and disabling it while idle. 
 * 
 * @warning While the TM1636, TM1637, TM1638, TM1639 (at least these are our most known members of this "family") are stable and well documented devices, the parts sold as **"TM1637 Display Modules"**, **"TM1638 Display Modules"** etc, breakboards that include the TM163X display driver, supporting electronics and one or several 7 segments display modules are not all created equal. A well known example: having the TM1637 modules the hability to drive 6 display ports, some breakboards present 4 display ports and a **center colon**, as is standard to time displaying modules. The **big issue** is the lack of a standard for those display modules, not all displays have the same disposition, not all of them are internally wired the same, and that not all the manufacturers wire the TM1637 modules to the 7 segments display modules in the same way. For example, some will attach the colon to the DP (decimal point) segment of the third port (RtL), some will attach them to ALL the DP segment, some will attach each of the dots of the colon independently, one to the 5th display port, the other to the 6th display port, and then some other manufacturer in some other way.  
 * Whenever is possible to get a specific module for testing, a SevenSegTM163X subclass might be added to manage it correctly, please read the subclasses' description for correct display module oriented class identification.  
 *
 * @class SevenSegTM163X
 */
class SevenSegTM163X: public SevenSegStatic{
    static uint8_t _usTmrUsrs;
private:
    const uint8_t _clkIndx {0};
    const uint8_t _dioIndx {1};
    const uint8_t _dspDigitsQtyMax{}; // Maximum display size in digits: 4 for TM1636, 6 for TM1637, 16 for TM1639
    const uint8_t _hwBrghtnssLvlMax{0x07};
    const uint8_t _hwBrghtnssLvlMin{0x00};
    uint32_t _txClkTckTm{2};

    uint8_t _clk {};
    uint8_t _dio {}; 
    uint8_t* _lclDspBuffPtr{nullptr};

    void _updDsplyCntnt();
protected:
    uint8_t _brghtnssLvl{};
    uint8_t _brghtnssLvlMax{};
    uint8_t _brghtnssLvlMin{};
    bool _isOn{false};
    uint8_t* _msgBffrPtr{nullptr};
    uint8_t _mssgBffrLngth{0};
 
    void _txStart();
    void _txAsk();
    void _txStop();
    void _txWrByte(uint8_t data);
    virtual void _sendBffr();
 
 public:
    /**
     * @brief Default class constructor
     */
    SevenSegTM163X();
    /**
     * @brief Class constructor
     * 
     * @param ioPins A pointer to an array holding the identifieres for the 2 GPIO pins required to send the data to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->dio
     * @param dspDigits 
     */
    SevenSegTM163X(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    /**
     * @brief Class destructor
     */
    ~SevenSegTM163X();
    /**
     * @brief Turns On the display to be ready to receive data.  
     * 
     * Turning on a TM163X display implies sending a command to the display and saving in an object attribute the isOn state, as the display controller does not provide any means to read it's state. 
     * 
     * @return true Always  
     */
    bool begin();
    /**
     * @brief Turns Off the display and registers the new state in the correspondig object's attribute.  
     * 
     * @return true Always  
     */
    bool end();
    /**
     * @brief Returns the current brightness level setting for the display module.  
     * 
     * The TM163X series display drivers have the capability of changing the led display brightness level by using PWM on it's output. The resulting brightness levels are not percieved as linear, and the minimum and maximum brightness values don't reach the levels of totaly turning the display off, neither turning the display to it's maximum possible brightness.
     * 
     * @note The SevenSegTM163X abstract class is instrumented so that any subclass must incorporate the minimum and maximum values for that specific display subclass. All the members of the TM163X family I could check at this point share the minimum and the maximum brightness values: 0 for the minimum, 7 for the maximum, resulting in 8 brightness levels. But is not taken for granted. See setBrghtnssLvl(const uint8_t &) for more details.  
     * 
     * @return The current brightness level setting.  
     */
    uint8_t getBrghtnssLvl();
    /**
     * @brief Returns the maximum brightness level for the instantiated object.  
     * 
     * The value returned is the maximum brightness level for the object's class.  
     * 
     * @return The uint8_t value of the maximum brightness setting available for the object's class.  
     */
    uint8_t getBrghtnssMaxLvl();
    /**
     * @brief Returns the minimum brightness level for the instantiated object.  
     * 
     * The value returned is the minimum brightness level for the object's class.  
     * 
     * @return The uint8_t value of the minimum brightness setting available for the object's class.  
     */
    uint8_t getBrghtnssMinLvl();
    /**
     * @brief See SevenSegDispHw::ntfyUpdDsply() for description
     */
    virtual void ntfyUpdDsply();
    /**
     * @brief Sets the Brghtness level of the display.  
     * 
     * @param newBrghtnssLvl The new brightness level for the display. The value must be in the range **getBrghtnssMinLvl() <= newBrghtnssLvl <= getBrghtnssMaxLvl()**
     * 
     * @return true The parameter was in the acceptable range, the display will be set to the new brightness level (or keep it's brightness level if the parameter passed is equal to the current brightness level) 
     * @return false The parameter was outside the acceptable range, no brightness level changes will be done.  
     */
    virtual bool setBrghtnssLvl(const uint8_t &newBrghtnssLvl); 
    /**
     * @brief Turns the display module off.  
     * 
     * The display module will be cleared and will keep that status until a turnOn(), or turnOn(const uint8_t &) is invoked. 
     */
    virtual void turnOff();
    /**
     * @brief Turns the display module on.  
     * 
     * The display module will be turned on and it's content displayed, and will keep that status until a turnOff() is invoked.  
     */
    virtual void turnOn();
    
    virtual void turnOn(const uint8_t &newBrghtnssLvl);
};
 
 //============================================================> Class declarations separator
class SevenSegTM1637: public SevenSegTM163X{
private:
    virtual void _unAbstract();
public:
    SevenSegTM1637(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegTM1637();
};

 //============================================================> Class declarations separator
 /*
 class SevenSegTM1639: public SevenSegTM163X{
 protected:
     const uint8_t _dspDigitsQtyMax{16}; // Maximum display size in digits, hardware dependent
 public:
     SevenSegTM1639(gpioPinId_t* ioPins, uint8_t dspDigits);
     ~SevenSegTM1639();
 
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

// Classes for the TM1638, Max7219, HT16K33, direct MPU pin connection, under implementation need analysis


#endif