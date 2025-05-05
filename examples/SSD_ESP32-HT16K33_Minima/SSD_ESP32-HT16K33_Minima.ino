/**
 ******************************************************************************
 * @file SSD_ESP32-HT16K33_Minima.ino
 * 
 * @brief Code example file to demonstrate SevenSegDisplays_ESP32 library use with SevenSegDispHw::HT16K33 class
 * 
 * @details 
 *
 * Repository: https://github.com/GabyGold67/SevenSegDisplays_ESP32  
 * 
 * Framework: Arduino
 * Platform: ESP32
 * 
 * @author	: Gabriel D. Goldman
 * mail <gdgoldman67@hotmail.com>
 * Github <https://github.com/GabyGold67>
 *
 * @date First release: 15/05/2023  
 *       Last update:   27/04/2025 17:00 GMT+0200 DST  
 ******************************************************************************
 * @warning **Use of this library is under your own responsibility**
 * 
 * @warning The use of this library falls in the category described by The Alan 
 * Parsons Project (c) 1980 "Games People play" disclaimer:
 * 
 * Games people play, you take it or you leave it
 * Things that they say aren't alright
 * If I promised you the moon and the stars, would you believe it?
 * 
 ******************************************************************************
 * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
 ******************************************************************************
*/
#include <Arduino.h>
#include <SevenSegDisplays.h>
//==============================================>> General use definitions BEGIN
#define LoopDlyTtlTm 1500 // Time between task unblocking, time taken from the start of the task execution to the next execution 
#define MainCtrlTskPrrtyLvl 4 // Task priority level

static BaseType_t xReturned; /*!<Static variable to keep returning result value from Tasks and Timers executions*/
static BaseType_t errorFlag {pdFALSE};

BaseType_t ssdExecTskCore = xPortGetCoreID();
BaseType_t ssdExecTskPrrtyCnfg = MainCtrlTskPrrtyLvl;
//================================================>> General use definitions END
 
//======================================>> General use function prototypes BEGIN
void Error_Handler();
//========================================>> General use function prototypes END
 
const uint8_t sda {GPIO_NUM_21};  // Pin connected to DIO of TM1637
const uint8_t scl {GPIO_NUM_22}; // Pin connected to CLK of TM1637

uint8_t myDispIOPins[2] {scl, sda}; // Pins set as an array as required by hw constructor

SevenSegDispHw* myLedDispPtr {new SevenSegHT16K33(myDispIOPins, 4, 0x70)};
SevenSegDisplays myLedDisp(myLedDispPtr);

void setup() { 
   delay(10);  //FTPO Part of the WOKWI simulator additions, for simulation startup needs

   Serial.begin(9600); //FTPO
   Serial.println("\n\n\nTest started"); //FTPO
   Serial.println("============"); //FTPO
   myLedDisp.begin();
}

void loop() {
   if(!myLedDisp.getIsOn())
      myLedDisp.turnOn();

   myLedDisp.clear();
   myLedDisp.print("GabY");
  vTaskDelay(2000);

  myLedDisp.print("Pau");
  vTaskDelay(2000);
  myLedDisp.turnOff();
}  

//=======================================>> User Functions Implementations BEGIN
/**
 * @brief Error Handling function
 * 
 * Placeholder for a Error Handling function, in case of an error the execution
 * will be trapped in this endless loop
 */
void Error_Handler(){
   for(;;)
   {    
   }
   
   return;
}
//=========================================>> User Functions Implementations END
