/**
 * @file SevenSegDisplays.cpp
 * @brief Code file for the SevenSegDisplays_ESP32 library 
 * 
 * @details The library provides a common API and tools to generate and manage contents formatting for seven segments displays.
 * 
 * @author Gabriel D. Goldman
 * 
 * @version 3.0.0
 * 
 * @date First release: 20/12/2023 
 *       Last update:   13/03/2025 13:40 (GMT+0200)
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
 */
#include "sevenSegDisplays.h"

uint8_t SevenSegDisplays::_displaysCount = 0;
uint16_t SevenSegDisplays::_dspLastSerialNum = 0;
SevenSegDisplays** SevenSegDisplays::_ssdInstancesLstPtr = nullptr;

TimerHandle_t SevenSegDisplays::_blinkTmrHndl = NULL;
TimerHandle_t SevenSegDisplays::_waitTmrHndl = NULL;

SevenSegDisplays::SevenSegDisplays()
{
}

SevenSegDisplays::SevenSegDisplays(SevenSegDispHw* dspUndrlHwPtr)
:_dspUndrlHwPtr{dspUndrlHwPtr}
{
   _dspDigitsQty = _dspUndrlHwPtr->getDspDigits(); //With the display size in digits, the needed arrays for data can be built
   _dspBuffPtr  = new uint8_t[_dspDigitsQty];
   _blinkMaskPtr = new bool[_dspDigitsQty];
   _dspUndrlHwPtr->setDspBuffPtr(_dspBuffPtr); //Indicates the hardware where the data to display is located
   _dspInstNbr = _dspLastSerialNum++; //This value is always incremented, as it's not related to the active objects but to amount of different displays created
   ++_displaysCount;  //This keeps the count of instantiated SevenSegDisplays objects
   _dspInstance = this;
   _pushSsd(_ssdInstancesLstPtr, _dspInstance);
   _setAttrbts();
   clear();
}

