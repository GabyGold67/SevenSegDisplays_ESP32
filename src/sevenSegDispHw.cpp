/**
 * @file SevenSegDispHw.cpp
 * @brief Code file for the SevenSegDisplays_ESP32 library, SevenSegDispHw class and subclasses 
 * 
 * @author Gabriel D. Goldman
 * 
 * @version 3.0.0
 * 
 * @date First release: 20/12/2023 
 *       Last update:   25/03/2025 11:20 (GMT+0200)
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
 */
#include "Arduino.h"
#include "sevenSegDispHw.h"
//----------------------------------------->> Global constants declaration BEGIN
const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t noName4Bits[4] {0, 1, 2, 3};
//------------------------------------------->> Global constants declaration END

//-------------------------------------->> Static variables initialization BEGIN
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
// TimerHandle_t SevenSegDynamic::_dynDspRfrshTmrHndl = nullptr;
// TimerHandle_t SevenSegDynHC595::_dynHC595DspRfrshTmrHndl = nullptr;
// TimerHandle_t SevenSegDynDummy::_dynDummyDspRfrshTmrHndl = nullptr;
//---------------------------------------->> Static variables initialization END

//============================================================> Class methods separator

SevenSegDispHw::SevenSegDispHw() {}

SevenSegDispHw::SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:_ioPins{ioPins}, _digitPosPtr{new uint8_t[dspDigits]}, _dspDigitsQty {dspDigits}, _commAnode {commAnode}
{
   Serial.println("\nSevenSegDispHw constructor"); //FTPO
   Serial.println("=========================="); //FTPO
    
   _dspHwInstNbr = _dspHwSerialNum++;
   for (uint8_t i{0}; i < _dspDigitsQty; i++){
      *(_digitPosPtr + i) = i;
   }    
}

SevenSegDispHw::~SevenSegDispHw() {
   delete [] _digitPosPtr;
}

bool SevenSegDispHw::begin(uint32_t updtLps){
   
   return true;
}

bool SevenSegDispHw::getCommAnode(){

   return _commAnode;
}

uint8_t* SevenSegDispHw::getDspBuffPtr(){
    
   return _dspBuffPtr;
}

uint8_t SevenSegDispHw::getHwDspDigitsQty(){

   return _dspDigitsQty;
}

void SevenSegDispHw::send(uint8_t *digitsBuffer){

   return;
}

void SevenSegDispHw::send(const uint8_t &segments, const uint8_t &port){

   return;
}

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
   bool result{true};

   for(int i {0}; i < _dspDigitsQty; i++){
      if (*(newOrderPtr + i) >= _dspDigitsQty){
         result = false;
         break;
      }   
   }
   if (result)
      memcpy(_digitPosPtr, newOrderPtr, _dspDigitsQty);

   return result;
}

void SevenSegDispHw::setDspBuffPtr(uint8_t* newDspBuffPtr){
   _dspBuffPtr = newDspBuffPtr;

   return;
}

void SevenSegDispHw::setNtfyUpdDsply(){

   return;
}

bool SevenSegDispHw::end(){
   
   return true;
}

//============================================================> Class methods separator
//FFDR Start revision from here Gaby
SevenSegDynamic::SevenSegDynamic(){}

SevenSegDynamic::SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{
   Serial.println("\nSevenSegDynamic constructor"); //FTPO
   Serial.println("==========================="); //FTPO
}

SevenSegDynamic::~SevenSegDynamic(){}

bool SevenSegDynamic::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   Serial.println("\n"); //FTPO
   Serial.println("SevenSegDynamic .begin()"); //FTPO
   Serial.println("\n"); //FTPO

   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
   if (!_svnSgDynTmrHndl){
      //Create a valid unique Name for identifying the timer created
      String rfrshTmrName{""};
      String dspSerialNumStr {"000" + String(_dspHwInstNbr)};
      dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
      rfrshTmrName = "DynDsp" + dspSerialNumStr + "rfrsh_tmr";

      Serial.print("\nSevenSegDynamic refresh timer name: "); //FTPO
      Serial.println(rfrshTmrName); //FTPO
      Serial.println("================"); //FTPO

      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
      _dynDspRfrshTmrHndl = xTimerCreate(
         rfrshTmrName.c_str(),
         pdMS_TO_TICKS((int)(1000/(30 * _dspDigitsQty))),
         // pdMS_TO_TICKS(100),
         pdTRUE,  //Auto-reload
         NULL,   //TimerID, data to be passed to the callback function
         SevenSegDynamic::tmrCbRfrshDyn  //Callback function
      );
      if((_dynDspRfrshTmrHndl != nullptr) && (!xTimerIsTimerActive(_dynDspRfrshTmrHndl))){
         tmrModResult = xTimerStart(_dynDspRfrshTmrHndl, portMAX_DELAY);
         if (tmrModResult == pdPASS)
            result = true;
      }
   }

   return result;
}

