/**
 ******************************************************************************
 * @file SSD_ESP32-Max7219x8.ino
 * 
 * @brief Code example file to demonstrate SevenSegDisplays_ESP32 library use with SevenSegDispHw::SevenSegMax7219 class
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
 *       Last update:   16/04/2025 17:30 GMT+0200 DST  
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

   //Set of variables and constants needed for the tests
   bool testResult{};
   const long testTime{2000};
   bool myBlinkMask[8] {true, true, true, true, true, true, true, true};

   const uint8_t clk {GPIO_NUM_25};  // Pin connected to clk of Max7219
   const uint8_t cs {GPIO_NUM_26};  // Pin connected to cs of Max7219
   const uint8_t din {GPIO_NUM_33}; // Pin connected to din of Max7219
   
   static uint8_t myDispIOPins[3] {clk, din, cs}; // Pins set as an array as required by hw constructor


/* Instantiation examples, different possibilities for use according to developer preferences*/
/* A three lines step by step code example:  
SevenSegDynHC595 myLedDispHw(myDispIOPins, 4, true);
SevenSegDispHw* myLedDispHwPtr = &myLedDispHw;
SevenSegDisplays myLedDisp(myLedDispHwPtr);
*/

/* A two lines example using the & operand to pass the pointer
SevenSegDynHC595 myLedDispHw(myDispIOPins, 4, true);
SevenSegDisplays myLedDisp(&myLedDispHw);
*/

/* A two lines example using a sub-class pointer to a dynamic instantiated object
SevenSegDynHC595* myLedDispPtr {new SevenSegDynHC595 (myDispIOPins, 4, true)};
SevenSegDisplays myLedDisp(myLedDispPtr);
*/

/* A two lines example using a base class pointer to a dynamic instantiated object
SevenSegDispHw* myLedDispPtr {new SevenSegDynHC595 (myDispIOPins, 4, true)};
SevenSegDisplays myLedDisp(myLedDispPtr);
*/

/* A one liner example using as argument the pointer returned from dynamic instantiated object
SevenSegDisplays myLedDisp(new SevenSegDynHC595 (myDispIOPins, 4, true));
*/

   // uint8_t theNewOrder [4] {3, 2, 1, 0};

   SevenSegDispHw* myLedDispPtr {new SevenSegMax7219(myDispIOPins, 8)};
   // myLedDispPtr -> setDigitsOrder(theNewOrder);
   SevenSegDisplays myLedDisp(myLedDispPtr);

   Serial.println("Service Started");

