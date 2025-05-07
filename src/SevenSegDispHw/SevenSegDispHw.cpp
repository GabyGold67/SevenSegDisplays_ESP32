/**
 ******************************************************************************
 * @file SevenSegDispHw.cpp
 * @brief Code file for the SevenSegDisplays_ESP32 library
 * 
 * @details This code files includes the SevenSegDispHw class and subclasses, including one class for each specific **Display controller** managed by the library/  
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
 * @version 3.2.0  
 * 
 * @date First release: 20/12/2023  
 *       Last update:   27/04/2025 20:10 (GMT+0200) DSP
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

SevenSegDispHw::SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:_ioPins{ioPins}, _digitPosPtr{new uint8_t[dspDigits]}, _dspDigitsQty {dspDigits}, _commAnode {commAnode}
{
   _dspHwInstNbr = _dspHwSerialNum++;
   _dspHwInstance = this;
   for (uint8_t i{0}; i < _dspDigitsQty; i++)
      *(_digitPosPtr + i) = i;
   _allLedsOff = (_commAnode)?0xFF:0x00;
}

SevenSegDispHw::~SevenSegDispHw() {
   end();
   if(_dspBlankBuffPtr != nullptr)
      delete [] _dspBlankBuffPtr;   
   if(_digitPosPtr != nullptr)
      delete [] _digitPosPtr;
}

bool SevenSegDispHw::begin(uint32_t updtLps){
   turnOn();

   return true;
}

bool SevenSegDispHw::end(){
   // Execute subset of turnOff()
   if(_isOn){
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

void SevenSegDispHw::ntfyUpdDsply(){

   return;
}

/*void SevenSegDispHw::_send(uint8_t *digitsBuffer){

   return;
}

void SevenSegDispHw::_send(const uint8_t &segments, const uint8_t &port){

   return;
}

void SevenSegDispHw::_sendDataUnit(void* dataUnit){   //FFDR unify send methods using void*

   return;
}  

void SevenSegDispHw::_sendMssg(void* dataMssg){   //FFDR unify send methods using void*

   return;
}
*/
bool SevenSegDispHw::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   
   return false;
} 

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
   bool result{true};
   uint8_t maxDspPos{(_dspDigitsQtyMax == 0)?_dspDigitsQty:_dspDigitsQtyMax};

   for(int i {0}; i < _dspDigitsQty; i++){
      if (*(newOrderPtr + i) >= maxDspPos){
         result = false;
         break;
      }   
   }
   if(result){
      uint8_t* posUseChkArry = new uint8_t [maxDspPos];
      memset(posUseChkArry, 0X00, maxDspPos);
      for(int i {0}; i < _dspDigitsQty; i++){
         if (*(posUseChkArry + *(newOrderPtr + i)) = 0x00){
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

SevenSegDynamic::SevenSegDynamic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
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

/*void SevenSegDynamic::_send(uint8_t content){ // Implementation is hardware dependant (subclasses) protocol!!

   return;
}

void SevenSegDynamic::_send(const uint8_t &segments, const uint8_t &port){

   return;
}*/

void SevenSegDynamic::tmrCbRfrshDyn(TimerHandle_t rfrshTmrCbArg){

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

   _firstRefreshed = 0;
   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
   if (!_dynHC595DspRfrshTmrHndl){
      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
      _dynHC595DspRfrshTmrHndl = xTimerCreate(
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

void SevenSegDynHC595::_unAbstract() {

   return;
}

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

SevenSegStatic::SevenSegStatic(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{   
}

SevenSegStatic::~SevenSegStatic() {}

//============================================================> Class methods separator

SevenSegStatHC595::SevenSegStatHC595(){}

SevenSegStatHC595::SevenSegStatHC595(uint8_t *ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegStatic(ioPins, dspDigits, commAnode)
{
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

SevenSegTM163X::SevenSegTM163X(uint8_t* ioPins, uint8_t dspDigits, bool commAnode, uint8_t dspContMaxDigits)
:SevenSegStatic(ioPins, dspDigits, commAnode), _dspDigitsQtyMax{dspContMaxDigits}
{
   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMax;

   if(_dspDigitsQty > _dspDigitsQtyMax)
      _dspDigitsQty = _dspDigitsQtyMax;   //FFDR Move the checking to the base class constructor and allocate the correct amount of memory there!!
   _lclDspBuffPtr = new uint8_t[_dspDigitsQty]; //FFDR The display buffer shared with SevenSegDisplays need no manipulation, use mutex to avoid overwriting while operating it from both sides and eliminate this uneeded buffer
   _xcdDspDigitsQty = _dspDigitsQtyMax  - _dspDigitsQty;

   if(_xcdDspDigitsQty > 0){
      _xcdDspBuffPtr = new uint8_t[_xcdDspDigitsQty];
      memset(_xcdDspBuffPtr, 0X00, _xcdDspDigitsQty);
   }

   _clk = *(ioPins + _clkIndx);
   _dio = *(ioPins + _dioIndx);

   digitalWrite(_clk, LOW);
   digitalWrite(_dio, LOW);
   pinMode(_clk, OUTPUT);
   pinMode(_dio, OUTPUT);
}

SevenSegTM163X::~SevenSegTM163X(){
   end();
   delete [] _lclDspBuffPtr;
   if(_xcdDspBuffPtr != nullptr)
      delete [] _xcdDspBuffPtr;
}

bool SevenSegTM163X::begin(uint32_t updtLps){
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

SevenSegTM1636::SevenSegTM1636(){}

SevenSegTM1636::SevenSegTM1636(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, 4)
{
}

SevenSegTM1636::~SevenSegTM1636(){}

void SevenSegTM1636::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegTM1637::SevenSegTM1637(){};

SevenSegTM1637::SevenSegTM1637(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, 6)
{
}

SevenSegTM1637::~SevenSegTM1637(){}

void SevenSegTM1637::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegTM1639::SevenSegTM1639(){}

SevenSegTM1639::SevenSegTM1639(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegTM163X(ioPins, dspDigits, commAnode, 8)
{
}

SevenSegTM1639::~SevenSegTM1639(){}

void SevenSegTM1639::_unAbstract(){

   return;
}

//============================================================> Class methods separator

SevenSegMax7219::SevenSegMax7219(){}

SevenSegMax7219::SevenSegMax7219(uint8_t* ioPins, uint8_t dspDigits)
:SevenSegStatic(ioPins, dspDigits, false)
{
   _brghtnssLvlMax = _hwBrghtnssLvlMax;
   _brghtnssLvlMin = _hwBrghtnssLvlMin;
   _brghtnssLvl = _brghtnssLvlMin;

   if(_dspDigitsQty > _dspDigitsQtyMax)
      _dspDigitsQty = _dspDigitsQtyMax;   //FFDR Move the checking to the base class constructor and allocate the correct amount of memory there!!
   _lclDspBuffPtr = new uint8_t[_dspDigitsQty]; //FFDR this local buffer is needed as it keeps the shared buffer contents into MAX7219 format, protect the shared buffer with mutex to avoid changing while loading
   
   _clk = *(ioPins + _clkIndx);
   _din = *(ioPins + _dinIndx);
   _cs = *(ioPins + _csIndx);

   pinMode(_clk, OUTPUT);
   pinMode(_din, OUTPUT);
   pinMode(_cs, OUTPUT);
   digitalWrite(_clk, LOW);
   digitalWrite(_din, LOW);
   digitalWrite(_cs, HIGH);
}

SevenSegMax7219::~SevenSegMax7219()
{
   delete [] _lclDspBuffPtr;
}

bool SevenSegMax7219::begin(uint32_t updtLps)
{
   
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
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   digitalWrite(_cs, LOW);
   _sendByte(address, MSbFrst);
   _sendByte(data, MSbFrst);
   digitalWrite(_cs, HIGH);
   taskEXIT_CRITICAL(&mux);

   return;
}

void SevenSegMax7219::_sendBffr(){
   for(uint8_t dspPrt{0}; dspPrt < _dspDigitsQty; dspPrt++)
   _sendAddrData((_DspPortsBaseAddr + dspPrt), *(_lclDspBuffPtr + dspPrt), true);

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
   for(uint8_t dspPrt{0}; dspPrt < _dspDigitsQty; dspPrt++)
      *(_lclDspBuffPtr + dspPrt) = _cnvrtStdDgtTo72xxDgt(*(_dspBuffPtr + dspPrt));  //FFDR Add the buffPos indirection element, as this statement supports the idea of no digits reordering
   
	return;
}

void SevenSegMax7219::_unAbstract(){

   return;
}

//============================================================> Class methods separator
