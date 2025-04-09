/**
 ******************************************************************************
 * @file SSD_ESP32-StatHC595x3.ino
 * 
 * @brief Code example file to demonstrate SevenSegDisplays_ESP32 library use with SevenSegDispHw::SevenSegStatHC595 class
 * 
 * @details This example uses a 3 digits static 74HC595 **display module component**
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
 *       Last update:   09/04/2025 18:50 GMT+0200 DST  
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
 *****************************************************************************
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
 
//====================================>> Task Callback function prototypes BEGIN
void mainCtrlTsk(void *pvParameters);
//======================================>> Task Callback function prototypes END
 
//===========================================>> Tasks Handles declarations BEGIN
TaskHandle_t mainCtrlTskHndl {NULL};
//=============================================>> Tasks Handles declarations END

void setup() { 
   delay(10);  //FTPO Part of the WOKWI simulator additions, for simulation startup needs

   Serial.begin(9600); //FTPO
   Serial.println("\n\n\nTest started"); //FTPO
   Serial.println("============"); //FTPO

   // Create the Main control task for setup and execution of the main code
   xReturned = xTaskCreatePinnedToCore(
      mainCtrlTsk,  // Callback function/task to be called
      "MainControlTask",  // Name of the task
      2048,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  // Pointer to the parameters for the function to work with
      ssdExecTskPrrtyCnfg, // Priority level given to the task
      &mainCtrlTskHndl, // Task handle
      ssdExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
}

void loop() {
   vTaskDelete(NULL); // Delete this task -the ESP-Arduino LoopTask()- and remove it from the execution list
}  

//===============================>> User Tasks Implementations BEGIN
void mainCtrlTsk(void *pvParameters){
   delay(10);  //FTPO Part of the WOKWI simulator additions, for simulation startup needs

   TickType_t loopTmrStrtTm{0};
   TickType_t* loopTmrStrtTmPtr{&loopTmrStrtTm};
   TickType_t totalDelay {LoopDlyTtlTm};

   //Set of variables and constants needed for the tests
   bool testResult{};
   const long testTime{2000};
   bool myBlinkMask[3] {true, true, true};

   const uint8_t dio {GPIO_NUM_33};  // Pin connected to DS of 74HC595 AKA DIO AKA SDI
   const uint8_t rclk {GPIO_NUM_25}; // Pin connected to ST_CP of 74HC595 AKA RCLK AKA LOAD
   const uint8_t sclk {GPIO_NUM_26}; // Pin connected to SH_CP of 74HC595 AKA SCLK
   
   static uint8_t myDispIOPins[3] {sclk, rclk, dio}; // Pins set as an array as required by hw constructor


   uint8_t theNewOrder [3] {2, 1, 0};

   SevenSegDispHw* myLedDispPtr {new SevenSegStatHC595 (myDispIOPins, 3, true)};
   myLedDispPtr -> setDigitsOrder(theNewOrder);
   SevenSegDisplays myLedDisp(myLedDispPtr);


   for(;;){
      {
         myLedDisp.getDspUndrlHwPtr()->begin();
         Serial.println("Service Started");
         vTaskDelay(250);
      }

      {
         myLedDisp.clear();
         vTaskDelay(testTime);
      }

      {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      testResult = myLedDisp.print("On");
      vTaskDelay(testTime);
      }

      {
         //print() with a string argument, four characters long, all characters included in the representable characters list
         testResult = myLedDisp.print("oah");
         vTaskDelay(testTime);
      }

      {
         //print() with a string argument, fails to represent as it is 5 chars long (enough to fail), AND has a non-representable char included (!)
         testResult = myLedDisp.print("Hello!");
         if(!testResult)
            testResult = myLedDisp.print("Err");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("You");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("are");
         vTaskDelay(testTime);
      }

      {
         //print() with a string argument, four characters long AND usable dots, all characters included in the representable characters list
         //Each valid character might be followed by a "." if needed, without being counted as a character, even spaces and special chars
         testResult = myLedDisp.print("F.u.n.");
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, ONE decimal digit place to display, no alignment specified
         testResult = myLedDisp.print(2.3, 1);
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, ONE decimal digit place to display, right alignment specified
         testResult = myLedDisp.print(2.3, 1, true);
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, TWO decimal digit places to display, no alignment specified
         testResult = myLedDisp.print(2.3, 2);
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, TWO decimal digit places to display, right alignment specified
         testResult = myLedDisp.print(2.3, 2, true);
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, TWO decimal digit places to display, right alignment specified, zero padded specified
         testResult = myLedDisp.print(2.3, 2, true, true);
         vTaskDelay(testTime);
      }

      {
         //print() with a negative floating point argument, one decimal digit argument, TWO decimal digit places to display, no alignment specified
         testResult = myLedDisp.print(-2.3, 2);
         if(!testResult)
            testResult = myLedDisp.print("Err");
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, ONE decimal digit place to display, right alignment specified
         testResult = myLedDisp.print(-2.3, 1, true);
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("024");
         vTaskDelay(testTime);
      }

      {         
         myBlinkMask[0] = true;
         myBlinkMask[1] = false;
         myBlinkMask[2] = false;
         myLedDisp.setBlinkMask(myBlinkMask);
         myLedDisp.blink(250);
         vTaskDelay(testTime*2);
      }

      {         
         myLedDisp.write("5", 0);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("6", 0);
         vTaskDelay(testTime);
      }
      
      {
         myBlinkMask[0] = false;
         myBlinkMask[1] = true;
         myBlinkMask[2] = false;
         myLedDisp.setBlinkMask(myBlinkMask);
         vTaskDelay(testTime*2);
      }

      {         
         myLedDisp.write(0xAD, 1);  //Random non ASCII character
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write(0x81, 1);  //Random non ASCII character
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("5", 1);
         vTaskDelay(testTime);
      }

      {
         myBlinkMask[0] = false;
         myBlinkMask[1] = false;
         myBlinkMask[2] = true;
         myLedDisp.setBlinkMask(myBlinkMask);
         vTaskDelay(testTime*2);
      }

      {         
         myLedDisp.write("1", 2);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("2", 2);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("3", 2);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("4", 2);
         vTaskDelay(testTime);
      }

      {
         myLedDisp.resetBlinkMask();
         vTaskDelay(testTime*2);
         myLedDisp.noBlink();
      }

      {
         myLedDisp.wait();
         vTaskDelay(testTime * 2);
      }

      {
         myLedDisp.setWaitRate(125);
         vTaskDelay(testTime * 2);
      }

      {
         myLedDisp.setWaitChar('.');
         vTaskDelay(testTime * 2);
      }

      {
         myLedDisp.setWaitRate(100);
         myLedDisp.setWaitChar('o');
         vTaskDelay(testTime * 2);
      }

      {
         myLedDisp.setWaitRate(100);
         myLedDisp.setWaitChar('0');
         vTaskDelay(testTime * 2);
      }

      {
         myLedDisp.noWait();
         myLedDisp.setWaitRate(250);
         myLedDisp.setWaitChar('-');
         vTaskDelay(testTime);
      }
      
      {
         myLedDisp.print("OFF");
         vTaskDelay(testTime);
      }

      {
         myLedDisp.clear();
         vTaskDelay(testTime);
      }

      {
         myLedDisp.getDspUndrlHwPtr()->end();
         Serial.println("Service stopped");
         vTaskDelay(testTime);
      }
   
      {
         // vTaskDelay(testTime);
      }
   }
}

//================================================>> General use functions BEGIN
//==================================================>> General use functions END

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
