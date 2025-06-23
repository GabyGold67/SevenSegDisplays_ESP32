/**
 ******************************************************************************
 * @file SevenSegDispHw.cpp
 * @brief Code file for the SevenSegDisplays_ESP32 library
 * 
 * @details This code files includes the SevenSegDispHw class and subclasses, including one class for each specific **Display controller** managed by the library.  
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
 * @version 3.3.0  
 * 
 * @date First release: 20/12/2023  
 *       Last update:   21/06/2025 18:20 (GMT+0200) DSP
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
 */
#include <Arduino.h>
#include <./SevenSegDispHw/SevenSegDispHw.h>
//----------------------------------------->> Global constants declaration BEGIN
//------------------------------------------->> Global constants declaration END

//-------------------------------------->> Static variables initialization BEGIN
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
//---------------------------------------->> Static variables initialization END

//============================================================> Class methods separator

SevenSegDispHw::SevenSegDispHw() {}

SevenSegDispHw::SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspDigitsQtyMax)
:_ioPins{ioPins}, _digitPosPtr{new uint8_t[dspDigits]}, _dspDigitsQty {dspDigits}, _commAnode {commAnode},_dspDigitsQtyMax {(dspDigitsQtyMax>0)?dspDigitsQtyMax:dspDigits}
{
   _dspHwInstNbr = _dspHwSerialNum++;
   _dspHwInstance = this;
   _allLedsOff = (_commAnode)?0xFF:0x00;

   if(_dspDigitsQty > _dspDigitsQtyMax)   //!> Warning, this may instantiate a non expected size display, instead of simply failing the construction
      _dspDigitsQty = _dspDigitsQtyMax;

   for (uint8_t i{0}; i < _dspDigitsQty; i++)
      *(_digitPosPtr + i) = i;

   _xcdDspDigitsQty = _dspDigitsQtyMax - _dspDigitsQty;
   if(_xcdDspDigitsQty > 0){
      _xcdDspBuffPtr = new uint8_t[_xcdDspDigitsQty];
      memset(_xcdDspBuffPtr, _allLedsOff, _xcdDspDigitsQty);
   }
}

SevenSegDispHw::~SevenSegDispHw() {
   end();
   if(_dspBlankBuffPtr != nullptr)
      delete [] _dspBlankBuffPtr;   
   if(_digitPosPtr != nullptr)
      delete [] _digitPosPtr;
   if(_xcdDspBuffPtr != nullptr)
      delete [] _xcdDspBuffPtr;
}

bool SevenSegDispHw::begin(uint32_t updtLps){
   turnOn();

   return true;
}

bool SevenSegDispHw::end(){
   if(_isOn){  // Execute subset of turnOff()
      memset(_dspBuffPtr, _allLedsOff, _dspDigitsQty);
      ntfyUpdDsply();
      _isOn = false;
   }

   return true;
}

uint8_t SevenSegDispHw::getBrghtnssLvl(){
   
   return _brghtnssLvl;
}

uint8_t SevenSegDispHw::getBrghtnssMaxLvl(){
   
   return _brghtnssLvlMax;
}

uint8_t SevenSegDispHw::getBrghtnssMinLvl(){
 
   return _brghtnssLvlMin;
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

bool SevenSegDispHw::getIsOn(){
   
   return _isOn;
}

uint8_t* SevenSegDispHw::getxcdDspBuffPtr(){

   return _xcdDspBuffPtr;
}

void SevenSegDispHw::ntfyUpdDsply(){

   return;
}

bool SevenSegDispHw::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   
   return false;
} 

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
   bool result{true};

   for(int i {0}; i < _dspDigitsQty; i++){ // First test: all values in *newOrderPtr are in valid range
      if (*(newOrderPtr + i) >= _dspDigitsQtyMax){
         result = false;
         break;
      }   
   }
   if(result){ // Second test: verify the positions in the newOrderPtr are never repeated
      uint8_t* posUseChkArry = new uint8_t [_dspDigitsQtyMax];
      memset(posUseChkArry, 0X00, _dspDigitsQtyMax);
      for(int i {0}; i < _dspDigitsQty; i++){
         if (*(posUseChkArry + *(newOrderPtr + i)) == 0x00){
            *(posUseChkArry + *(newOrderPtr + i)) = 0x01;   // Mark position as used
         }
         else{ // The position was already set to be used, check failed, leave method
            result = false;
            break;
         }            
      }
      delete [] posUseChkArry;
   }
   if(result)
      memcpy(_digitPosPtr, newOrderPtr, _dspDigitsQty);

   //FFDR Add the re-construction of the _xcdDspBuffPtr array including all unused positions, add mechanisms to assign specific functions to those positions: Colons, apostrophes, etc.

   return result;
}