SevenSegDisplays::~SevenSegDisplays(){
   if(_blinking)
      noBlink();  //Stops the blinking, frees the _dspAuxBuffPtr pointed memory, Stops the timer attached to the process
   if(_waiting)
      noWait();   //Stops the waiting, frees the _dspAuxBuffPtr pointed memory, Stops the timer attached to the process    
   if(_dspAuxBuffPtr)
      delete [] _dspAuxBuffPtr;   //Free the resources of the auxiliary display digits buffer (to keep a copy of the dspBuffer contents for blinking, waiting, etc.)
   delete [] _blinkMaskPtr;    //Free the resources of the blink mask buffer
   delete [] _dspBuffPtr;  //Free the resources of the display digits buffer
   _popSsd(_ssdInstancesLstPtr, _dspInstance);
   --_displaysCount;
}
//FFDR Checked 2025-03-14 to this point
bool SevenSegDisplays::blink(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(!_waiting){   //If the display is waiting the blinking option is blocked out as they are mutually excluyent, as both simultaneous has no logical use!
      if (!_blinking){
         //Create a valid unique Name for identifying the timer created
         char blnkTmrName[15];
         char dspSerialNumChar[3]{};
         sprintf(dspSerialNumChar, "%0.2d", (int)_dspLastSerialNum);
         strcpy(blnkTmrName, "Disp");
         strcat(blnkTmrName, dspSerialNumChar);
         strcat(blnkTmrName, "blnk_tmr");

         if (!_blinkTmrHndl){
            _blinkTmrHndl = xTimerCreate(
               blnkTmrName,
               pdMS_TO_TICKS(_blinkRatesGCD),
               pdTRUE,  //Autoreload
               _dspInstance,   //TimerID, data to be passed to the callback function
               tmrCbBlink  //Callback function
            );
         }
         if(_blinkTmrHndl && (!xTimerIsTimerActive(_blinkTmrHndl))){
            // The timer was created, but it wasn't started. Start the timer
            tmrModResult = xTimerStart(_blinkTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
               result = true;
         }

         _dspAuxBuffPtr = new uint8_t[_dspDigitsQty];
         _saveDspBuff();
         _blinkShowOn = false;
         _blinking = true;
         _blinkTimer = 0;  //Start the blinking pace timer...
         result = true;
      }
   }

   return result;
}

bool SevenSegDisplays::blink(const unsigned long &onRate, const unsigned long &offRate){
   bool result {false};

   if(!_waiting){
      if (!_blinking){
         if (offRate == 0)
            result = setBlinkRate(onRate, onRate);
         else
            result = setBlinkRate(onRate, offRate);        
         if (result)
            result = blink();
      }
   }

   return result;
}

unsigned long SevenSegDisplays::_blinkTmrGCD(unsigned long blnkOnTm, unsigned long blnkOffTm){
   /*returning values:
      0: One of the input values was 0
      1: No GDC greater than 1
      Other: This value would make the blink timer save resources by checking the blink time as less frequent as possible*/
   unsigned long result{ 0 };

   if ((blnkOnTm != 0) && (blnkOffTm != 0)) {
      if (blnkOnTm == blnkOffTm)
         result = blnkOnTm;
      else if ((blnkOnTm % blnkOffTm == 0) || (blnkOffTm % blnkOnTm == 0))
         result = (blnkOffTm < blnkOnTm)? blnkOffTm : blnkOnTm;

      if (result == 0) {
         for (unsigned long int i{ (blnkOnTm < blnkOffTm) ? blnkOnTm : blnkOffTm }; i > 0; i--) {
            if ((blnkOnTm % i == 0) && (blnkOffTm % i == 0)) {
               result = i;
               break;
            }
         }
      }
   }

   return result;
}

void SevenSegDisplays::clear(){
   //Cleans the contents of the internal display buffer (All leds off for all digits)
   if(!_waiting){ // If in waiting condition clearing makes no sense, the mechanism will keep displaying the sequence independently      
      if(_blinking){         
         //If the display is blinking the backup buffer will be restored, so the display clearing() would be reverted
         //So BOTH buffers must be cleared, starting by the _dspAuxBuff, and blocking the access to it while clearing takes place
         for (int i{0}; i < _dspDigitsQty; i++){
            if(*(_dspAuxBuffPtr + i) != _space)
               *(_dspAuxBuffPtr + i) = _space;
         }
      }
      for (int i{0}; i < _dspDigitsQty; i++){
         if(*(_dspBuffPtr + i) != _space){
            *(_dspBuffPtr + i) = _space;
            _dspBuffChng = true;    //Signal for the hardware refresh mechanism
         }
      }
   }
   
   return;
}

bool SevenSegDisplays::doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft, char labelRight){
   bool displayable{true};
   String readOut{""};
   String lvlChar{""};

   if ((levelLeft < 0) || (levelRight < 0) || (levelLeft > 3 || (levelRight > 3))) {
      clear();
      displayable = false;
   }
   else {
      readOut += labelLeft;
      if (readOut == "")
         readOut = " ";
         switch (levelLeft) {
            case 0:
               //  readOut += " ";
                lvlChar = " ";
                break;
            case 1:
               //  readOut += "_";
                lvlChar = "_";
                break;
            case 2:
               //  readOut += "=";
                lvlChar = "=";
                break;
            case 3:
               //  readOut += "~";
                lvlChar = "~";
                break;
         };
         readOut += lvlChar;

         if(_dspDigitsQty > 4){
            for (int i{0}; i < (_dspDigitsQty - 4)/2; i++)
               readOut += " ";
         }
        readOut += labelRight;
        if (readOut.length() == 2)
            readOut += " ";
        switch (levelRight) {
            case 0:
               //  readOut += " ";
               lvlChar = " ";
               break;
            case 1:
               //  readOut += "_";
               lvlChar = "_";
               break;
            case 2:
               //  readOut += "=";
               lvlChar = "=";
               break;
            case 3:
               //  readOut += "~";
               lvlChar = "~";
               break;
        };
        readOut += lvlChar;
        displayable = print(readOut);
    }

    return displayable;
}

