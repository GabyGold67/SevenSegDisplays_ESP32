/**
 ******************************************************************************
 * @file SevenSegDispHw.h
 * @brief Header file for the SevenSegDisplays_ESP32 library, SevenSegDispHw class and subclasses 
 * 
 * Repository: https://github.com/GabyGold67/SevenSegDisplays_ESP32  
 * 
 * Framework: Arduino  
 * Platform: ESP32  
 * Check https://github.com/GabyGold67 for other Frameworks and Platforms versions of this library availability.  
 * 
 * @author Gabriel D. Goldman  
 * mail <gdgoldman67@hotmail.com>  
 * Github <https://github.com/GabyGold67>  
 * 
 * @version 3.2.0
 * 
 * @date First release: 20/12/2023  
 *       Last update:   08/05/2025 10:40 (GMT+0200) DST  
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
private:
   virtual void _unAbstract() = 0; // Makes this an Abstract class. For the subclasses to be instantiable they'll have to implement the _unAbstract() method.  

protected:
   uint8_t* _ioPins{};
   uint8_t* _digitPosPtr{nullptr};
   uint8_t _dspDigitsQty{}; // Display size in digits    
   bool _commAnode {true}; // SevenSegDisplays objects need this info to build the right segments to represent each character
	const uint8_t _dspDigitsQtyMax{0};

   uint8_t _allLedsOff{};  //!< Value to set in the display port to set all leds off (a "space"), dependent of _commAnode
   uint8_t _brghtnssLvl{0};  //!< Current display brightness level
   uint8_t _brghtnssLvlMax{0};   //!< Maximum display brightness level
   uint8_t _brghtnssLvlMin{0};   //!< Minimum display brightness level
   uint8_t* _dspBlankBuffPtr{nullptr}; //!< Pointer to a display buffer filled with _allLedsOff ("spaces") to use as display buffer while in "Off State"
   uint8_t* _dspBuffPtr{nullptr};   //!< Pointer to the display buffer, will be provided by the SevenSegDisplays object when it's instantiated
   uint8_t* _dspBuffPtrBkp{nullptr};  //!< Pointer to the display buffer, copy of the original **_dspBuffPtr** to be used as backup
   SevenSegDispHw* _dspHwInstance{nullptr};
   uint8_t _dspHwInstNbr{0};
   bool _isOn{false};   //!< Current display status: On/Off
   uint8_t* _xcdDspBuffPtr{nullptr};    //!< A pointer to a buffer the size of the exceeding digits used to control display specific amenities.
   uint8_t _xcdDspDigitsQty{};  //!<  Number of unused available display ports, its the difference  (_dspDigitsQtyMax  - _dspDigitsQty), being the size of the array pointed by _xcdDspBuffPtr

public:
   /**
     * @brief Default class constructor
     */
   SevenSegDispHw();
   /**
     * @brief Class constructor
     * 
     * @param ioPins A pointer to an array holding the identifiers for the MCU GPIO pins required to send the data to be displayed to the **Display controller**. The correlation between the array positions and the pin function is given as in-class defined constants for each instantiable subclass.  
     * @param dspDigits Quantity of digits/ports of the display. This value is directly related to the **display module component** quantity of ports and characteristics.  
     * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).  
     * @param dspDigitsQtyMax Maximum quantity of digits/ports the display controller is able to manage. Sometimes **display module components** are attached to **display controller component** able to manage more than the display digits/ports.  
     * 
     * @note  In the case the there are remaining display controller unused positions, they might be reasigned for special uses or artifacts, see the complete documentation for access to those positions.  
     *      
     * @attention The dspDigits parameter, indicating the quantity of digits of the display module, is a **basic fundamental** parameter passed at instantiation time. This information provides the value to be used to create the data buffer of the right size, the digits order table, auxiliary buffers and the required information to calculate the range of values displayable and to generate a valid left or right aligned display. Obviously the quantity is bigger than 0, and must be less than or equal to the hardware maximum display digits capability.  
     * 
     * @warning The dspDigits parameter value must not be confused with the maximum display digits a display controller component can handle. Many displays are designed using part of the controller's digits management for real seven segment displays digits while using non connected to ports pins to manage some other display elements, like individual leds, icons, special backlighting elements, etc. The class instantiated object needs the right amount of display digits available as such, the exceeding digits/ports is currently out of the libray management scope.  
     */
   SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true, uint8_t dspDigitsQtyMax = 0);
   /**
     * @brief Class destructor
     */
   virtual ~SevenSegDispHw();    
   /**
    * @brief Sets up the hardware display to work, and starts the display activities.  
    * 
    * Depending on the display technology and the resources it needs to work with, this method takes care of those preparations. That means that each specific subclass of display will provide it's own version of `begin()` that will take care of:  
    * - Configuring timers or interrupts required.  
    * - Setup tasks and unblocking procedures to get new contents from the SevenSegDisplays object.  
    * - Setup communications parameters and establish communications with the display hardware.  
    * - Other specific services configuration and setups.  
    * - Turn On the display.  
    * 
    * @param updtLps Time lapse between updating activities required, specially by dynamic subclasses. The parameter will be used according to each subclass needs, which will be described in each class begin() method.  
    * 
    * @retval true The specific configurations and startups could be successfully completed.  
    * @return false One or more of the specific configurations or startups failed.  
    */
   virtual bool begin(uint32_t updtLps = 0);
   /**
     * @brief Reverts the begin(uint32_t) actions, stopping the display activities, turning the display off and freeing the resources used by the Seven Segment display hardware object.  
     * 
     * @return true The display activities could be stopped and resources freed with no problems.  
     * @return false The display activities couldn't be stopped. The method failed.  
     * 
     * @note The `end()` method does not destroy the object, and it's effect can be reverted by invoking a new `begin(uint32_t)` method for the object.  
     */
   virtual bool end();
   /**
    * @brief Returns the current display brightness level setting for the display.  
    * 
    * For displays with variable display brighness levels this method will return the current brightness level. The returned value will be in the range **getBrghtnssMinLvl() <= getBrghtnssLvl() <= getBrghtnssMaxLvl()**, see setBrghtnssLvl(const uint8_t &) for more details.  
    * 
    * @note For display classes that models object without the capacity of changing brightness levels, getBrghtnssMinLvl() = getBrghtnssLvl() = getBrghtnssMaxLvl() = 0, this implies that using (getBrghtnssMinLvl() == getBrghtnssMaxLvl()) condition is enough to identify if a given display has or has not the capacity of changing brightness levels.  
    * 
    * @return The current display brightness level setting, as long as the display has the capacity of changing brightness levels.  
    */
   virtual uint8_t getBrghtnssLvl();
   /**
    * @brief Returns the maximum display brightness level setting allowed for the display.  
    * 
    * For displays with variable display brightness levels this method will return the maximum brightness level value valid setting. See setBrghtnssLvl(const uint8_t &) for more details.  
    * 
    * @note For display classes that models object without the capacity of changing brightness levels, getBrghtnssMinLvl() = getBrghtnssLvl() = getBrghtnssMaxLvl() = 0, this implies that using (getBrghtnssMinLvl() == getBrghtnssMaxLvl()) condition is enough to identify if a given display has or has not the capacity of changing brightness levels.  
    * 
    * @return The display brightness level maximum value setting as long as the display has the capacity of changing brightness levels.  
    */
   virtual uint8_t getBrghtnssMaxLvl();
   /**
    * @brief Returns the minimum display brightness level setting for the display.  
    * 
    * For displays with variable display brightness levels this method will return the minimum brightness level value setting. See setBrghtnssLvl(const uint8_t &) for more details.  
    * 
    * @note For display classes that models object without the capacity of changing brightness levels, getBrghtnssMinLvl() = getBrghtnssLvl() = getBrghtnssMaxLvl() = 0, this implies that using `(getBrghtnssMinLvl() == getBrghtnssMaxLvl())` condition is enough to identify if a given display has or has not the capacity of changing brightness levels.  
    * 
    * @return The display brightness level minimum value setting, as long as the display has the capacity of changing brightness levels.  
    */
   virtual uint8_t getBrghtnssMinLvl();
   /**
     * @brief Returns a value indicating if the **display module component** of the display hardware uses a common anode or a common cathode led wiring.  
     * 
     * The SevenSegDisplays instantiated objects will compose the values corresponding to each character it displays according to the SevenSegDispHw _commAnode attribute. Each SevenSegDispHw instantiable subclass will have that constant attribute set by the subclass developer to correspond to the technical specifications of the display module component, unless the display module has the capacity to work with both technologies, in wich case the subclass constructor will include a _commAnode boolean parameter to set that attribute.  
     * 
     * @retval true The display is built with a Common Anode display module component.  
     * @retval false The display is built with a Common Cathode  display module component.  
     */
   bool getCommAnode();
   /**
     * @brief Returns a pointer to the display buffer.  
     * 
     * The display buffer is the memory area set by the SevenSegDisplays class object instantiated to hold the data to be displayed, and so it's shared by that object and it's SevenSegDispHw subclass object component. When the SevenSegDisplays object is constructed it sets the SevenSegDispHw subclass object buffer pointer to the memory area it set for that purpose. This method retrieves that memory pointer.  
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
     * 
     * @warning The dspDigits parameter value return by this method must not be confused with the maximum display digits a display controller component can handle. For more information see SevenSegDispHw(uint8_t*, uint8_t, bool commAnode)
     */
   uint8_t getHwDspDigitsQty();
   /**
     * @brief Returns the state of the display.  
     * 
     * Returns a boolean value indicating if the display is turned On or Off
     * 
     * @retval true The display is turned On
     * @retval false The display is turned Off 
     */
   virtual bool getIsOn();
   /**
    * @brief Returns a pointer to an array holding the contents for the digits/ports available from the display controller but not used for plain digits display.  
    * 
    * @return uint8_t* The pointer to the array holding the contents for the exceeding display controller ports.  
    */
   uint8_t* getxcdDspBuffPtr();
   /**
     * @brief Notifies the SevenSegDispHw component object of a change of content available in the display buffer, so that the object proceeds to update the display.  
     * 
     * While the dynamic displays must periodically refresh the data displayed, and so they get the most updated content from the display buffer without the need to receive any notification, the data displayed by the static displays internal buffer to hold the displayed data. The internal buffer contents remain constant until it's data buffer or registers are loaded with new data. This method notifies the display that the data buffer content has changed and it must load the new contents of it's internal registers or buffers to display the new information.  
     * 
     * @note The usual possibilities to work in these cases are two: exception notification or constant polling. The exception notification mechanism has been selected for economy of resources and usual considerations of similar cases.  
     */
   virtual void ntfyUpdDsply();
   /**
    * @brief Sets the brightness level for the display.  
    * 
    * Although dummy dynamic and dummy static displays brightness might be dimmed using PWM management, both displays require MCU intervention, added to the regular required cinematic effect generation workload, making the brightness control very expensive in resources use. For this reason the brightness control in this library will be available for the **smart static** displays equiped with a display controller unit specifically providing the option. Each class description and documentation will detail if the service is available or not, and in case the method is invoked in an object instantiated from a class with no brightness control capabilities, the value returned will be false, avoiding the need of a error handling mechanism.  
    * 
    * @param newBrghtnssLvl The new brightness level for the display. The value must be in the range **getBrghtnssMinLvl() <= newBrghtnssLvl <= getBrghtnssMaxLvl()**
    * 
    * @retval true The parameter was in the valid range, the display will be set to the new brightness level (or keep it's brightness level if the parameter passed is equal to the current brightness level) 
    * @return false The display module does not support brightness level setting, or the parameter passed is outside the valid range, no brightness level changes could be made.  
    */
   virtual bool setBrghtnssLvl(const uint8_t &newBrghtnssLvl); 
   /**
    * @brief Sets a mapping to relate the display buffer positions to the display port assigned to exhibit it's contents.  
    * 
     * As different **Seven Segment display hardware** implement different wiring schemes between the **display controller component** and the **display module component**, some implement the leftmost display port as it's lowest memory position of it's buffer, while some implement the rightmost position to it. When more than one **display module components** are used, it adds a new level of hardware implementation that differs from one supplier to the other. The library implements a mechanism to provide the instantiated object to relate the positions of the display ports to the display buffer positions through an array. The array has the size of the display buffer, and each array element is meant to hold the number of the corresponding port that is wired to display the data in that display buffer position The array is default defined in the constructor as (0, 1, 2,...) that is the most usual implementation found. If the order needs to be changed the setDigitsOrder() method is the way to set a new mapping.  
     * 
     * @param newOrderPtr Pointer to an uint8_t array of _dspDigits length containing the position of the port in the **display module components** wired to display that buffer position content. Each value will be checked against the _dspDigits value to ensure that they are all in the range acceptable, 0 <= value <= _dspDigits - 1. If one of the values is out of the valid range no change will be done. Please note that no checking will be done to ensure all of the array values are different. A repeated value will be accepted, ending in an undetermined non-critic, display behavior.  
     * @return true All of the elements of the array were in the accepted range. The change was performed.  
     * @return false At least one of the values of the array passed were out of range. The change wasn't performed.  
     * 
   * Use example:  
   * @code {.cpp}
   * const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4}; //Builds an array with the port order of the "DIY MORE 8-bit LED Display".
   * const uint8_t stdLtoRx4 [4] {0, 1, 2, 3};  // Simple left to right 4 digits order
   * const uint8_t stdRtoLx4 [4] {3, 2, 1, 0};  // Simple right to left 4 digits order (default)
   * 
   * myLedDisp.setDigitsOrder(diyMore8Bits); //Changes the display bit to port mapping according to the display characteristics.  
   * @endcode
   * 
*/
   virtual bool setDigitsOrder(uint8_t* newOrderPtr);
    /**
     * @brief Sets the pointer to the Display Buffer memory area.  
     * 
     * When a **SevenSegDisplays** object is instantiated it's constructor sets a display buffer memory area to store the contents ready to be displayed. Part of the constructor execution includes passing to the SevenSegDispHw subclass component that pointer, as the underlying hardware display object will be taking the information to display from that memory buffer. The resource to set the pointer is this method. 
     * 
     * @return uint8_t* The pointer to the **Display Buffer Memory Area**.  
     * 
     * @attention Using this method is a resource to generate "animations" by changing the memory area from with the hardware displays takes it's contents, to some other area with ready to display information.  
     * 
     * @note The dspBuffPtr attribute might be changed to use a temporary different source of data to be displayed.  
     * 
     * @warning Setting the display buffer pointer to an address not coinciding with the one configured in the SevenSegDisplays will **disable** the possibility for it to get new SevenSegDisplays generated content displayed!! Handle with extreme care, first saving the original provided pointer, changing it as needed, and reseting the _dspBuffPtr to the saved value!!
     */
   void setDspBuffPtr(uint8_t* newDspBuffPtr);
   /**
    * @brief Turns the display module off.  
    * 
    * The display module will be cleared and will keep that status until a turnOn(), or turnOn(const uint8_t &) is invoked.  
    * 
    * @note Turning the display Off is not the same as clearing the display -see clear() method- as clearing the display implies changing the display data buffer content to fill it with spaces, while turning it off implies keeping the display data buffer updated, while showing the display leds turned off. Some of the display modules managed by display controllers have the hability to turn off the leds display while keeping it's buffer contents unmodified. So turning On/Off those displays will have the effect of holding the data displayed and even receiving and filling their buffer with updated data while keeping their display with no leds turned on.  
    * 
    * @todo For the seven segment displays without controller components to implement On and Off commands, the class will provide a propietary solution to achieve similar features. While there are not implemented the turnOff(), turnOn() and turnOn(const uint8_t &) will be ignored and will produce no effect in the object behavior. 
    */
    virtual void turnOff();
   /**
    * @brief Turns the display module on.  
    * 
    * The display module will be turned on, making visible the contents of the display buffer. For more information see turnOff() method.   
    */
   virtual void turnOn();
   /**
    * @brief Turns the display module on set at the provided brightness level.  
    * 
    * For correct execution and visual response, this method executes a setBrghtnssLvl(const uint8_t &) method followed by a turnOn() method execution.  
    * 
    * @warning If the setBrghtnssLvl(const uint8_t &) method returns false indicating it has failed, this method will go on invoking the turnOn() method instead of failing altogether, as it gives higher priority to the turning on of the display than to the brightness level setting. If the developer is not ok with this scheme he can always use the two methods independently in the sequence he decides better fits his needs and act as his own criteria dictates after the setBrghtnssLvl(const uint8_t &) fails.  
    * 
    * @param newBrghtnssLvl The new brightness level for the display. The value must be in the range **getBrghtnssMinLvl() <= newBrghtnssLvl <= getBrghtnssMaxLvl()**
    */
   virtual void turnOn(const uint8_t &newBrghtnssLvl);
};