for(;;){

   {
      if(!myLedDisp.getDspUndrlHwPtr()->getIsOn()){
         myLedDisp.getDspUndrlHwPtr()->turnOn();
         Serial.println("Display turned on");
      }
      else
         Serial.println("Display WAS ALREADY turned ON");   
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      myLedDisp.clear();
      Serial.println("Display cleared");
      testResult = myLedDisp.print("On");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0");
      testResult = myLedDisp.print("0");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1");
      testResult = myLedDisp.print("0.1");
      vTaskDelay(testTime);
   }
      
   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2");
      testResult = myLedDisp.print("0.1.2");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3");
      testResult = myLedDisp.print("0.1.2.3");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3.4");
      testResult = myLedDisp.print("0.1.2.3.4");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3.4.5");
      testResult = myLedDisp.print("0.1.2.3.4.5");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3.4.5.6");
      testResult = myLedDisp.print("0.1.2.3.4.5.6");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3.4.5.6.7");
      testResult = myLedDisp.print("0.1.2.3.4.5.6.7");
      vTaskDelay(testTime);
   }

   {
      //print() with a string argument, two characters long, all characters included in the representable characters list
      Serial.println("Display 0.1.2.3.4.5.6.7.");
      testResult = myLedDisp.print("0.1.2.3.4.5.6.7.");
      vTaskDelay(testTime);
   }

   {
         //print() with a string argument, four characters long, all characters included in the representable characters list
         Serial.println("Display turned Off");
         myLedDisp.getDspUndrlHwPtr()->turnOff();  // Demonstrates the display control keeps receiving data altough it's set turned Off
         Serial.println("Text 'Strt'sent to the display while turned Off");
         testResult = myLedDisp.print("Strt");
         myLedDisp.getDspUndrlHwPtr()->turnOn();
         Serial.println("Display turned On, showing the data was received");
         vTaskDelay(testTime);
      }

      {
         //print() with a string argument, fails to represent as it is 5 chars long (enough to fail), AND has a non-representable char included (!)
         testResult = myLedDisp.print("Hello!");
         if(!testResult)
            testResult = myLedDisp.print("FAIL");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("thiS");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("teSt");
         vTaskDelay(testTime);
      }

      {
         //print() with a string argument, four characters long AND usable dots, all characters included in the representable characters list
         //Each valid character might be followed by a "." if needed, without being counted as a character, even spaces and special chars
         testResult = myLedDisp.print("I.F.Y.I.");
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, ONE decimal digit place to display, no alignment specified
         testResult = myLedDisp.print(2.3, 1);
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
         vTaskDelay(testTime);
      }

      {
         //print() with a floating point argument, one decimal digit argument, ONE decimal digit place to display, right alignment specified
         testResult = myLedDisp.print(-2.3, 1, true);
         vTaskDelay(testTime);
      }

      {
         //gauge() with a floating point argument, 0 <= value <= 1.0, representing a percentage, the four ranges are:
         // 0 <= 1st range <0.25
         //0.25 <= 2nd range < 0.50
         //0.50 <= 3rd range < 0.75
         //0.75 <= 4rd range <= 1.0
         //A character to show ahead of the value is an option, 
         //if not specified will be a blank space, in this case 'b', or any other representable char, for example:
         //b for battery, F for fuel, t for temperature, r for remaining...
         testResult = myLedDisp.gauge(1.0, 'b');
         vTaskDelay(testTime);
      }

      {
         //gauge() with a floating point argument, 3rd range
         testResult = myLedDisp.gauge(0.74, 'b');
         vTaskDelay(testTime);
      }

      {
         //gauge() with an integer argument, 2nd range, start blinking to show low level
         testResult = myLedDisp.gauge(1, 'b');
         myLedDisp.blink();
         vTaskDelay(testTime);
      }

      {
         //gauge() with an integer argument, 2nd range, start blinking to show low level
         testResult = myLedDisp.gauge(1, 'b');
         myLedDisp.blink(400);
         vTaskDelay(testTime);
      }
      
      {
         //gauge() with an integer argument, 1st range, blinks faster
         myLedDisp.setBlinkRate(300);
         testResult = myLedDisp.gauge(0, 'b');
         vTaskDelay(testTime*2);
      }
      
      {
         //gauge() with an integer argument, 1st range, blinks faster
         myLedDisp.setBlinkRate(110, 200);
         vTaskDelay(testTime*2);
         testResult = myLedDisp.noBlink();
      }
      
      {
         testResult = myLedDisp.print("You");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("ran");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("out");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("oF");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("batterY");
         vTaskDelay(testTime);
      }

      {
         testResult = myLedDisp.print("0246");
         vTaskDelay(testTime);
      }

      {         
         myBlinkMask[0] = true;
         myBlinkMask[1] = false;
         myBlinkMask[2] = false;
         myBlinkMask[3] = false;
         myLedDisp.setBlinkMask(myBlinkMask);
         myLedDisp.blink(200);
         vTaskDelay(testTime*2);
      }

      {         
         myLedDisp.write("7", 4);
         vTaskDelay(testTime);
      }

      {
         myBlinkMask[0] = false;
         myBlinkMask[1] = true;
         myBlinkMask[2] = false;
         myBlinkMask[3] = false;
         myLedDisp.setBlinkMask(myBlinkMask);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("5", 5);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("6", 5);
         vTaskDelay(testTime);
      }

      {
         myBlinkMask[0] = false;
         myBlinkMask[1] = false;
         myBlinkMask[2] = true;
         myBlinkMask[3] = false;
         myLedDisp.setBlinkMask(myBlinkMask);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("3", 6);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("4", 6);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("5", 6);
         vTaskDelay(testTime);
      }

      {
         myBlinkMask[0] = false;
         myBlinkMask[1] = false;
         myBlinkMask[2] = false;
         myBlinkMask[3] = true;
         myLedDisp.setBlinkMask(myBlinkMask);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("1", 7);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("2", 7);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("3", 7);
         vTaskDelay(testTime);
      }

      {         
         myLedDisp.write("4", 7);
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
         int8_t minBrgthnss {(int8_t)myLedDisp.getDspUndrlHwPtr()->getBrghtnssMinLvl()};
         int8_t maxBrgthnss {(int8_t)myLedDisp.getDspUndrlHwPtr()->getBrghtnssMaxLvl()};
         int8_t curBrgthnss {(int8_t)myLedDisp.getDspUndrlHwPtr()->getBrghtnssLvl()};

         Serial.print("Minimum brightness level: ");
         Serial.println(minBrgthnss);
         Serial.print("Maximum brightness level: ");
         Serial.println(maxBrgthnss);
         Serial.print("Current brightness level: ");
         Serial.println(curBrgthnss);

         for(int8_t curBrgthnss{maxBrgthnss}; curBrgthnss >= minBrgthnss; --curBrgthnss){
            myLedDisp.getDspUndrlHwPtr()->setBrghtnssLvl(curBrgthnss);
            Serial.print("Brightness Level = ");
            Serial.println(curBrgthnss);
            vTaskDelay(350);   
         }
         for(int8_t curBrgthnss{minBrgthnss}; curBrgthnss <= maxBrgthnss; ++curBrgthnss){
            myLedDisp.getDspUndrlHwPtr()->setBrghtnssLvl(curBrgthnss);
            Serial.print("Brightness Level = ");
            Serial.println(curBrgthnss);
            vTaskDelay(350);   
         }
      }

      {
         myLedDisp.print("OFF");
         vTaskDelay(testTime);
      }

      {
         myLedDisp.getDspUndrlHwPtr()->turnOff();
         Serial.println("Display turned Off");
         vTaskDelay(testTime);
      }

      {
         myLedDisp.print("On");
         myLedDisp.getDspUndrlHwPtr()->turnOn();
         Serial.println("Display turned On");
         vTaskDelay(testTime);
      }
      {
         myLedDisp.print("End");
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
         myLedDisp.getDspUndrlHwPtr()->turnOff();
         Serial.println("Display turned off");
         vTaskDelay(testTime);
      }

      {
         vTaskDelay(testTime);
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