bool SevenSegDisplays::gauge(const int &level, char label){
   bool displayable{true};
   String readOut{""};

   clear();
   if (((level < 0) || (level > 3)) || (_dspDigitsQty < 4)) {
      displayable = false;
   }
   else {
      readOut += label;
      if (readOut == "")
         readOut = " ";
      switch (level) {
         case 0:
            readOut += "   ";
            break;
         case 1:
            readOut += "_  ";
            break;
         case 2:
            readOut += "_= ";
            break;
         case 3:
            readOut += "_=~";
            break;
      };
      displayable = print(readOut);
   }

   return displayable;
}

bool SevenSegDisplays::gauge(const double &level, char label) {
   bool displayable{true};
   int intLevel{0};

   if (((level < 0.0) || (level > 1.0)) || (_dspDigitsQty < 4)) {
      clear();
      displayable = false;
   }
   else {
      if (level < 0.25)
         intLevel = 0;
      else if (level < 0.50)
         intLevel = 1;
      else if (level < 0.75)
         intLevel = 2;
      else
         intLevel = 3;
      displayable = gauge(intLevel, label);
   }

   return displayable;
}

uint8_t SevenSegDisplays::getDigitsQty(){

   return _dspDigitsQty;
}

int32_t SevenSegDisplays::getDspValMax(){

   return _dspValMax;
}

int32_t SevenSegDisplays::getDspValMin(){

   return _dspValMin;
}

uint16_t SevenSegDisplays::getInstanceNbr(){

   return _dspInstNbr;
}

unsigned long SevenSegDisplays::getMaxBlinkRate(){
    
   return _maxBlinkRate;
}

unsigned long  SevenSegDisplays::getMinBlinkRate(){

   return _minBlinkRate;
}

bool SevenSegDisplays::isBlank(){
   uint8_t result{true};

   for (int i{0}; i < _dspDigitsQty; i++){
      if(*(_dspBuffPtr + i) != _space){
         result = false;
         break;
      }
   }

   return result;
}

bool SevenSegDisplays::isBlinking(){

   return _blinking;
}

bool SevenSegDisplays::isWaiting(){

   return _waiting;
}

bool SevenSegDisplays::noBlink(){
   bool result {false};
    BaseType_t tmrModResult {pdFAIL};
   //Stops the blinking, frees the _dspAuxPtr pointed memory, Stops the timer attached to the process, cleans flags

   if(_blinking){
      _blinking = false;
      if(_blinkTmrHndl){   //if the timer still exists and is running, stop and delete
         tmrModResult = xTimerStop(_blinkTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS)
            tmrModResult = xTimerDelete(_blinkTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS)
            _blinkTmrHndl = NULL;
      }
      _restoreDspBuff();
      delete [] _dspAuxBuffPtr;
      _blinkTimer = 0;
      _blinkShowOn = true;
      _dspBuffChng = true;    //Signal for the hardware refresh mechanism
      result = true;
   }

   return result;
}

bool SevenSegDisplays::noWait(){
   //Stops the waiting, frees the _dspAuxPtr pointed memory, Stops the timer attached to the process
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (_waiting){
      _waiting = false;
      if(_waitTmrHndl){   //if the timer still exists and is running, stop and delete
         tmrModResult = xTimerStop(_waitTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS)
            tmrModResult = xTimerDelete(_waitTmrHndl, portMAX_DELAY);
         if(tmrModResult == pdPASS)
            _waitTmrHndl = NULL;
      }
      _restoreDspBuff();
      delete [] _dspAuxBuffPtr;
      _waitTimer = 0;
      _dspBuffChng = true;    //Signal for the hardware refresh mechanism
      result = true;
   }

   return result;
}