//============================================================> Class declarations separator

/**
 * @class SevenSegDynamic
 * 
 * @brief Abstract class models a generic dynamically updated **Seven Segment display hardware**
 */
class SevenSegDynamic: public SevenSegDispHw{    
   static void tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg);
    
private:
   virtual void _unAbstract() = 0; // Makes this an Abstract class.

protected:
   TimerHandle_t _dynDspRfrshTmrHndl{NULL};
   uint8_t _firstRefreshed{0};
   String _rfrshTmrName{""};

   void _refresh();

public:
   SevenSegDynamic();
   SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspDigitsQtyMax = 0);
   virtual ~SevenSegDynamic();
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
   const uint8_t _dspDigitsQtyMax{8};
   TimerHandle_t _dynHC595DspRfrshTmrHndl{NULL};  

   void _refresh();

public:
    /**
     * @brief Class constructor
     * 
     * @param ioPins A pointer to an array holding the identifiers for the 3 GPIO pins required to send the data to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->sclk, 1->rclk, 2->dio
     * @param dspDigits Quantity of digits/ports of the display. This class supports the wiring scheme allowing a maximum of 8 digits.  
     * @param commAnode Boolean indicating if the hardware uses **display module component** wired as common anode (true) or common cathode (false).  
     */
    SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    /**
     * @brief Class destructor
     * 
     */
    virtual ~SevenSegDynHC595();
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
    * Stops the display Timer Service which takes care of refreshing the display regularly. The method will not destroy the timer nor the timerHandle to work faster if a begin(uint32_t) is invoked again.
    * 
    * @return true The instance of the display was found and it's timer stopped.  
    * @return false The instance of the display wasn't found attached to a running software timer, no action was carried out as it wasn't needed.  
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
 * For each periodic "display refresh" event a message will be transmitted through the UART, including: 
 * - A time stamp
 * - The content of each port indicating:  
 *      - The port position as a **decimal**  
 *      - The content for that position as a **hexadecimal** value.  
 */
