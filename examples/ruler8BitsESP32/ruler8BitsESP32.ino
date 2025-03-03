/*
  ruler8BtisESP32.ino - Example file to demonstrate SevenSeg74HC595 class use with a single display
  Created by Gabriel D. Goldman, May, 2023.
  Updated by Gabriel D. Goldman, December, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
*/
#include <Arduino.h>
#include <SevenSeg-74HC595.h>

// Pin connected to DS of 74HC595 AKA DIO
const uint8_t dio {GPIO_NUM_12}; // // Board pin # of ESP32 WROOM-32 uPesy dev Board
// Pin connected to ST_CP of 74HC595 AKA RCLK
const uint8_t rclk {GPIO_NUM_13}; // Board pin # of ESP32 WROOM-32 uPesy dev Board
// Pin connected to SH_CP of 74HC595 AKA SCLK
const uint8_t sclk {GPIO_NUM_14}; // Board pin # of ESP32 WROOM-32 uPesy dev Board

//Set of variables and constants needed just for Demo purposes
bool testResult{false};

//Display instance creation
// Creating the instance of the display, the only parameters needed are
// the pins that will be connected to the display module, usually marked as:
// SCLK
// RCLK
// DIO
SevenSeg74HC595 myLedDisp(sclk, rclk, dio, true, 8);
//Display digits ports order for DIY More 8-Digit LED Display
uint8_t diyMore[8] {3, 2, 1, 0, 7, 6, 5, 4};

void setup(){
  myLedDisp.begin();
  myLedDisp.setDigitsOrder(diyMore, 8);
}

void loop(){
  testResult = myLedDisp.print("0");
  delay(1500);
  testResult = myLedDisp.print("01");
  delay(1500);
  testResult = myLedDisp.print("012");
  delay(1500);
  testResult = myLedDisp.print("0123");
  delay(1500);
  testResult = myLedDisp.print("01234");
  delay(1500);
  testResult = myLedDisp.print("012345");
  delay(1500);
  testResult = myLedDisp.print("0123456");
  delay(1500);
  testResult = myLedDisp.print("01234567");
  delay(1500);
}