void SevenSegDisplays::_popSsd(SevenSegDisplays** &ssdInstncObjLst, SevenSegDisplays* ssdToPop){
	int arrSize{0};
	int auxPtr{0};
	bool ssdFnd{false};
	SevenSegDisplays** tmpArrPtr{nullptr};

	while(*(ssdInstncObjLst + arrSize) != nullptr){
		if(*(ssdInstncObjLst + arrSize) == ssdToPop){
			ssdFnd = true;
		}
		++arrSize;
	}
	if(ssdFnd){
		if(arrSize > 1){
			tmpArrPtr = new SevenSegDisplays* [arrSize];
			arrSize = 0;
			while(*(ssdInstncObjLst + arrSize) != nullptr){
				if(*(ssdInstncObjLst + arrSize) == ssdToPop){
					++arrSize;
					if(*(ssdInstncObjLst + arrSize) == nullptr)
						break;
				}
				*(tmpArrPtr + auxPtr) = *(ssdInstncObjLst + arrSize);
				++arrSize;
				++auxPtr;
			}
			*(tmpArrPtr + auxPtr) = nullptr;
			delete [] ssdInstncObjLst;
			ssdInstncObjLst = tmpArrPtr;
		}
		else{
			delete [] ssdInstncObjLst;
			ssdInstncObjLst = nullptr;
		}
	}

   return;
}

void SevenSegDisplays::_pushSsd(SevenSegDisplays** &ssdInstncObjLst, SevenSegDisplays* ssdToPush){
	int arrSize{0};
	bool ssdFnd{false};
	SevenSegDisplays** tmpArrPtr{nullptr};

	if(ssdInstncObjLst == nullptr){	// There are no "Seven Segment displays instances list", so create it			
		ssdInstncObjLst = new SevenSegDisplays* [1];
		*ssdInstncObjLst = nullptr;
	}

	while(*(ssdInstncObjLst + arrSize) != nullptr){
		if(*(ssdInstncObjLst + arrSize) == ssdToPush){
			ssdFnd = true;
			break;
		}
		else{
			++arrSize;
		}
	}
	if(!ssdFnd){
		tmpArrPtr = new SevenSegDisplays* [arrSize + 2];
		for (int i{0}; i < arrSize; ++i){
			*(tmpArrPtr + i) = *(ssdInstncObjLst + i);
		}
		*(tmpArrPtr + (arrSize + 0)) = ssdToPush;
		*(tmpArrPtr + (arrSize + 1)) = nullptr;
		delete [] ssdInstncObjLst;
		ssdInstncObjLst = tmpArrPtr;
	}

   return;
}

bool SevenSegDisplays::print(String text){
   bool displayable{true};
   bool printOnBlink{_blinking};
   int position{-1};

   String tempText{""};
   uint8_t temp7SegData[_dspDigitsQty];
   uint8_t tempDpData[_dspDigitsQty];

   for (int i{0}; i < _dspDigitsQty; i++){
      temp7SegData[i] = _space;
      tempDpData[i] = _space;
   }
   // Finds out if there are '.' in the string to display, creates a mask to add them to the display
   // and takes them out of the string to process the chars/digits
   for(unsigned int i{0}; i < text.length(); ++i){
      if (text.charAt(i) != '.')
         tempText += text.charAt(i);
      else{
         if (i == 0 || text.charAt(i-1) == '.')
            tempText += " ";
         if(tempText.length() <= _dspDigitsQty)
            tempDpData[_dspDigitsQty - tempText.length()] = _dot;
      }
   }
   text = tempText;
   if (text.length() <= _dspDigitsQty){
      for (unsigned int i {0}; i < text.length(); ++i){
         position = _charSet.indexOf(text.charAt(i));
         if (position > -1) {
            // Character found for translation
            temp7SegData[(_dspDigitsQty - 1) - i] = _charLeds[position];
         }
         else {
            displayable = false;
            break;
         }
      }
   }
   else {
      displayable = false;
   }
   if (displayable) {
      if(_waiting)
         noWait();
      if(printOnBlink)
         noBlink();
      for (uint8_t i{0}; i < _dspDigitsQty; ++i)
         *(_dspBuffPtr + i) = temp7SegData[i] & tempDpData[i];
      if(printOnBlink)
         blink();
      _dspBuffChng = true;
   }

   return displayable;
}