void SevenSegDynamic::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    for (int i {0}; i < _dspDigitsQty; i++){
        tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
        send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

    return;
}

void SevenSegDynamic::send(uint8_t content){ // Implementation is hardware dependant (subclasses) protocol!!

   return;
}

void SevenSegDynamic::send(const uint8_t &segments, const uint8_t &port){

   return;
}

bool SevenSegDynamic::end() {
    bool result {false};

    if(_dynDspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
        xTimerStop(_dynDspRfrshTmrHndl, portMAX_DELAY);
        xTimerDelete(_dynDspRfrshTmrHndl, portMAX_DELAY);
        _dynDspRfrshTmrHndl = nullptr;
    }

    return result;
}

void SevenSegDynamic::tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg){
   // No need for specific executable code in this callback function at this stage

   return;
}

//============================================================> Class methods separator

SevenSegDynHC595::SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(ioPins, dspDigits, commAnode)
{    
   _sclk = *(ioPins + _sclkIndx);
   _rclk = *(ioPins + _rclkIndx);
   _dio = *(ioPins + _dioIndx);
    
    _drvrShftRegPtr = new ShiftRegGPIOXpander(_dio, _sclk, _rclk, 2, nullptr);
    _drvrShftRegSndPtr = new uint8_t[2];

    begin();
}

SevenSegDynHC595::~SevenSegDynHC595(){}

