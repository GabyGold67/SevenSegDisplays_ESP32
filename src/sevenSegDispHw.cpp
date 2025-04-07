/**
 ******************************************************************************
 * @file SevenSegDispHw.cpp
 * @brief Code file for the SevenSegDisplays_ESP32 library, SevenSegDispHw class and subclasses 
 * 
 * @details  
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
 *       Last update:   25/03/2025 11:20 (GMT+0200)  
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
 */
#include "Arduino.h"
#include "sevenSegDispHw.h"
//----------------------------------------->> Global constants declaration BEGIN
const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t stdLtoRx4 [4] {0, 1, 2, 3};
const uint8_t stdRtoLx4 [4] {3, 2, 1, 0};
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
   _dspHwInstance = this;
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

void SevenSegDispHw::ntfyUpdDsply(){

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

   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
   if (!_svnSgDynTmrHndl){
      //Create a valid unique Name for identifying the timer created
      String rfrshTmrName{""};
      String dspSerialNumStr {"000" + String(_dspHwInstNbr)};
      dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
      rfrshTmrName = "DynDsp" + dspSerialNumStr + "rfrsh_tmr";

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

void SevenSegDynamic::_refresh(){
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

SevenSegDynHC595::~SevenSegDynHC595(){
   if(_dynHC595DspRfrshTmrHndl)
      end();
   delete _drvrShftRegPtr;
   delete _drvrShftRegSndPtr;
}

bool SevenSegDynHC595::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

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
         pdMS_TO_TICKS((updtLps > 0)?updtLps:((int)(1000/(30 * _dspDigitsQty)))),
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

//FFDR Split this end() as the noWait(): 1)Check if the timer is running and just stop it. 2) Place the end() call at the destructor, and delete the timer if existed. Before that ensure the begin checks for existance and status before creating new timer.

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
bool SevenSegDynHC595::end() {
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};
   
   if(_dynHC595DspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      tmrModResult = xTimerStop(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         tmrModResult = xTimerDelete(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS){
            _dynHC595DspRfrshTmrHndl = NULL;
            result = true;
         }
      }   
   }

   return result;
}

void SevenSegDynHC595::_refresh(){
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

// void SevenSegDynHC595::send(uint8_t content){

//    return;
// }

// void SevenSegDynHC595::send(const uint8_t &segments, const uint8_t &port){

//    return;
// }

void SevenSegDynHC595::tmrCbRfrshDynHC595(TimerHandle_t rfrshTmrCbArg){
   //Timer Callback to keep the display lit by calling this display's refresh() method
   SevenSegDynHC595* SevenSegUndrlHC595 = (SevenSegDynHC595*) pvTimerGetTimerID(rfrshTmrCbArg);
   SevenSegUndrlHC595->_refresh();

   return;
}

void SevenSegDynHC595::_unAbstract() {

   return;
}

//============================================================> Class methods separator

SevenSegDynDummy::SevenSegDynDummy(uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(nullptr, dspDigits, commAnode)
{
   begin();
}

SevenSegDynDummy::~SevenSegDynDummy(){
   if(_dynDummyDspRfrshTmrHndl)
      end();

}

bool SevenSegDynDummy::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   Serial.begin(9600);
   Serial.println("Seven Segment Dynamic Dummy Display Begin");
   Serial.println("================================================");

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
         pdMS_TO_TICKS((updtLps > 0)?updtLps:2000),
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
   BaseType_t tmrModResult {pdFAIL};

   if(_dynDummyDspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      tmrModResult = xTimerStop(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         tmrModResult = xTimerDelete(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS){
            _dynDummyDspRfrshTmrHndl = NULL;
            result = true;
         }
      }   
   }
   if(result){
      Serial.println("=======================================");
      Serial.println("Seven Segment Dynamic Dummy Display End");
      Serial.end();   
   }

   return result;
}

void SevenSegDynDummy::_refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

   Serial.print("\nDynamic Dummy Display refreshing. Timestamp: ");
   Serial.println(millis(), DEC);
   Serial.println("--------------------------------------------------------");

   for (int i {0}; i < _dspDigitsQty; i++){
      tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      send(tmpDigToSend, *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));

   }
   Serial.println("\n\rRefreshing complete.");
   Serial.println("--------------------");

   ++_firstRefreshed;
   if (_firstRefreshed == _dspDigitsQty)
      _firstRefreshed = 0;

   return;
}

void SevenSegDynDummy::send(const uint8_t &segments, const uint8_t &port){
   Serial.print("Pos.: ");
   Serial.print(port, DEC);
   Serial.print(", Cont.: ");
   Serial.print(segments, HEX);
   Serial.print("// ");
   return;
}

void SevenSegDynDummy::tmrCbRfrshDynDummy(TimerHandle_t rfrshTmrCbArg){
   //Timer Callback to keep the dynamic display lit by calling this display's refresh() method
   SevenSegDynDummy* SevenSegUndrlDummy = (SevenSegDynDummy*) pvTimerGetTimerID(rfrshTmrCbArg);
   SevenSegUndrlDummy->_refresh();

   return;
}

void SevenSegDynDummy::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegStatic::SevenSegStatic(){}

SevenSegStatic::SevenSegStatic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{   
   Serial.println("\nSevenSegStatic constructor"); //FTPO
   Serial.println("==========================="); //FTPO
}

SevenSegStatic::~SevenSegStatic() {}

void SevenSegStatic::ntfyUpdDsply(){

   return;
}

//============================================================> Class methods separator

SevenSegStatHC595::SevenSegStatHC595(){}

SevenSegStatHC595::SevenSegStatHC595(uint8_t *ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegStatic(ioPins, dspDigits, commAnode)
{
   Serial.println("\nSevenSegStatHC595 constructor"); //FTPO
   Serial.println("==========================="); //FTPO

   _sclk = *(ioPins + _sclkIndx);
   _rclk = *(ioPins + _rclkIndx);
   _dio = *(ioPins + _dioIndx);
    
    _dsplyHwShftRegPtr = new ShiftRegGPIOXpander(_dio, _sclk, _rclk, _dspDigitsQty, nullptr);
    _lclDspBuffPtr = new uint8_t[_dspDigitsQty];
}

SevenSegStatHC595::~SevenSegStatHC595() {}

void SevenSegStatHC595::ntfyUpdDsply(){
   _updDsplyCntnt();

   return;
}

void SevenSegStatHC595::_unAbstract(){

   return;
}

void SevenSegStatHC595::_updDsplyCntnt(){
   uint8_t dspBuffPtrOffset{0};

   for (int i {0}; i < _dspDigitsQty; i++){
      dspBuffPtrOffset = *(_digitPosPtr + i);
      *(_lclDspBuffPtr + i) = *(_dspBuffPtr + dspBuffPtrOffset);
   }
   _dsplyHwShftRegPtr->stampOverMain(_lclDspBuffPtr);

   return;
}
//============================================================> Class methods separator

SevenSegTM163X::SevenSegTM163X()
{}

SevenSegTM163X::SevenSegTM163X(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegStatic(ioPins, dspDigits, commAnode)
{
   Serial.println("\nSevenSegTM163X constructor"); //FTPO
   Serial.println("==========================="); //FTPO

    _clk = *(ioPins + _clkIndx);
	 _dio = *(ioPins + _dioIndx);

   digitalWrite(_clk, LOW);
   digitalWrite(_dio, LOW);
   pinMode(_clk, OUTPUT);
   pinMode(_dio, OUTPUT);

   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMax;

   _lclDspBuffPtr = new uint8_t[_dspDigitsQty];
   begin();
}

SevenSegTM163X::~SevenSegTM163X()
{
   end();
   delete [] _lclDspBuffPtr;
}

bool SevenSegTM163X::begin()
{
   turnOn();

	return true;
}

bool SevenSegTM163X::end(){
   turnOff();

	return true;
}

uint8_t SevenSegTM163X::getBrghtnssLvl(){

   return _brghtnssLvl;
}

uint8_t SevenSegTM163X::getBrghtnssMaxLvl(){

   return _brghtnssLvlMax;
}

uint8_t SevenSegTM163X::getBrghtnssMinLvl(){

   return _brghtnssLvlMin;
}

void SevenSegTM163X::ntfyUpdDsply(){
   _updDsplyCntnt();
   _sendBffr();

   return;
}

void SevenSegTM163X::_sendBffr(){
	/* If it's low cost confirm the new buffer contents are different from the display content
	 * Create a message buffer according to the TM1637 I2C modified protocol:
	 * Invoke the send() method to output the message to the display
	 * Delete the message buffer
	 * >> SOT commands + buffer contents + EOT command
	 * SOT Commands: Command1 + Command2
	 * >> - Command1: Data command
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- - - ---
	 *   |   |  | |  |
	 *   |   |  | |  Data Write to display: 00
	 *   |   |  | Address auto-increment:  0
	 *   |   |  Normal/Test mode:         0
	 *   |   N/C:                      00
	 *   Data command setting:       01
	 *   Command1:                 0b01000000 = 0x40 
	 *
	 * >> - Command2: Address command setting, for TM1637 and TM1639 is 0xC0, 6 consecutive addresses for TM1637, 16 for TM1639
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- -------
	 *   |   |     |
	 *   |   |     00H:  0000 First address of the data register
	 *   |   N/C:      00
	 *   Add. comm.: 11
	 *             0b11000000 = 0xC0
	 *
	 * >> Buffer contents: 6 ~ 16 bytes data sequence
	 *
	 * >> EOT commands: Command3:
	 * Command3: Display control
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- - -----
	 *   |   |  |   |
	 *   |   |  |   Brightness control:    000~111
	 *   |   |  Display switch On/Off:    1/0
	 *   |   N/C:                       00
	 *   Display Control:             10
	 *                              0b1000XXXX -> 0x8F Display On, maximum brightness
	 */

   _txStart();
   _txWrByte(0x40);  // TM1637_COMM1: 40H -> address is automatically incremented by 1 mode (44H -> fixed address mode)
   _txAsk();
   _txStop();

   _txStart();
   _txWrByte(0xC0);  // Set the first address
   _txAsk();

   for(uint8_t i{0}; i < _dspDigitsQty ; i++){
      _txWrByte(*(_lclDspBuffPtr + i));
      _txAsk();
   }
   for(uint8_t i{_dspDigitsQty}; i < 6 ; i++){
      _txWrByte(0x00);
      _txAsk();
   }

   _txStop();

   return;
}

bool SevenSegTM163X::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   bool result{false};

   if((newBrghtnssLvl >=_brghtnssLvlMin)&&(newBrghtnssLvl<=_brghtnssLvlMax)){
      if(newBrghtnssLvl != _brghtnssLvl){
         _txStart();
         _txWrByte((_isOn?0x88:0x80) | newBrghtnssLvl);  // Keep display on/of state, set new brightness
         _txAsk();
         _txStop();
         _brghtnssLvl = newBrghtnssLvl;
      }
      result = true;
   }

   return result;
}

void SevenSegTM163X::turnOff(){
   if(_isOn){
      _txStart();
      _txWrByte(0b10000000 | _brghtnssLvl);  // Close display(0x80), keep brightness
      _txAsk();
      _txStop();
      _isOn = false;
   }

   return;
}

void SevenSegTM163X::turnOn(){
   if(!_isOn){
      _txStart();
      _txWrByte(0b10001000 | _brghtnssLvl);  // Open display(0x88), keep brightness
      _txAsk();
      _txStop();
      _isOn = true;
   }

   return;
}

void SevenSegTM163X::turnOn(const uint8_t &newBrghtnssLvl){
   if(!_isOn){
      if(newBrghtnssLvl != _brghtnssLvl){
         setBrghtnssLvl(newBrghtnssLvl);
      }
      turnOn();
   }

   return;
}

void SevenSegTM163X::_txAsk(){   // void I2Cask (void)
   pinMode(_dio, INPUT);

   digitalWrite(_clk, LOW);
   delayMicroseconds(5*_txClkTckTm);
   while (digitalRead(_dio)){
   }
   digitalWrite(_clk, HIGH);
   delayMicroseconds(2*_txClkTckTm);
   digitalWrite(_clk, LOW);
   
   pinMode(_dio, OUTPUT);

	return;
}

void SevenSegTM163X::_txStart(){ // void I2CStart(void)
   digitalWrite(_clk, HIGH);
   digitalWrite(_dio, HIGH);
   delayMicroseconds(2*_txClkTckTm);
   digitalWrite(_dio, LOW);

	return;
}

void SevenSegTM163X::_txStop(){  // void I2CStop (void)
   digitalWrite(_clk, LOW);
   delayMicroseconds(2*_txClkTckTm);
   digitalWrite(_dio, LOW);
   delayMicroseconds(2*_txClkTckTm);
   digitalWrite(_clk, HIGH);
   delayMicroseconds(2*_txClkTckTm);
   digitalWrite(_dio, HIGH);
   
	return;
}

void SevenSegTM163X::_txWrByte(uint8_t data){   // void I2CWrByte (unsigned char oneByte)
   for(uint8_t i{0}; i < 8; i++){
      digitalWrite(_clk, LOW);
      digitalWrite(_dio, (data &0x01)?HIGH:LOW); //Equivalent single line ternary operation
      delayMicroseconds(3*_txClkTckTm);
      data = data >> 1;
      digitalWrite(_clk, HIGH);
      delayMicroseconds(3*_txClkTckTm);
   }
   
	return;
}

void SevenSegTM163X::_updDsplyCntnt(){
   uint8_t dspBuffPtrOffset{0};
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   for (int i {0}; i < _dspDigitsQty; i++){
      dspBuffPtrOffset = *(_digitPosPtr + i);
      *(_lclDspBuffPtr + i) = *(_dspBuffPtr + dspBuffPtrOffset);
   }
   taskEXIT_CRITICAL(&mux);

   return;
}

//============================================================> Class methods separator

SevenSegTM1637::SevenSegTM1637(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode)
{
   Serial.println("\nSevenSegTM1637 constructor"); //FTPO
   Serial.println("================================"); //FTPO
}

SevenSegTM1637::~SevenSegTM1637(){}

void SevenSegTM1637::_unAbstract(){

   return;
}


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



template<typename T>
void pushElmnt(T* &elmntLstPtr, T elmntToPush, uint8_t &elmntQty){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	T* tmpArrPtr{nullptr};

   taskENTER_CRITICAL(&mux);
   if(elmntLstPtr == nullptr){	// There are no array previously created			
		elmntLstPtr = new T [1];
	}

   tmpArrPtr = new T [elmntQty + 1];
   for (int i{0}; i < elmntQty; ++i){
      *(tmpArrPtr + i) = *(elmntLstPtr + i);
   }
   *(tmpArrPtr + elmntQty) = elmntToPush;
   if(elmntLstPtr != nullptr)
      delete [] elmntLstPtr;
   elmntLstPtr = tmpArrPtr;
   elmntQty++;

   taskEXIT_CRITICAL(&mux);

   return;
}