bool SevenSegDisplays::print(const int32_t &value, bool rgtAlgn, bool zeroPad){
   bool displayable{true};
   String readOut{""};

   if ((value < _dspValMin) || (value > _dspValMax)) {
      clear();
      displayable = false;
   }
   else {
      readOut = String(abs(value));
      if (rgtAlgn) {
         if (zeroPad)
            readOut = _zeroPadding + readOut;
         else
            readOut = _spacePadding + readOut;

         if (value >= 0)
            readOut = readOut.substring(readOut.length() - _dspDigitsQty);
         else
            readOut = readOut.substring(readOut.length() - (_dspDigitsQty - 1));
      }
      if (value < 0)
         readOut = "-" + readOut;
      displayable = print(readOut);
   }

   return displayable;
}

bool SevenSegDisplays::print(const double &value, const unsigned int &decPlaces, bool rgtAlgn, bool zeroPad){
   bool displayable{true};
   String readOut{""};
   String pad{""};
   int start{0};

   if (decPlaces == 0)
      displayable = print(int(value), rgtAlgn, zeroPad);
   else if ((value < _dspValMin) || (value > _dspValMax) || (decPlaces > _dspDigitsQty)) {
      displayable = false;
      clear();
   }
   else if ((decPlaces + String(int(value)).length()) > (((value < 0) && (value > (-1))) ? (_dspDigitsQty - 1) : _dspDigitsQty)) {
      displayable = false;
      clear();
   }
   else {
      if (value < 0 && value > -1)
         readOut = "-";
      readOut += String(int(value)) + ".";
      start = String(value).indexOf('.') + 1;
      readOut += (String(value) + _zeroPadding).substring(start, start + decPlaces);
      if (rgtAlgn) {
         if (readOut.length() < _dspDigitsQty + 1) {
            if (value < 0)
               pad += "-";
            if (zeroPad)
               pad += _zeroPadding;
            else
               pad += _spacePadding;
            if (value < 0)
               readOut = pad.substring(0, (_dspDigitsQty+1) - (readOut.length()-1)) + readOut.substring(1);
            else
               readOut = pad.substring(0, (_dspDigitsQty+1) - (readOut.length())) + readOut;
            readOut = readOut.substring(0, _dspDigitsQty + 1);
         }
      }
      displayable = print(readOut);
   }

   return displayable;
}

void SevenSegDisplays::resetBlinkMask(){
   for (uint8_t i{0}; i < _dspDigitsQty; i++)
      *(_blinkMaskPtr + i) = true;

   return;
}

void SevenSegDisplays::_restoreDspBuff(){
   //  for (int i{0}; i < _dspDigitsQty; i++)
   //      (*(_dspBuffPtr + i)) = (*(_dspAuxBuffPtr + i));
   memcpy(_dspBuffPtr, _dspAuxBuffPtr, _dspDigitsQty);   // destPtr, srcPtr, size

   return;
}

void SevenSegDisplays::_saveDspBuff(){
   //  for (int i{0}; i < _dspDigitsQty; i++)
   //      (*(_dspAuxBuffPtr + i)) = (*(_dspBuffPtr + i));
      memcpy(_dspAuxBuffPtr, _dspBuffPtr, _dspDigitsQty);   // destPtr, srcPtr, size

   return;
}

void SevenSegDisplays::_setAttrbts(){
   if (_dspDigitsQty > 1){ // Calculate the minimum integer value displayable with this display's available digits
      _dspValMin = 1;
      for (uint8_t i{0}; i < (_dspDigitsQty - 1); i++)
         _dspValMin *= 10;
      _dspValMin = -(--_dspValMin); //_dspValMin = (-1)*(_dspValMin - 1);
   }
   else
      _dspValMin = 0;

   _dspValMax = 1; // Calculate the maximum integer value displayable with this display's available digits, create a Zero and a Space padding string for right alignment
   for (uint8_t i{0}; i < _dspDigitsQty; i++) {
      _dspValMax *= 10;
      _zeroPadding += "0";
      _spacePadding += " ";
      *(_blinkMaskPtr + i) = true;
   }
   --_dspValMax;

   if (!_dspUndrlHwPtr->getCommAnode()) {
      _waitChar = ~_waitChar;
      _space = ~_space;
      _dot = ~_dot;
      for (int i{0}; i < (int)_charSet.length(); i++)
         _charLeds[i] = ~_charLeds[i];
   }

   return;
}