class SevenSegDynDummy: public SevenSegDynamic{
   static void tmrCbRfrshDynDummy(TimerHandle_t rfrshTmrCbArg);
    
private:    
   virtual void _unAbstract();

protected:
   // static TimerHandle_t _dynDummyDspRfrshTmrHndl;
   TimerHandle_t _dynDummyDspRfrshTmrHndl{NULL};

   void _refresh();
   virtual void send(const uint8_t &segments, const uint8_t &port);

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
    virtual ~SevenSegDynDummy();
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

    virtual void turnOff();

    virtual void turnOn();

};

//============================================================> Class declarations separator

/**
 * @class SevenSegStatic
 * 
 * @brief Abstract class that models displays that don't need permanent MCU data updating to keep the data correctly displayed. 
 * 
 * The lack of need of periodic intervention from the MCU implies that the **display controller component** includes a "display driver" chip (or chipset) that takes care of the display update and refreshing. So this is a base class for Seven **Segment display hardware** based on different **display controller component** chips. 
 * The MCU sends the data to be displayed and the chip required commands through diverse channels and protocols depending on the specific chip.  
 * The chip then connects to the **display module component** through their output pins. The hardware developer might decide to implement the **display module component** wiring according to the project requirements, but all the chips modeled by this class and it's subclasses share the same characteristics:  
 * - They provide 8 pins for the seven segments + DP ports.  
 * - They provide 1 pin per digit/port supported by the specific chip.  
 * 
 * @note **NONE** of the **driver chips** controlling the displays modeled by the SevenSegStatic subclasses includes **COLONS**, **ICONS** or any other amenity some **display module component** includes, this class and it's subclasses model generic use chip drivers for seven segment display modules. For these chips the activation of those **colons** and/or **icons** is provided by the use of some of the existing described chip pins in a display module proprietary exclusive way, and are described in those display modules' datasheet. Some of those mechanisms are:  
 * - Have less display ports than the maximum supported by the chip, and use one or more segments of the exceeding ports wired to the colon/s or icon/s of the display.  
 * - Use an external source to activate the colon/s or icon/s independently from the driving chip.  
 * - Wire **one or more** of the visible display ports DP segments to the colon/s or icon/s of the display.  
 * 
 * @warning Using displays that implement the colon/s and/or icon/s through the DP segments of the active ports make the display unfit to display decimal non integer values, as no DP might be used. Verify the display module characteristics to setup the corresponding class with the right parameters.  
 */