void SevenSegDispHw::setDspBuffPtr(uint8_t* newDspBuffPtr){
   _dspBuffPtr = newDspBuffPtr;
   _dspBuffPtrBkp = newDspBuffPtr;

   return;
}

void SevenSegDispHw::turnOff(){
   if(_isOn){
      if(_dspBlankBuffPtr == nullptr){
         _dspBlankBuffPtr = new uint8_t[_dspDigitsQty];
         memset(_dspBlankBuffPtr, _allLedsOff, _dspDigitsQty);
      }
      _dspBuffPtr = _dspBlankBuffPtr;
      ntfyUpdDsply();
      _isOn = false;
   }

   return;
}

void SevenSegDispHw::turnOn(){
   if(!_isOn){
      if(_dspBlankBuffPtr != nullptr){
         _dspBuffPtr = _dspBuffPtrBkp;
         delete [] _dspBlankBuffPtr;
         _dspBlankBuffPtr = nullptr;
      }
      ntfyUpdDsply();
      _isOn = true;
   }
   
   return;
}

void SevenSegDispHw::turnOn(const uint8_t &newBrghtnssLvl){
   setBrghtnssLvl(newBrghtnssLvl);
   turnOn();

   return;
}

//============================================================> Class methods separator

SevenSegDynamic::SevenSegDynamic(){}

SevenSegDynamic::SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspDigitsQtyMax)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{
   String dspSerialNumStr {"000" + String(_dspHwInstNbr)};
   dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
   _rfrshTmrName = "DynDsp" + dspSerialNumStr + "rfrsh_tmr";
}

SevenSegDynamic::~SevenSegDynamic(){}

bool SevenSegDynamic::begin(uint32_t updtLps){

   return true;
}

bool SevenSegDynamic::end() {

   return true;
}

void SevenSegDynamic::_refresh(){

    return;
}

void SevenSegDynamic::tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg){

   return;
}

//============================================================> Class methods separator

SevenSegDynHC595::SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(ioPins, dspDigits, commAnode, _dspDigitsQtyMax)
{    
   _sclk = *(ioPins + _sclkIndx);
   _rclk = *(ioPins + _rclkIndx);
   _dio = *(ioPins + _dioIndx);    
}

SevenSegDynHC595::~SevenSegDynHC595(){
   BaseType_t tmrModResult {pdFAIL};

   if(_dynHC595DspRfrshTmrHndl){
      end();
      tmrModResult = xTimerDelete(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         _dynHC595DspRfrshTmrHndl = NULL;
      }
   }
   delete _drvrShftRegPtr;
   delete _drvrShftRegSndPtr;
}

bool SevenSegDynHC595::begin(uint32_t updtLps){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   _drvrShftRegPtr = new ShiftRegGPIOXpander(_dio, _sclk, _rclk, 2);
   _drvrShftRegSndPtr = new uint8_t[2];
   _drvrShftRegPtr->begin();

   _firstRefreshed = 0;   
   if (!_dynHC595DspRfrshTmrHndl){  //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)      
      _dynHC595DspRfrshTmrHndl = xTimerCreate(  //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
         _rfrshTmrName.c_str(),   // Timer human readable name
         pdMS_TO_TICKS((updtLps > 0)?updtLps:((int)(1000/(30 * _dspDigitsQty)))),
         pdTRUE,  // Autoreload
         this,   // TimerID, data to be passed to the callback function
         tmrCbRfrshDynHC595  //Callback function
      );
   }
   if(_dynHC595DspRfrshTmrHndl != NULL){
      if(!xTimerIsTimerActive(_dynHC595DspRfrshTmrHndl)){
         tmrModResult = xTimerStart(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
         if (tmrModResult == pdPASS)
            result = true;
      }
      else{
         result = true;
      }   
   }
   turnOn();

   return result;
}

bool SevenSegDynHC595::end() {
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};
   
   SevenSegDispHw::end();
   if(_dynHC595DspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      tmrModResult = xTimerStop(_dynHC595DspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         result = true;
      }   
   }

   return result;
}