void SevenSegDisplays::setBlinkMask(const bool* newBlnkMsk){
   for (int i{0}; i < _dspDigitsQty; i++)
      *(_blinkMaskPtr + i) = *(newBlnkMsk + i);

   return;
}

bool SevenSegDisplays::setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate){
   bool result {false};
   long unsigned tmpOffRate{newOffRate};
   BaseType_t tmrModResult {pdFAIL};

   if (tmpOffRate == 0)
      tmpOffRate = newOnRate;
   if ((newOnRate != _blinkOnRate) || (tmpOffRate != _blinkOffRate)) {
      if ((newOnRate >= _minBlinkRate) && (newOnRate <= _maxBlinkRate)) { //The new ON rate is in the valid range
         if ((tmpOffRate >= _minBlinkRate) && (tmpOffRate <= _maxBlinkRate)) {    //The new OFF rate is in the valid range or is equal to 0 to set a symmetric blink
            if(_blinkOnRate != newOnRate)
               _blinkOnRate = newOnRate;
            if(_blinkOffRate != tmpOffRate)
               _blinkOffRate = tmpOffRate;
            _blinkRatesGCD = _blinkTmrGCD(newOnRate, newOffRate);
            result =  true;

            if(_blinking){ // If it's active and running modify the timer taking care of the blinking               
               tmrModResult = xTimerChangePeriod(_blinkTmrHndl, 
                              pdMS_TO_TICKS(_blinkRatesGCD),
                              portMAX_DELAY
                              );
               if(tmrModResult == pdFAIL)
                  result = false;
            }
         }
      }
   }
   else{
      result = true; //There's no need to change the current values, but as those were valid, they are still valid
   }

   return result;  
}

bool SevenSegDisplays::setWaitChar (const char &newChar){
    bool result {false};
    int position {-1};

   if(_waitChar != newChar){
      position = _charSet.indexOf(newChar);
      if (position > -1) {
         _waitChar = _charLeds[position];
         result = true;
      }
   }
   else{
      result = true;
   }

   return result;
}

bool SevenSegDisplays::setWaitRate(const unsigned long &newWaitRate){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(_waitRate != newWaitRate){
      if ((newWaitRate >= _minBlinkRate) && newWaitRate <= _maxBlinkRate) {//if the new waitRate is within the accepted range, set it
         _waitRate = newWaitRate;
         result =  true;

         if(_waiting){  // If it's active and running modify the timer taking care of the blinking            
            tmrModResult = xTimerChangePeriod(_waitTmrHndl, 
                           pdMS_TO_TICKS(_waitRate),
                           portMAX_DELAY
                           );
            if(tmrModResult == pdFAIL)
               result =  false;
         }
      }
   }
   return result;
}

void SevenSegDisplays::tmrCbBlink(TimerHandle_t blinkTmrCbArg){
   SevenSegDisplays* thisDisplay = (SevenSegDisplays*)blinkTmrCbArg;
   thisDisplay->_updBlinkState();

   return;
}

void SevenSegDisplays::tmrCbWait(TimerHandle_t waitTmrCbArg){
   SevenSegDisplays* thisDisplay = (SevenSegDisplays*)waitTmrCbArg;
   thisDisplay-> _updWaitState();

   return;
}