bool SevenSegDynHC595::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   Serial.println("\n"); //FTPO
   Serial.println("SevenSegDynHC595 .begin()"); //FTPO
   Serial.println("\n"); //FTPO

   _firstRefreshed = 0;
   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
   if (!_dynHC595DspRfrshTmrHndl){
        //Create a valid unique Name for identifying the timer created
         String rfrshTmrName{""};
         String dspSerialNumStr {"000" + String(_dspHwInstNbr)};
         dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
         rfrshTmrName = "DynHC595Dsp" + dspSerialNumStr + "rfrsh_tmr";

        //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
        _dynHC595DspRfrshTmrHndl = xTimerCreate(
            rfrshTmrName.c_str(),   // Timer human readable name
            pdMS_TO_TICKS((int)(1000/(30 * _dspDigitsQty))),
            pdTRUE,  // Autoreload
            this,   // TimerID, data to be passed to the callback function
            tmrCbRfrshDynHC595  //Callback function
        );
        if((_dynHC595DspRfrshTmrHndl != NULL) && (!xTimerIsTimerActive(_dynHC595DspRfrshTmrHndl))){
            tmrModResult = xTimerStart(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

void SevenSegDynHC595::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

   for (int i {0}; i < _dspDigitsQty; i++){
      tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      // send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));

      *(_drvrShftRegSndPtr + 0) = uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      *(_drvrShftRegSndPtr + 1) = tmpDigToSend;
      _drvrShftRegPtr->stampOverMain(_drvrShftRegSndPtr);

   }
   ++_firstRefreshed;
   if (_firstRefreshed == _dspDigitsQty)
      _firstRefreshed = 0;

   return;
}

void SevenSegDynHC595::send(uint8_t content){

   return;
}

void SevenSegDynHC595::send(const uint8_t &segments, const uint8_t &port){

   return;
}

bool SevenSegDynHC595::end() {
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};
   
   //FFDR Check for the blink and the WAIT (specially the wait) timers to stop them if they are running to avoid funny combinations
   if(_dynHC595DspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      tmrModResult = xTimerStop(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult){
         tmrModResult = xTimerDelete(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
         if(tmrModResult){
            _dynHC595DspRfrshTmrHndl = NULL;
            result = true;
         }
      }   
   }

   return result;
}

void SevenSegDynHC595::tmrCbRfrshDynHC595(TimerHandle_t rfrshTmrCbArg){
    SevenSegDynHC595* SevenSegUndrlHC595 = (SevenSegDynHC595*) pvTimerGetTimerID(rfrshTmrCbArg);
    //Timer Callback to keep the display lit by calling this display's refresh() method
    
    SevenSegUndrlHC595->refresh();

    return;
}

//============================================================> Class methods separator

SevenSegDynDummy::SevenSegDynDummy(uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(nullptr, dspDigits, commAnode)
{
   begin();
}

SevenSegDynDummy::~SevenSegDynDummy(){}

bool SevenSegDynDummy::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   Serial.begin(9600);
   Serial.println("Seven Segment Dynamic Dummy Display Begin");
   Serial.println("================================================");
   Serial.println("For testing purposes the 'display' will be refreshed once every 2 seconds or 0.5Hz");
   Serial.println("================================================");

   //FFDR Grab the begin() code from the SevenSegDynHC595 and adapt it here

   _firstRefreshed = 0;
   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
   if (!_dynDummyDspRfrshTmrHndl){
      //Create a valid unique Name for identifying the timer created
      String rfrshTmrName{""};
      String dspSerialNumStr {"000" + String(_dspHwInstNbr)};
      dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
      rfrshTmrName = "DynDummyDsp" + dspSerialNumStr + "rfrsh_tmr";

      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
      _dynDummyDspRfrshTmrHndl = xTimerCreate(
         rfrshTmrName.c_str(),   // Timer human readable name
         pdMS_TO_TICKS(2000),
         pdTRUE,  // Autoreload
         this,   // TimerID, data to be passed to the callback function
         tmrCbRfrshDynDummy  //Callback function
      );
      if((_dynDummyDspRfrshTmrHndl != NULL) && (!xTimerIsTimerActive(_dynDummyDspRfrshTmrHndl))){
         tmrModResult = xTimerStart(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
         if (tmrModResult == pdPASS)
            result = true;
      }
   }

   return result;
}

bool SevenSegDynDummy::end(){
   bool result {false};
   //FFDR Check for the blink and the WAIT (specially the wait) timers to stop them if they are running to avoid funny combinations

   if(_dynDummyDspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      xTimerStop(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
      xTimerDelete(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
      _dynDummyDspRfrshTmrHndl = NULL;
   }


   Serial.println("=======================================");
   Serial.println("Seven Segment Dynamic Dummy Display End");
   Serial.end();

   return result;
}

void SevenSegDynDummy::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

   Serial.print("\nDynamic Dumb Display refreshing. Timestamp: ");
   Serial.println(millis(), DEC);
   Serial.println("--------------------------------------------------------");

   for (int i {0}; i < _dspDigitsQty; i++){
      tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      send(tmpDigToSend, /*uint8_t(1) <<*/ *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));

   }
   Serial.println("\n\rRefreshing complete.");
   Serial.println("--------------------");

   ++_firstRefreshed;
   if (_firstRefreshed == _dspDigitsQty)
      _firstRefreshed = 0;

   return;
}

void SevenSegDynDummy::send(const uint8_t &segments, const uint8_t &port){
   //FFDR serial.print() text indicating which sengment and what content is sending to each
   Serial.print("Pos.: ");
   Serial.print(port, DEC);
   Serial.print(", Cont.: ");
   Serial.print(segments, HEX);
   Serial.print("// ");
   return;
}

void SevenSegDynDummy::tmrCbRfrshDynDummy(TimerHandle_t rfrshTmrCbArg){
   SevenSegDynDummy* SevenSegUndrlDummy = (SevenSegDynDummy*) pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling this display's refresh() method
   
   SevenSegUndrlDummy->refresh();

   return;
}

//============================================================> Class methods separator

/*
SevenSegStatic::SevenSegStatic() {}

SevenSegStatic::~SevenSegStatic() {}
*/

//============================================================> Class methods separator

/*
SevenSegTM1637::SevenSegTM1637() {}

SevenSegTM1637::~SevenSegTM1637() {}
*/

//============================================================> Class methods separator

/*
SevenSegStatHC595::SevenSegStatHC595() {}

SevenSegStatHC595::~SevenSegStatHC595() {}
*/

//============================================================> Class methods separator

/*
SevenSegStatDummy::SevenSegStatDummy(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
{
   _ioPins = ioPins;
   _dspDigitsQty = dspDigits;
   _commAnode = commAnode;
   Serial.begin(9600);
}

SevenSegStatDummy::~SevenSegStatDummy(){}
*/