/**
 ******************************************************************************
 * @file SSD_ESP32-Max7219_cnvrtStdDgtTo72xxDgt.ino
*/
#include <Arduino.h>
#include <SevenSegDisplays.h>
//==============================================>> General use definitions BEGIN
#define LoopDlyTtlTm 1500 // Time between task unblocking, time taken from the start of the task execution to the next execution 
#define MainCtrlTskPrrtyLvl 4 // Task priority level

//================================================>> General use definitions END
/*
    B01111110,	// 0 <- B00111111
	 B00110000,	// 1 <- B00000110
	 B01101101,	// 2 <- B01011011
	 B01111001,	// 3 <- B01001111
	 B00110011,	// 4 <- B01100110
	 B01011011,	// 5 <- B01101101
	 B01011111,	// 6 <- B01111101
	 B01110000,	// 7 <- B00000111
	 B01111111,	// 8 <- B01111111
	 B01111011,	// 9 <- B01101111
	 B00000001	// - <- B01000000
*/

const uint8_t clk {GPIO_NUM_25};  // Pin connected to clk of Max7219
const uint8_t cs {GPIO_NUM_26};  // Pin connected to cs of Max7219
const uint8_t din {GPIO_NUM_33}; // Pin connected to din of Max7219

static uint8_t myDispIOPins[3] {clk, din, cs}; // Pins set as an array as required by hw constructor

SevenSegDispHw* myLedDispPtr {new SevenSegMax7219(myDispIOPins, 8)};
// SevenSegDisplays myLedDisp(myLedDispPtr);

void setup() { 
   Serial.begin(9600); //FTPO
   myLedDispPtr->begin();
   Serial.println("\n\n\nTest started"); //FTPO
   Serial.println("============"); //FTPO
}

void loop() {
   Serial.print("Original value: ");
   Serial.print("B00111111; Converted to: ");
   Serial.println(myLedDispPtr->_cnvrtStdDgtTo72xxDgt(B00111111), BIN);

   Serial.print("Original value: ");
   Serial.print("B00000110; Converted to: ");
   Serial.println(myLedDispPtr->_cnvrtStdDgtTo72xxDgt(B00000110), BIN);

   Serial.print("Original value: ");
   Serial.print("B01011011; Converted to: ");
   Serial.println(myLedDispPtr->_cnvrtStdDgtTo72xxDgt(B01011011), BIN);

   Serial.print("Original value: ");
   Serial.print("B01001111; Converted to: ");
   Serial.println(myLedDispPtr->_cnvrtStdDgtTo72xxDgt(B01001111), BIN);

   Serial.print("Original value: ");
   Serial.print("B01100110; Converted to: ");
   Serial.println(myLedDispPtr->_cnvrtStdDgtTo72xxDgt(B01100110), BIN);
   for(;;){
      vTaskDelay(1000);
   }
}  

//===============================>> User Tasks Implementations BEGIN

/* A one liner example using as argument the pointer returned from dynamic instantiated object
SevenSegDisplays myLedDisp(new SevenSegDynHC595 (myDispIOPins, 4, true));
*/

   // uint8_t theNewOrder [4] {3, 2, 1, 0};
   // myLedDisp.getDspUndrlHwPtr()->begin();

//=========================================>> User Functions Implementations END