void SevenSegDisplays::_updBlinkState(){
   //The use of a xTimer that keeps flip-floping the _blinkShowOn value is better suited for symmetrical blinking, but not for asymmetrical cases.
   if (_blinking == true){
      if (_blinkShowOn == false) {
         if (_blinkTimer == 0){
            //The turn-Off display stage of the blinking started, copy the dspBuff contents to the dspAuxBuff before blanking the appropriate ports
            _saveDspBuff();
            //turn off the digits by placing a space to each corresponding position of the buffer
            for (int i{0}; i < _dspDigitsQty; i++)
               if(*(_blinkMaskPtr + i))
                  *(_dspBuffPtr + i) = _space;
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS; //Starts the count for the blinkRate control            
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer)> _blinkOffRate){
            _blinkTimer = 0;
            _blinkShowOn = true;
         }
      }
      else{
         if (_blinkTimer == 0){
            //The turn-On display stage of the blinking started, restore the dspBuff contents from the dspAuxBuff
            _restoreDspBuff();
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS;
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer) > _blinkOnRate){
            _blinkTimer = 0;
            _blinkShowOn = false;
         }
      }
   }

   return;
}

void SevenSegDisplays::_updWaitState(){
   if (_waiting == true){
      if (_waitTimer == 0){
         clear();
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
      else if((xTaskGetTickCount()/portTICK_RATE_MS - _waitTimer) > _waitRate){
         for (int i{_dspDigitsQty - 1}; i >= 0; i--){

            if(( _dspDigitsQty - i) <= _waitCount)
               *(_dspBuffPtr + i) = _waitChar;
            else
               *(_dspBuffPtr + i) = _space;
         }
         _dspBuffChng = true;
         _waitCount++;
         if (_waitCount == (_dspDigitsQty + 1))
            _waitCount = 0;
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
   }

   return;
}

bool SevenSegDisplays::wait(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(!_waiting){   //If the display is waiting the blinking option is blocked out as they are mutually excluyent, as both simultaneous has no logical use!
      //Create a valid unique Name for identifying the timer created
      char waitTmrName[15];
      char dspSerialNumChar[3]{};
      sprintf(dspSerialNumChar, "%0.2d", (int)_dspLastSerialNum);
      strcpy(waitTmrName, "Disp");
      strcat(waitTmrName, dspSerialNumChar);
      strcat(waitTmrName, "wait_tmr");

      if (!_waitTmrHndl){
            _waitTmrHndl = xTimerCreate(
               waitTmrName,
               pdMS_TO_TICKS(_waitRate),
               pdTRUE,  //Autoreload
               _dspInstance,   //TimerID, data to be passed to the callback function
               tmrCbWait  //Callback function
            );
      }
      if(_waitTmrHndl && (!xTimerIsTimerActive(_waitTmrHndl))){
            // The timer was created, but it wasn't started. Start the timer
            tmrModResult = xTimerStart(_waitTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
               result = true;
      }

      if (_blinking)
         noBlink();
      _dspAuxBuffPtr = new uint8_t[_dspDigitsQty];
      _saveDspBuff();
      _waitCount = 0;
      _waitTimer = 0;  //Start the blinking pace timer...
      _waiting = true;
      result = true;
    }

   return result;
}

bool SevenSegDisplays::wait(const unsigned long &newWaitRate){
   bool result {true};
   
   if (!_waiting){
      if(_waitRate != newWaitRate){
         if ((newWaitRate >= _minBlinkRate) && (newWaitRate <= _maxBlinkRate))
            _waitRate = newWaitRate;         
         else
            result = false;
      }
      if (result == true)
         wait();
   }
   else{
      result = false;
   }

   return result;
}

bool SevenSegDisplays::write(const uint8_t &segments, const uint8_t &port){
   bool result {false};
   bool writeOnBlink{_blinking};
    
   if (port < _dspDigitsQty){
      if(writeOnBlink)
         noBlink();
      *(_dspBuffPtr + port) = segments;
      if(writeOnBlink)
         blink();
      result = true;
   }
    
    return result;
}

bool SevenSegDisplays::write(const String &character, const uint8_t &port){
   bool result {false};
   int position {-1};
   bool writeOnBlink{_blinking};
    
   if (port < _dspDigitsQty){
      position = _charSet.indexOf(character);
      if (position > -1) { // Character found for translation                
         if(writeOnBlink)
            noBlink();
            *(_dspBuffPtr + port) = _charLeds[position];
         if(writeOnBlink)
            blink();
         result = true;
      }
    }

   return result;
}