void SevenSegDynHC595::_refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

   for (int i {0}; i < _dspDigitsQty; i++){
      tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      *(_drvrShftRegSndPtr + 0) = uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty));
      *(_drvrShftRegSndPtr + 1) = tmpDigToSend;
      _drvrShftRegPtr->stampOverMain(_drvrShftRegSndPtr);

   }
   ++_firstRefreshed;
   if (_firstRefreshed == _dspDigitsQty)
      _firstRefreshed = 0;

   return;
}

void SevenSegDynHC595::tmrCbRfrshDynHC595(TimerHandle_t rfrshTmrCbArg){
   //Timer Callback to keep the display lit by calling this display's refresh() method
   SevenSegDynHC595* SevenSegUndrlHC595 = (SevenSegDynHC595*) pvTimerGetTimerID(rfrshTmrCbArg);
   SevenSegUndrlHC595->_refresh();

   return;
}

void SevenSegDynHC595::_unAbstract() {return;}

//============================================================> Class methods separator

SevenSegDynDummy::SevenSegDynDummy(uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(nullptr, dspDigits, commAnode)
{
}

SevenSegDynDummy::~SevenSegDynDummy(){
   BaseType_t tmrModResult {pdFAIL};

   if(_dynDummyDspRfrshTmrHndl){
      end();
      tmrModResult = xTimerDelete(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         _dynDummyDspRfrshTmrHndl = NULL;
      }
   }
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
      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
      _dynDummyDspRfrshTmrHndl = xTimerCreate(
         _rfrshTmrName.c_str(),   // Timer human readable name
         pdMS_TO_TICKS((updtLps > 0)?updtLps:2000),
         pdTRUE,  // Autoreload
         this,   // TimerID, data to be passed to the callback function
         tmrCbRfrshDynDummy  //Callback function
      );
   }
   if(_dynDummyDspRfrshTmrHndl != NULL){
      if(!xTimerIsTimerActive(_dynDummyDspRfrshTmrHndl)){
         tmrModResult = xTimerStart(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
         if (tmrModResult == pdPASS)
            result = true;
      }
      else{
         result = true;
      }
   }
   turnOn();

   return result;
}

bool SevenSegDynDummy::end(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   SevenSegDispHw::end();
   if(_dynDummyDspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
      tmrModResult = xTimerStop(_dynDummyDspRfrshTmrHndl, portMAX_DELAY);
      if(tmrModResult == pdPASS){
         result = true;
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

void SevenSegDynDummy::turnOff(){
   SevenSegDispHw::turnOff();
   Serial.println("\n\rSeven Segment Dynamic Dummy Display Turned Off");

   return;
}

void SevenSegDynDummy::turnOn(){
   SevenSegDispHw::turnOn();
   Serial.println("\n\rSeven Segment Dynamic Dummy Display Turned On");

   return;
}

void SevenSegDynDummy::_unAbstract(){return;}

//============================================================> Class methods separator

SevenSegStatic::SevenSegStatic(){}

SevenSegStatic::SevenSegStatic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspDigitsQtyMax)
:SevenSegDispHw(ioPins, dspDigits, commAnode, dspDigitsQtyMax)
{   
}

SevenSegStatic::~SevenSegStatic() {}

//============================================================> Class methods separator

SevenSegStatHC595::SevenSegStatHC595(){}

SevenSegStatHC595::SevenSegStatHC595(uint8_t *ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegStatic(ioPins, dspDigits, commAnode, dspDigits)
{
   _sclk = *(ioPins + _sclkIndx);
   _rclk = *(ioPins + _rclkIndx);
   _dio = *(ioPins + _dioIndx);
    
}

SevenSegStatHC595::~SevenSegStatHC595() {}

bool SevenSegStatHC595::begin(uint32_t updtLps){
   _dsplyHwShftRegPtr = new ShiftRegGPIOXpander(_dio, _sclk, _rclk, _dspDigitsQty);
   _lclDspBuffPtr = new uint8_t[_dspDigitsQty];
   _dsplyHwShftRegPtr->begin();

   return true;
}

void SevenSegStatHC595::ntfyUpdDsply(){
   _updDsplyCntnt();

   return;
}

void SevenSegStatHC595::_unAbstract(){return;}

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

SevenSegTM163X::SevenSegTM163X(){}

SevenSegTM163X::SevenSegTM163X(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspDigitsQtyMax)
:SevenSegStatic(ioPins, dspDigits, commAnode, dspDigitsQtyMax)
{
   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMax;

   _lclDspBuffPtr = new uint8_t[_dspDigitsQty]; //FFDR The display buffer shared with SevenSegDisplays need no manipulation, use mutex to avoid overwriting while operating it from both sides and eliminate this uneeded buffer

   _clk = *(ioPins + _clkIndx);
   _dio = *(ioPins + _dioIndx);
}

SevenSegTM163X::~SevenSegTM163X(){
   end();
   delete [] _lclDspBuffPtr;
}

bool SevenSegTM163X::begin(uint32_t updtLps){
   digitalWrite(_clk, LOW);
   digitalWrite(_dio, LOW);
   pinMode(_clk, OUTPUT);
   pinMode(_dio, OUTPUT);
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

void SevenSegTM163X::ntfyUpdDsply(){
   _updLclBffrCntnt();
   _sendBffr();

   return;
}

void SevenSegTM163X::_sendBffr(){
	/* 
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
	 *   |   |  | Address autoinc./Fixed:  0
	 *   |   |  Normal/Test mode (0/1):   0
	 *   |   N/C:                       00
	 *   Data command setting:        01
	 *   Command1:                  0b01000000 = 0x40 
	 *
	 * >> - Command2: Address command setting, for TM1637 and TM1639 is 0xC0, 6 consecutive addresses for TM1637, 16 for TM1639
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- -------
	 *   |   |     |
	 *   |   |     00H:     0000 First address of the data register to write to
	 *   |   N/C:         00
	 *   Address comm.: 11
	 *                0b11000000 = 0xC0
	 *
	 * >> Buffer contents: 4 ~ 16 bytes data sequence
	 *
	 * Command3: Display control
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- - -----
	 *   |   |  |   |
	 *   |   |  |   Brightness control:     000~111
	 *   |   |  Display switch On/Off(1/0):1
	 *   |   N/C:                        00
	 *   Display Control:              10
	 *                               0b1000XXXX -> 0x8F Display On, maximum brightness
	 */

   _sendByte(0x40);  // TM163X_COMM1: 40H -> address is automatically incremented by 1 mode (44H -> fixed address mode)

   _txStart();
   _txWrByte(0xC0);  // Set the first display's buffer address
   _txAsk();

   for(uint8_t i{0}; i < _dspDigitsQty ; i++){  // Send the contents for the visible standard display ports
      _txWrByte(*(_lclDspBuffPtr + i));
      _txAsk();
   }
   for(uint8_t i{_dspDigitsQty}; i < _dspDigitsQtyMax ; i++){// Send the contents for the non visible display ports
      _txWrByte(*(_xcdDspBuffPtr + i));
      _txAsk();
   }

   _txStop();

   return;
}

bool SevenSegTM163X::_sendByte(uint8_t data){
   bool result{false};

   _txStart();
   _txWrByte(data);
   _txAsk();
   _txStop();

   return result;
}
   
bool SevenSegTM163X::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   bool result{false};

   if((newBrghtnssLvl >=_brghtnssLvlMin) && (newBrghtnssLvl<=_brghtnssLvlMax)){
      if(newBrghtnssLvl != _brghtnssLvl){
         _sendByte((_isOn?0x88:0x80) | newBrghtnssLvl);  // Keep display on/of state, set new brightness
         _brghtnssLvl = newBrghtnssLvl;
      }
      result = true;
   }

   return result;
}

void SevenSegTM163X::turnOff(){
   if(_isOn){
      _sendByte(0b10000000 | _brghtnssLvl);  // Close display(0x80), keep brightness
      _isOn = false;
   }

   return;
}

void SevenSegTM163X::turnOn(){
   if(!_isOn){
      _sendByte(0b10001000 | _brghtnssLvl);  // Open display(0x88), keep brightness
      _isOn = true;
   }

   return;
}

void SevenSegTM163X::turnOn(const uint8_t &newBrghtnssLvl){
   if(newBrghtnssLvl != _brghtnssLvl)
      setBrghtnssLvl(newBrghtnssLvl);
   if(!_isOn)
      turnOn();

   return;
}

void SevenSegTM163X::_txAsk(){   // void I2Cask (void)
   pinMode(_dio, INPUT);
   digitalWrite(_clk, LOW);
   delayMicroseconds(5*_txClkTckTm);
   while(digitalRead(_dio)){
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
   uint8_t  mask{0x01};
   
   for(uint8_t i{0}; i < 8; i++){
      digitalWrite(_clk, LOW);
      digitalWrite(_dio, (data & mask)?HIGH:LOW);
      delayMicroseconds(3 * _txClkTckTm);
      mask = mask << 1;
      digitalWrite(_clk, HIGH);
      delayMicroseconds(3 * _txClkTckTm);
   }
   
	return;
}

void SevenSegTM163X::_updLclBffrCntnt(){
   uint8_t dspBuffPtrOffset{0};

   //FFDR Possibly use a mutex to protect the _dspBuffPtr from being changed while this method is running
   for (int i {0}; i < _dspDigitsQty; i++){
      dspBuffPtrOffset = *(_digitPosPtr + i);
      *(_lclDspBuffPtr + i) = *(_dspBuffPtr + dspBuffPtrOffset);
   }

   return;
}

//============================================================> Class methods separator

SevenSegTM1636::SevenSegTM1636(){}

SevenSegTM1636::SevenSegTM1636(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, _dspDigitsQtyMax)
{
}

SevenSegTM1636::~SevenSegTM1636(){}

void SevenSegTM1636::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegTM1637::SevenSegTM1637(){};

SevenSegTM1637::SevenSegTM1637(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, _dspDigitsQtyMax)
{
}

SevenSegTM1637::~SevenSegTM1637(){}

void SevenSegTM1637::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegTM1639::SevenSegTM1639(){}

SevenSegTM1639::SevenSegTM1639(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, _dspDigitsQtyMax)
{
}

SevenSegTM1639::~SevenSegTM1639(){}

void SevenSegTM1639::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegMax7219::SevenSegMax7219(){}

SevenSegMax7219::SevenSegMax7219(uint8_t* ioPins, uint8_t dspDigits)
:SevenSegStatic(ioPins, dspDigits, false, _dspDigitsQtyMax)
{
   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMin;

   _lclDspBuffPtr = new uint8_t[_dspDigitsQty]; //FFDR this local buffer is needed as it keeps the shared buffer contents into MAX7219 format, protect the shared buffer with mutex to avoid changing while loading
   
   _clk = *(ioPins + _clkIndx);
   _din = *(ioPins + _dinIndx);
   _cs = *(ioPins + _csIndx);
}

SevenSegMax7219::~SevenSegMax7219(){
   end();
   delete [] _lclDspBuffPtr;
}

bool SevenSegMax7219::begin(uint32_t updtLps){
   pinMode(_clk, OUTPUT);
   pinMode(_din, OUTPUT);
   pinMode(_cs, OUTPUT);
   digitalWrite(_clk, LOW);
   digitalWrite(_din, LOW);
   digitalWrite(_cs, HIGH);

   _sendAddrData(_DspTestAddr, _NormalOp, true);   // Set Not in test mode!
   _sendAddrData(_ScanLimitAddr, (_dspDigitsQty - 1), true);   //!< Set Scan Limit: Register data 0x00 to 0x07: quantity of digits to keep updated
   _sendAddrData(_DecodeModeAddr, _NoDecode, true);   // Set No Decode format
   setBrghtnssLvl(_brghtnssLvlMax);
   turnOn();

	return true;
}

uint8_t SevenSegMax7219::_cnvrtStdDgtTo72xxDgt(const uint8_t &stdDgt){
   uint8_t result {0x00};

   for(uint8_t i {0}; i < 7; i++) {
      if((stdDgt & (1 << (6 - i))) != 0)
         result |=  (1 << i);
   }
   result |= (stdDgt & 0x80);

   return result;
}

bool SevenSegMax7219::end()
{
   turnOff();

   return true;
}

bool SevenSegMax7219::getIsOn(){

   return _isOn;
}

void SevenSegMax7219::ntfyUpdDsply(){
   _updLclBffrCntnt();
   _sendBffr();

   return;
}

void SevenSegMax7219::_sendByte(const uint8_t &val, const  bool &MSbFrst) {
   for(uint8_t i {0}; i < 8; i++) {
      if(MSbFrst)
         digitalWrite(_din, (val & (1 << (7 - i)))?HIGH:LOW);
      else
         digitalWrite(_din, (val & (1 << i))?HIGH:LOW);
      digitalWrite(_clk, HIGH);
      digitalWrite(_clk, LOW);
   }

   return;
}

void SevenSegMax7219::_sendAddrData(const uint8_t &address, const uint8_t &data, const  bool &MSbFrst){
   digitalWrite(_cs, LOW);
   _sendByte(address, MSbFrst);
   _sendByte(data, MSbFrst);
   digitalWrite(_cs, HIGH);

   return;
}

void SevenSegMax7219::_sendBffr(){
   for(uint8_t dspPrt{0}; dspPrt < _dspDigitsQty; dspPrt++){
      _sendAddrData((_DspPortsBaseAddr + dspPrt), *(_lclDspBuffPtr + dspPrt), true);
   }

   return;
}

bool SevenSegMax7219::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   bool result{false};

   if((newBrghtnssLvl >= _brghtnssLvlMin) && (newBrghtnssLvl <= _brghtnssLvlMax)){
      if(newBrghtnssLvl != _brghtnssLvl){
         _sendAddrData(_BrghtnsSettAddr, newBrghtnssLvl, true);   
         _brghtnssLvl = newBrghtnssLvl;
      }
      result = true;
   }

   return result;
}

void SevenSegMax7219::turnOff(){
   if(_isOn){
      _sendAddrData(_ShutDownAddr, _TurnOff, true);   
      _isOn = false;
   }

   return;
}

void SevenSegMax7219::turnOn(){
   if(!_isOn){
      _sendAddrData(_ShutDownAddr, _TurnOn, true);   
      _isOn = true;
   }

   return;
}

void SevenSegMax7219::turnOn(const uint8_t &newBrghtnssLvl){
   setBrghtnssLvl(newBrghtnssLvl);
   turnOn();

   return;
}

void SevenSegMax7219::_updLclBffrCntnt(){
   for(uint8_t i{0}; i < _dspDigitsQty; i++)
      *(_lclDspBuffPtr + i) = _cnvrtStdDgtTo72xxDgt(*(_dspBuffPtr + *(_digitPosPtr + i)));

	return;
}

void SevenSegMax7219::_unAbstract(){return;}

//============================================================> Class methods separator

SevenSegHT16K33::SevenSegHT16K33(){};

SevenSegHT16K33::SevenSegHT16K33(uint8_t* ioPins, uint8_t dspDigits, uint8_t i2cAddress)
:SevenSegStatic(ioPins, dspDigits, false, _dspDigitsQtyMax), _i2cAddress{i2cAddress}
{
   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMin;

   _lclDspBuffPtr = new uint8_t[16]; //FFDR this local buffer is needed as it keeps the shared buffer contents mapped into real HT16K33 wired format, protect the shared buffer with mutex to avoid changing while loading

   if(ioPins == nullptr){
      _sda = SDA;
      _scl = SCL;
   }
   else{
      _scl = *(ioPins + _sclIndx);
      _sda = *(ioPins + _sdaIndx);
   }
}

SevenSegHT16K33::~SevenSegHT16K33(){
   end();
   delete [] _lclDspBuffPtr;
};

bool SevenSegHT16K33::begin(uint32_t updtLps){
   bool result {false};

   // Wire.begin(static_cast<int>(_sda), static_cast<int>(_scl));
   // Wire.setClock(_i2cRate);
   Wire.begin(static_cast<int>(_sda), static_cast<int>(_scl), _i2cRate);
   _sendCmmnd(_ExitStndBy);  // Activate display
   _sendCmmnd(_SetRowOtpt);  // Display Row/Int output pin = Row
   setBrghtnssLvl(_brghtnssLvlMax); // Set display brightness level to maximum
   _lclClear();
   turnOn();

   result = true;

   return result;
}

bool SevenSegHT16K33::end(){
   Serial.println("Entering end()");
   Serial.println("================");

   _lclClear();
   #ifdef WIRE_HAS_END
      Wire.end();   // Set I2C port Inactive
   #endif

   Serial.println("Exiting end()");
   Serial.println("=============");

   return true;
}

void SevenSegHT16K33::_lclClear(){
   int result{0};

   Wire.beginTransmission(_i2cAddress); 
   Wire.write(_DspPortsBaseAddr); 
   for(uint8_t mssgPtrOffset{0}; mssgPtrOffset <= 0x0F; mssgPtrOffset++)
      Wire.write(0x00);   // Single data byte to send
   result = Wire.endTransmission(true);
   
   return;
}

void SevenSegHT16K33::ntfyUpdDsply(){
   _updLclBffrCntnt();
   _sendBffr();

   return;
}

void SevenSegHT16K33::_sendBffr(){
   int result{0};

   Wire.beginTransmission(_i2cAddress); 
   Wire.write(_DspPortsBaseAddr); 
   for(uint8_t mssgPtrOffset{0}; mssgPtrOffset < 16; mssgPtrOffset++){
      Wire.write(*(_lclDspBuffPtr + mssgPtrOffset));   // Single data byte to send
   }
   result = Wire.endTransmission(true);
   
   return ;
}

bool SevenSegHT16K33::_sendCmmnd(uint8_t cmmnd){
   int result{0};

   Wire.beginTransmission(_i2cAddress);  
   Wire.write(cmmnd);   // Single data byte to send
   result = Wire.endTransmission(true);
   // result = Wire.endTransmission();

   return (result == 0);
}

 bool SevenSegHT16K33::_sendPrtData(uint8_t prt, uint8_t data){
   int result{0};

   Wire.beginTransmission(_i2cAddress); 
   Wire.write(_DspPortsBaseAddr + prt); 
   Wire.write(data);   // Single data byte to send
   result = Wire.endTransmission(true);
   
   return (result == 0);
}

bool SevenSegHT16K33::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   bool result{false};
   uint8_t mssgData{0};

   if((newBrghtnssLvl >= _brghtnssLvlMin) && (newBrghtnssLvl <= _brghtnssLvlMax)){
      if(newBrghtnssLvl != _brghtnssLvl){
         mssgData = _SetBrghtnssCmd | newBrghtnssLvl;
         result = _sendCmmnd(mssgData);   
         if(result)
            _brghtnssLvl = newBrghtnssLvl;
      }
      else
         result = true;
   }

   return result;
}

void SevenSegHT16K33::turnOff(){
   if(_isOn){
      if(_sendCmmnd(_TurnOffDsp))
         _isOn = false;
   }

   return;
}

void SevenSegHT16K33::turnOn(){
   if(!_isOn){
      if(_sendCmmnd(_TurnOnBlnkNo))
         _isOn = true;
   }
   
   return;
}

void SevenSegHT16K33::turnOn(const uint8_t &newBrghtnssLvl){
   setBrghtnssLvl(newBrghtnssLvl);
   turnOn();

   return;
}

void SevenSegHT16K33::_unAbstract(){return;}

void SevenSegHT16K33::_updLclBffrCntnt(){
   uint8_t lclDspBuffPtrOffset{0};

   memset(_lclDspBuffPtr, 0x00, 16); // Clear the local buffer
   for (int i {0}; i < _dspDigitsQty; i++){
      lclDspBuffPtrOffset = (*(_digitPosPtr + i)*2);
      *(_lclDspBuffPtr + lclDspBuffPtrOffset) = *(_dspBuffPtr + i);
   }

   return;
}

//============================================================> Class methods separator