class SevenSegStatic: public SevenSegDispHw{
private:
   virtual void _unAbstract() = 0; // Makes this an Abstract class.  

public:
   SevenSegStatic();
   SevenSegStatic(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
   virtual ~SevenSegStatic();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegStatHC595
 * 
 * @brief Models displays driven by 74HC595 or similar shift registers, one shift register per display port, wired so that the eight output pins of each shift register is connected to the 8 segment pins of the corresponding display module.  
 * 
 * For more than one digit displays, the shift registers driving each port is connected to the next in the traditional **daisy-chain** fashion, using the cascading pins to connect the shift register to the next in the chain.  
 * 
 * @note This class uses ShiftRegGPIOXpander objects defined in the ShiftRegGPIOXpander_ESP32 library available from the Arduino Library Manager. You can find the last version and extensive documentation at it's Github repository https://github.com/GabyGold67/ShiftRegGPIOXpander_ESP32 .  
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
     * @param ioPins A pointer to an array holding the identifiers for the 3 GPIO pins required to send the data to be displayed to the **display controller component**. The correlation between the array positions and the pin function is given as in-class defined constants: 0->sclk, 1->rclk, 2->dio
     * @param dspDigits Quantity of digits/ports of the display. This class supports the wiring scheme allowing a maximum of 8 digits per shift register composing the **display controller component**, up to the 256 limit imposed by the parameter data type **uint8_t**.  
     * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).  
     */
    SevenSegStatHC595(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class destructor
     */
    virtual ~SevenSegStatHC595();
    /**
     * @brief See SevenSegDispHw::ntfyUpdDsply() for description
     */
    virtual void ntfyUpdDsply();
};

//============================================================> Class declarations separator

/**
 * @brief Abstract class that models Seven Segments LEDs static displays hardware using Titan Micro TM163X series chips as **display controller component**
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
 * - Keyscaning services
 *
 * @note As the communications protocol doesn't completely comply with the I2C protocol, the communications must be implemented in software. For that reason, for resources saving sake, the CLK speed will be reduced from the data sheet **Maximum clock frequency** stated as 500KHz to a less demanding time slices, managed by `delayMicroseconds()` function keyword, (or a timer interrupt set at 100KHz for other implementations, enabling the timer interrupt service only while transmitting data, and disabling it while idle).  
 * 
 * @warning While the TM1636, TM1637, TM1639 (at least these are our most known members of this "family" that includes other chips) are stable and well documented devices, the parts sold as **"TM1637 Display Modules"**, **"TM1638 Display Modules"** etc, breakboards that include the TM163X display driver, supporting electronics and one or several 7 segments display modules are not all created equal. A well known example: having the TM1637 modules the ability to drive 6 display ports, some breakboards present 4 display ports and a **center colon**, as is standard to time displaying modules. The **big issue** is the lack of a standard for those display modules, not all displays have the same disposition, not all of them are internally wired the same, and that not all the manufacturers wire the TM1637 modules to the 7 segments display modules in the same way. For example, some will attach the colon to the DP (decimal point) segment of the third port (RtL), some will attach them to ALL the DP segment, some will attach each of the dots of the colon independently, one to the 5th display port, the other to the 6th display port, and then some other manufacturer in some other way.  
 * Whenever is possible to get a specific module for testing, a SevenSegTM163X subclass might be added to manage it correctly, please read the subclasses' description for correct display module oriented class identification.  
 *
 * @class SevenSegTM163X
 */
class SevenSegTM163X: public SevenSegStatic{
private:
   const uint8_t _clkIndx {0};
   const uint8_t _dioIndx {1};
   const uint8_t _hwBrghtnssLvlMax{0x07};
   const uint8_t _hwBrghtnssLvlMin{0x00};
   uint32_t _txClkTckTm{2};
   uint8_t _clk {};
   uint8_t _dio {}; 

   virtual void _unAbstract() = 0; // Makes this an Abstract class explicitly for documentation generation.   
   void _updLclBffrCntnt();

protected:
   uint8_t* _lclDspBuffPtr{nullptr};    //!< Pointer to an array of size equal to or less than **display module component** digits ports, will be equal to or less than **display controller component** maximum digits/ports. The need for a divided display buffer comes from the fact that some **Seven Segment display hardware** use controllers with larger digits management capabilities than the display module digits, and the exceeding digits are used for proprietary amenities, as colons, icons, etc.
   uint8_t _dspDigitsQtyMax{}; // Maximum display size in digits

   void _txStart();
   void _txAsk();
   void _txStop();
   void _txWrByte(uint8_t data);
   virtual void _sendBffr();
   virtual bool _sendByte(uint8_t data);   
   
 public:
   /**
    * @brief Default class constructor
    */
   SevenSegTM163X();
   /**
    * @brief Class constructor
    * 
    * @param ioPins A pointer to an array holding the identifiers for the 2 GPIO pins required to send the data to the **Seven Segment display hardware** to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->dio
    * @param dspDigits Quantity of digits/ports of the **display module component**. This parameter acceptable values are directly related to the TM163X family specific member.  
    * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).
    * @param dspContMaxDigits Maximum quantity of digits/ports the **display controller component** can handle, is a value that in this case depends on the TM163X family module member selected.
 */
   SevenSegTM163X(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspContMaxDigits);
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
    * 
    * @note The class constructor invokes the begin() method as it's last statement, the begin() method is kept for ease of modifications to developers interested in modifying the class.  
    */
   bool begin(uint32_t updtLps = 0);
   /**
    * @brief Turns Off the display.  
    * 
    * @return true Always  
    */
   bool end();
   /**
    * @brief Returns the current brightness level setting for the display module.  
    * 
    * The TM163X series display drivers have the capability of changing the led display brightness level by using PWM on it's output pins. The resulting brightness levels are not perceived as linear, and the minimum and maximum brightness values don't reach the levels of totally turning the display off, neither turning the display to it's maximum possible brightness.
    * 
    * @note The SevenSegTM163X abstract class is instrumented so that any subclass must incorporate the minimum and maximum values for that specific display subclass. All the members of the TM163X family currently checked at this point share the minimum and the maximum brightness values: 0 for the minimum, 7 for the maximum, resulting in 8 brightness levels. But this is not taken for granted. See setBrghtnssLvl(const uint8_t &) for more details.  
    * 
    * @return The current brightness level setting.  
    */
   virtual uint8_t getBrghtnssLvl();
   /**
    * @brief See SevenSegDispHw::ntfyUpdDsply() for description
    */
   virtual void ntfyUpdDsply();
   /**
    * @brief Sets the Brightness level of the display.  
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
   /**
    * @brief Turns the display module on.  
    * 
    * The display module will be turned on, it's brightness level set to the requested level, it's content displayed, and will keep that status until a turnOff() is invoked. 
    * 
    * @param newBrghtnssLvl The new brightness level for the display. The value must be in the range **getBrghtnssMinLvl() <= newBrghtnssLvl <= getBrghtnssMaxLvl()**
    */
   virtual void turnOn(const uint8_t &newBrghtnssLvl);
};
 
//============================================================> Class declarations separator

/**
 * @class SevenSegTM1636
 * 
 * @brief Models a Seven Segment display hardware controlled by a TM1636 display controller component.  
 * 
 * The TM1636 is a Titan Micro TM163X family member, whose differential characteristics from other members of this family (related to this library incumbent attributes) are:  
 * - Maximum number of display ports: 4
 */
class SevenSegTM1636: public SevenSegTM163X{
private:
   virtual void _unAbstract();

protected:
   const uint8_t _dspDigitsQtyMax{4}; // Maximum display controller managing digits capabilities

public:
   /**
    * @brief Default constructor
    */
   SevenSegTM1636();
   /**
    * @brief Class constructor
    * 
    * @param ioPins A pointer to an array holding the identifiers for the 2 GPIO pins required to send the data to the **Seven Segment display hardware** to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->dio
    * @param dspDigits Quantity of digits/ports of the display. This parameter for this subclass must be in the range 1 <= dspDigits <= 4.  
    * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).
*/
   SevenSegTM1636(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
   /**
    * @brief Default destructor
    */
   virtual ~SevenSegTM1636();
};
       
//============================================================> Class declarations separator

/**
 * @class SevenSegTM1637
 * 
 * @brief Models a Seven Segment display hardware controlled by a TM1637 display controller component.  
 * 
 * The TM1637 is a very popular Titan Micro TM163X family member, whose differential characteristics from other members of this family (related to this library incumbent attributes) are:  
 * - Maximum number of display ports: 6
 */
class SevenSegTM1637: public SevenSegTM163X{
private:
   const uint8_t _dspDigitsQtyMax{6}; // Maximum display size in digits
   virtual void _unAbstract();

public:
   /**
    * @brief Default constructor
    */
   SevenSegTM1637();
   /**
    * @brief Class constructor
    * 
    * @param ioPins A pointer to an array holding the identifiers for the 2 GPIO pins required to send the data to the **Seven Segment display hardware** to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->dio
    * @param dspDigits Quantity of digits/ports of the display. This parameter for this subclass must be in the range 1 <= dspDigits <= 6.  
    * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).
  */
   SevenSegTM1637(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
   /**
    * @brief Default destructor
    */
   ~SevenSegTM1637();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegTM1639
 * 
 * @brief Models a Seven Segment display hardware controlled by a TM1639 display controller component.  
 * 
 * The TM1639 is a Titan Micro TM1639 family member, whose differential characteristics from other members of this family (related to this library incumbent attributes) are:  
 * - Maximum number of display ports: 8  
 */
class SevenSegTM1639: public SevenSegTM163X{
private:
   const uint8_t _dspDigitsQtyMax{8}; // Maximum display size in digits
   virtual void _unAbstract();

public:
   /**
    * @brief Default constructor
    */
   SevenSegTM1639();
   /**
    * @brief Class constructor
    * 
    * @param ioPins A pointer to an array holding the identifiers for the 2 GPIO pins required to send the data to the **Seven Segment display hardware** to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->dio
    * @param dspDigits Quantity of digits/ports of the display. This parameter for this subclass must be in the range 1 <= dspDigits <= 8.  
    * @param commAnode Boolean indicating if the hardware uses a **display module component** wired as common anode (true) or common cathode (false).
  */
   SevenSegTM1639(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
   /**
    * @brief Default destructor
    */
   ~SevenSegTM1639();
};

//============================================================> Class declarations separator

/**
 * @class SevenSegMax7219
 * 
 * @brief Models a **Seven Segment display hardware** using a Max7219 **display controller component**
 * 
 * The Max7219 is a seven segment 8 digits (maximum) display controller that shares most of it's characteristics with the Max7221. The main difference is that the Max7219 is not completely compliant with the SPI communications protocol standard as the Max7221 is, so a software line handling solution must be provided. 
 * 
 * The Max72xx defines it's messages as 16-bit units.
 * Each 16-bit message includes data and address sections. 
 * The address space is separated into two sections: 
 * - Display ports content addresses
 * - Control registers addresses
 * 
 * 16-bit Message format: 
 * ----------------------------------
 * |F|E|D|C|B|A|9|8||7|6|5|4|3|2|1|0|
 *  ------- -------  ---------------
 *     |       |    MSb     |     LSb
 *     |       |            Data
 *     |       Address
 *     N/C
 * 
 * Address Space map: 
 * --------------------------------------------------------
 * Address|Purpose           |Selected value/Accepted range
 * --------------------------------------------------------
 * 0xX0   | NOP              | N/A
 * 0xX1~X8| Digit 0~7 content| 0x00~0xFF with DpABCDEFG bit order
 * 0xX9   | Decode Mode      | 0x00 NO decode of any kind (0x01, 0x0F, 0xFF valid options)
 * 0xXA   | Brightness level | 0xX0~0xXF (min to max, turns on at minimum)
 * 0xXB   | Used digits Qty. | 0xX0~0xX7 (1 to 7 digits, turns on at 1)
 * 0xXC   | Shutdown Register| 0x00: Shutdown, 0x01: normal operation
 * 0xXD   | N/C              |
 * 0x0E   | N/C              |
 * 0x0F   | Display test     | 0xX0: Normal operation, 0xX1: Test all leds ON
 */
class SevenSegMax7219: public SevenSegStatic{
	// Address Map Constants
	const uint8_t _NoOpAddr{0x00};
	const uint8_t _DspPortsBaseAddr{0x01};
	const uint8_t _DecodeModeAddr{0x09};
	const uint8_t _BrghtnsSettAddr{0x0A};
	const uint8_t _ScanLimitAddr{0x0B};
	const uint8_t _ShutDownAddr{0x0C};
	const uint8_t _DspTestAddr{0x0F};

	// Valid parameters constants
	const uint8_t _DisplayTestMode{0x01};
	const uint8_t _NoDecode{0x00};
	const uint8_t _NormalOp{0x00};
	const uint8_t _TurnOff{0x00};
	const uint8_t _TurnOn{0x01};

private:
	const uint8_t _clkIndx {0};
	const uint8_t _dinIndx {1};
	const uint8_t _csIndx {2};

	const uint8_t _dspDigitsQtyMax{8};
	const uint8_t _hwBrghtnssLvlMax{0x0F};
	const uint8_t _hwBrghtnssLvlMin{0x00};

	uint8_t _clk {};	// Serial clock max. rate 10 MHz. Data is shifted into the chip on **clk rising edge**
	uint8_t _din {};	// Data value to get into the chip reg, must be set before the clk rising edge to be accepted.
	uint8_t _cs {}; // The data in the internal 16 bits are accepted to be loaded while _cs is low, and will be latched and exposed to pins at _cs rising edge.

	uint8_t _cnvrtStdDgtTo72xxDgt(const uint8_t &StdDgt);
	virtual void _unAbstract();
	void _updLclBffrCntnt();

protected:
	uint8_t* _lclDspBuffPtr{nullptr};    //!< Pointer to an array of size equal to _dspDigitsQty, the local buffer differs from the shared _dspBuffPtr because it holds the data of the _dspBuffPtr formatted and ready to be sent to the display controller    
    
	virtual void _sendByte(const uint8_t &val, const bool &MSbFrst = true);
	virtual void _sendAddrData(const uint8_t &address, const uint8_t &data, const bool &MSbFrst = true);
	virtual void _sendBffr();
    
public:
	/**
	 * @brief Default class constructor
	 */
	SevenSegMax7219();    
	/**
    * @brief Class constructor
    * 
    * @param ioPins A pointer to an array holding the identifiers for the 3 GPIO pins required to send the data to the **Seven Segment display hardware** to be displayed. The correlation between the array positions and the pin function is given as in-class defined constants: 0->clk, 1->din, 2->cs.  
    * @param dspDigits Quantity of digits/ports of the display. This parameter for this subclass must be in the range 1 <= dspDigits <= 8.  
	 * 
	 * @note The Max72XX **display controller** family is designed to control common cathode **display modules**, so there is no parameter provided to instantiate a common anode SevenSegMax7219 class object.  
    */
	SevenSegMax7219(uint8_t* ioPins, uint8_t dspDigits);
	/**
 	 * @brief Class destructor
	 */
	~SevenSegMax7219();
	/**
	 * @brief Sets up the hardware display to work, and starts the display activities.  
	 * 
	 * For the Max72XX display controllers this setup includes:
	 * - Setting the display in "Normal operation mode"
	 * - Setting the quantity of display ports of the "Display module" connected to the controller.  
	 * - BCD decoding use.  
	 * - Turning on the Display controller (shutdown register setting).  
	 * 
	 * @return true Always
	*/
	bool begin(uint32_t updtLps = 0);
	/**
	 * @brief Ends the active mode of the display by shutting it off.  
	 * 
	 * While the control module will keep receiving and updating it's registers, while the display controller is in shutdown mode the display module will be turned off and no activity will be executed in the corresponding led powering lines.  
	 * 
	 * @return true Always
	 */
	bool end();
	/**
	 * @brief Returns a value indicating if the display controller is in working/On or shutdown/Off mode
	 * 
	 * @retval true The display controller is in working/On mode.  
	 * @retval false The display controller is in shutdown/Off mode.  
	 */
	virtual bool getIsOn();
	/**
    * @brief See SevenSegDispHw::ntfyUpdDsply() for description
    */
   virtual void ntfyUpdDsply();
   /**
    * @brief Sets the brightness level for the display module
    * 
	 * See SevenSegTM163X::setBrghtnssLvl(const uint8_t &) for details.  
    */
	virtual bool setBrghtnssLvl(const uint8_t &newBrghtnssLvl); 
    /**
    * @brief Turns the display module off.  
    * 
    * The display module will be cleared and will keep that status until a turnOn(), or turnOn(const uint8_t &) is invoked. 
    * 
    * @note While the display is Off, it will keep receiving display data and commands, but will keep the leds turned off. When it receives a turnOn command the display will show the data it received while turned off, or the same it was showing at turnOff if no change to the contents was done.  
    */
   virtual void turnOff();
   /**
    * @brief Turns the display module on.  
    * 
    * The display module will be turned on and it's content displayed, and will keep that status until a turnOff() is invoked.  
    */
   virtual void turnOn();
   /**
    * @brief Turns the display module on.  
    * 
    * The display module will be turned on, it's brightness level set to the requested level, it's content displayed, and will keep that status until a turnOff() is invoked. 
    * 
    * @param newBrghtnssLvl The new brightness level for the display. The value must be in the range **getBrghtnssMinLvl() <= newBrghtnssLvl <= getBrghtnssMaxLvl()**
    */
   virtual void turnOn(const uint8_t &newBrghtnssLvl);
};

//============================================================> Class declarations separator

#endif