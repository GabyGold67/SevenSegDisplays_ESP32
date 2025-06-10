/**
 ******************************************************************************
 * @file SevenSegDisplays.cpp
 * 
 * @brief Code file for the SevenSegDisplays_ESP32 library 
 * 
 * @details The library provides a common API and tools to generate and manage contents formatting for seven segments displays.
 * 
 * 
 * 
 * 
 * 
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
 *       Last update:   27/04/2025 17:10 (GMT+0200) DST  
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
 */
#include <SevenSegDisplays.h>

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
   _SSDAuxBffMutex = xSemaphoreCreateMutex();
   _SSDBffrMutex = xSemaphoreCreateMutex();
   _SSDBlnkMskMutex = xSemaphoreCreateMutex();
   _SSDBlnkSttngMutex = xSemaphoreCreateMutex();
   _SSDObjLstMutex = xSemaphoreCreateMutex();
   _SSDWaitSttngMutex = xSemaphoreCreateMutex();

   _dspDigitsQty = _dspUndrlHwPtr->getHwDspDigitsQty(); // With the display size in digits, the needed arrays for data can be built
   _dspBuffPtr  = new uint8_t[_dspDigitsQty];
   _blinkMaskPtr = new bool[_dspDigitsQty];
   _dspUndrlHwPtr->setDspBuffPtr(_dspBuffPtr); // Indicates the hardware where the data to display is located
   _dspSerialNbr = _dspLastSerialNum++; // This value is always incremented, as it's not related to the active objects but to amount of different displays created
   ++_displaysCount;  // This keeps the count of instantiated SevenSegDisplays objects
   _dspInstance = this; // A pointer to the instantiated object
   _pushSsd(_ssdInstancesLstPtr, _dspInstance);
   _setAttrbts();
   clear(); 
}

SevenSegDisplays::~SevenSegDisplays(){
   BaseType_t tmrModResult;

   if(_isBlinking)
      noBlink();  // Stops the blinking, frees the _dspAuxBuffPtr pointed memory, Stops the timer attached to the process
   if(_blinkTmrHndl){   
      if(xTimerIsTimerActive(_blinkTmrHndl)) //if the timer still exists and is running, stop and delete
         tmrModResult = xTimerStop(_blinkTmrHndl, portMAX_DELAY);
      // if(tmrModResult == pdPASS)
      tmrModResult = xTimerDelete(_blinkTmrHndl, portMAX_DELAY);
      // if(tmrModResult == pdPASS)
      _blinkTmrHndl = NULL;         
   }

   if(_isWaiting)
      noWait();   // Stops the waiting, frees the _dspAuxBuffPtr pointed memory, Stops the timer attached to the process    
   if(_waitTmrHndl){   //if the timer still exists and is running, stop and delete
      if(xTimerIsTimerActive(_waitTmrHndl)) //if the timer still exists and is running, stop and delete
         tmrModResult = xTimerStop(_waitTmrHndl, portMAX_DELAY);
      // if(tmrModResult == pdPASS)
      tmrModResult = xTimerDelete(_waitTmrHndl, portMAX_DELAY);
      // if(tmrModResult == pdPASS)
      _waitTmrHndl = NULL;
   }

   if(_dspAuxBuffPtr != nullptr){
      delete [] _dspAuxBuffPtr;
     _dspAuxBuffPtr = nullptr;
   }
   delete [] _blinkMaskPtr;    // Free the resources of the blink mask buffer
   delete [] _dspBuffPtr;  // Free the resources of the display digits buffer
   _popSsd(_ssdInstancesLstPtr, _dspInstance);
   --_displaysCount;
   vSemaphoreDelete(_SSDAuxBffMutex); //FFDR destruct all the mutexes created at the constructor
   vSemaphoreDelete(_SSDBffrMutex); //FFDR destruct all the mutexes created at the constructor
   vSemaphoreDelete(_SSDBlnkSttngMutex); //FFDR destruct all the mutexes created at the constructor
   vSemaphoreDelete(_SSDObjLstMutex); //FFDR destruct all the mutexes created at the constructor
   vSemaphoreDelete(_SSDWaitSttngMutex); //FFDR destruct all the mutexes created at the constructor
}

bool SevenSegDisplays::begin(uint32_t updtLps){
   bool result{false};

   Serial.println("Entered SevenSegDisplays::begin()");
   if(!_begun){
      Serial.println("Underlying Hardware needs Begining");
      
      result = _dspUndrlHwPtr->begin(updtLps);
      
      Serial.print("Underlying Hardware Passed Begining, result = ");
      Serial.println(result);

      if(result){
         Serial.println("_begun attribute flag SET");
         _begun = true;   
      }       
   }
   else
      result = true;

   return result;
}

bool SevenSegDisplays::_blink(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(_isWaiting)   // If the display is waiting stop, as the waiting and the blinking options are mutually exclusive, as both simultaneous states has no logical use!
      noWait();   //FFDR develop and modify to _noWait()
   if (!_isBlinking){

      if (!_blinkTmrHndl){
         String blnkTmrName{""}; // Create a valid unique Name for identifying the timer created
         String dspSerialNumStr {"000" + String(_dspSerialNbr)};
         dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
         blnkTmrName = "Disp" + dspSerialNumStr + "blnk_tmr";
         
         _blinkTmrHndl = xTimerCreate(
            blnkTmrName.c_str(),  // Timer name
            pdMS_TO_TICKS(_blinkRatesGCD),
            pdTRUE,  // Autoreload
            _dspInstance,   // TimerID, data to be passed to the callback function: a pointer to this display  
            tmrCbBlink  // Callback function
         );
      }
      if(_blinkTmrHndl){
         if(!xTimerIsTimerActive(_blinkTmrHndl)){ // The timer was created, but it wasn't running. Start the timer  
            tmrModResult = xTimerStart(_blinkTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS){
               result = true;
            }
         }
         else{
            result = true;
         }
      }

      if(result){
         if(_dspAuxBuffPtr == nullptr){
            _dspAuxBuffPtr = new uint8_t[_dspDigitsQty];
         }
         if(_dspAuxBuffPtr){
            _blinkShowOn = false;
            _isBlinking = true;
            _blinkTimer = 0;  //Start the blinking pace timer...      
         }
         else{
            result = false;
         }
      }
   }

   return result;
}

bool SevenSegDisplays::blink(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(_isWaiting)   // If the display is waiting stop, as the waiting and the blinking options are mutually exclusive, as both simultaneous states has no logical use!
      noWait();
   if (!_isBlinking){
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBlnkSttngMutex, portMAX_DELAY) == pdTRUE){

            if (!_blinkTmrHndl){
               String blnkTmrName{""}; // Create a valid unique Name for identifying the timer created
               String dspSerialNumStr {"000" + String(_dspSerialNbr)};
               dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
               blnkTmrName = "Disp" + dspSerialNumStr + "blnk_tmr";
               
               _blinkTmrHndl = xTimerCreate(
                  blnkTmrName.c_str(),  // Timer name
                  pdMS_TO_TICKS(_blinkRatesGCD),
                  pdTRUE,  // Autoreload
                  _dspInstance,   // TimerID, data to be passed to the callback function: a pointer to this display  
                  tmrCbBlink  // Callback function
               );
            }
            if(_blinkTmrHndl){
               if(!xTimerIsTimerActive(_blinkTmrHndl)){ // The timer was created, but it wasn't running. Start the timer  
                  tmrModResult = xTimerStart(_blinkTmrHndl, portMAX_DELAY);
                  if (tmrModResult == pdPASS){
                     result = true;
                  }
               }
               else{
                  result = true;
               }
            }

            if(result){
               if(_dspAuxBuffPtr == nullptr){
                  _dspAuxBuffPtr = new uint8_t[_dspDigitsQty];
               }
               if(_dspAuxBuffPtr){
                  _blinkShowOn = false;
                  _isBlinking = true;
                  _blinkTimer = 0;  //Start the blinking pace timer...      
               }
               else{
                  result = false;
               }
            }
            xSemaphoreGive(_SSDAuxBffMutex);
            xSemaphoreGive(_SSDBlnkSttngMutex);
         }
      }
   }

   return result;
}

bool SevenSegDisplays::blink(const uint32_t &onRate, const uint32_t &offRate){
   bool result {false};

   if (!_isBlinking){
      if (offRate == 0)
         result = setBlinkRate(onRate, onRate);
      else
         result = setBlinkRate(onRate, offRate);        
      if (result)
         result = blink();
   }
   else
      result = true;

   return result;
}

uint32_t SevenSegDisplays::_blinkTmrGCD(uint32_t blnkOnTm, uint32_t blnkOffTm){
   /*returning values:
      0: One of the input values was 0
      1: No GDC greater than 1
      Other: This value would make the blink timer save resources by checking the blink time as less frequent as possible*/
      uint32_t result{ 0 };

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
   bool mainBuffChng{false};

   if(!_isWaiting){ // If in waiting condition clear() is not executed
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_isBlinking){         
               memset(_dspAuxBuffPtr,_space, _dspDigitsQty);   // Fast single command memory filling with byte
            }
            xSemaphoreGive(_SSDAuxBffMutex);
            for (int i{0}; i < _dspDigitsQty; i++){   // This mechanism provided to avoid signaling buffer change if it was already blank. blank() method not used to avoid the call time and resources
               if(*(_dspBuffPtr + i) != _space){
                  memset(_dspBuffPtr,_space, _dspDigitsQty);
                  mainBuffChng = true;
                  break;
               }
            }
         
            if(mainBuffChng)
               _setDspBuffChng(); // Signal for the hardware display update mechanism
            xSemaphoreGive(_SSDBffrMutex);
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
                lvlChar = " ";
                break;
            case 1:
                lvlChar = "_";
                break;
            case 2:
                lvlChar = "=";
                break;
            case 3:
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
               lvlChar = " ";
               break;
            case 1:
               lvlChar = "_";
               break;
            case 2:
               lvlChar = "=";
               break;
            case 3:
               lvlChar = "~";
               break;
        };
        readOut += lvlChar;
        displayable = print(readOut);
    }

    return displayable;
}

bool SevenSegDisplays::end(){
   bool result{false};

   if(_begun){
      result = _dspUndrlHwPtr->end();
      if (result)
         _begun = false;
   }
   else
      result = true;

   return result;
}

bool SevenSegDisplays::gauge(const int &level, char label){
   bool displayable{true};
   String readOut{_spacePadding};

   clear();
   if (((level < 0) || (level > 3)) || (_dspDigitsQty < 4)) {
      displayable = false;
   }
   else {
      switch (level) {
         case 3:
         readOut = "~" + readOut;
         case 2:
         readOut = "=" + readOut;
         case 1:
         readOut = "_" + readOut;
      };
      readOut = (label == ' ')?" ":label + readOut;
      readOut = readOut.substring(0, _dspDigitsQty);
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

uint8_t SevenSegDisplays::getCurBrghtnssLvl(){

   return _dspUndrlHwPtr->getBrghtnssLvl();
}

uint8_t SevenSegDisplays::getDigitsQty(){

   return _dspDigitsQty;
}

uint8_t SevenSegDisplays::getDspCount(){

   return _displaysCount;
}

bool SevenSegDisplays::getDspIsDmmbl(){

   return !(getMinBrghtnssLvl() == getMaxBrghtnssLvl());
}

bool SevenSegDisplays::getIsOn(){

   return _dspUndrlHwPtr->getIsOn();
}

SevenSegDispHw* SevenSegDisplays::getDspUndrlHwPtr(){
   
   return _dspUndrlHwPtr;
}

int32_t SevenSegDisplays::getDspValMax(){

   return _dspValMax;
}

int32_t SevenSegDisplays::getDspValMin(){

   return _dspValMin;
}

uint8_t SevenSegDisplays::getMaxBrghtnssLvl(){

   return _dspUndrlHwPtr->getBrghtnssMaxLvl();
}

uint8_t SevenSegDisplays::getMinBrghtnssLvl(){

   return _dspUndrlHwPtr->getBrghtnssMinLvl();
}

uint16_t SevenSegDisplays::getSerialNbr(){

   return _dspSerialNbr;
}

uint32_t SevenSegDisplays::getMaxBlinkRate(){
    
   return _maxBlinkRate;
}

uint32_t  SevenSegDisplays::getMinBlinkRate(){

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

   return _isBlinking;
}

bool SevenSegDisplays::isWaiting(){

   return _isWaiting;
}

bool SevenSegDisplays::_noBlink(){
   BaseType_t tmrModResult {pdFAIL};
   bool result {false};

   if(_isBlinking){
      _isBlinking = false;

      // tmrModResult = xTimerStop(_blinkTmrHndl, portMAX_DELAY); //FFDR The method fails when stopping the timer as it retrieves the buffer not correctly modified for a write while blinkin. Check for the auxiliary buffer being modified if a write is executed while blinking!!!
      // if (tmrModResult == pdPASS){
      //    result = true;
      // }

      _restoreDspBuff();   // This method calls _setDspBuffChng() if it suits
      _blinkTimer = 0;
      _blinkShowOn = true;
      result = true;
   }

   return result;
}

bool SevenSegDisplays::noBlink(){
   BaseType_t tmrModResult {pdFAIL};
   bool result {false};

   if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
      if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBlnkSttngMutex, portMAX_DELAY) == pdTRUE){
            if(_isBlinking){
               _isBlinking = false;

               // tmrModResult = xTimerStop(_blinkTmrHndl, portMAX_DELAY); //FFDR The method fails when stopping the timer as it retrieves the buffer not correctly modified for a write while blinkin. Check for the auxiliary buffer being modified if a write is executed while blinking!!!
               // if (tmrModResult == pdPASS){
               //    result = true;
               // }

               _restoreDspBuff();   // This method calls _setDspBuffChng() if it suits
               xSemaphoreGive(_SSDAuxBffMutex);
               xSemaphoreGive(_SSDBffrMutex);
               _blinkTimer = 0;
               _blinkShowOn = true;
               result = true;
            }
            xSemaphoreGive(_SSDBlnkSttngMutex);
         }
      }
   }   

   return result;
}

bool SevenSegDisplays::_noWait(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (_isWaiting){
      if(_waitTmrHndl){   //if the timer still exists and is running, stop and delete
         tmrModResult = xTimerStop(_waitTmrHndl, portMAX_DELAY);
      }
      if(tmrModResult == pdPASS){
         _restoreDspBuff();   // This method calls _setDspBuffChng() if it suits
         _waitTimer = 0;
         _isWaiting = false;
         result = true;
      }
   }
   else
      result = true;

   return result;
}

bool SevenSegDisplays::noWait(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (_isWaiting){
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
            if(xSemaphoreTake(_SSDWaitSttngMutex, portMAX_DELAY) == pdTRUE){
               if(_waitTmrHndl){   //if the timer still exists and is running, stop and delete
                  tmrModResult = xTimerStop(_waitTmrHndl, portMAX_DELAY);
               }
               if(tmrModResult == pdPASS){
                  _restoreDspBuff();   // This method calls _setDspBuffChng() if it suits
                  _waitTimer = 0;
                  _isWaiting = false;
                  result = true;
               }
               xSemaphoreGive(_SSDAuxBffMutex);
               xSemaphoreGive(_SSDBffrMutex);
               xSemaphoreGive(_SSDWaitSttngMutex);
            }
         }
      }
   }
   else
      result = true;

   return result;
}

void SevenSegDisplays::_ntfyToHwBuffChng(){
   _dspUndrlHwPtr->ntfyUpdDsply();

   return;
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
      if(xSemaphoreTake(_SSDObjLstMutex, portMAX_DELAY) == pdTRUE){
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
         xSemaphoreGive(_SSDObjLstMutex);
      }
	}

   return;
}

void SevenSegDisplays::_pushSsd(SevenSegDisplays** &ssdInstncObjLst, SevenSegDisplays* ssdToPush){
	int arrSize{0};
	bool ssdFnd{false};
	SevenSegDisplays** tmpArrPtr{nullptr};

   if(xSemaphoreTake(_SSDObjLstMutex, portMAX_DELAY) == pdTRUE){
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
      xSemaphoreGive(_SSDObjLstMutex);
   }

   return;
}

bool SevenSegDisplays::print(String text){
   bool displayable{true};
   bool dspCntnChng{false};
   // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool printOnBlink{_isBlinking};
   int position{-1};
   uint8_t temp7SegData[_dspDigitsQty];
   uint8_t tempDpData[_dspDigitsQty];
   String tempText{""};

   memset(temp7SegData,_space, _dspDigitsQty);
   memset(tempDpData,_space, _dspDigitsQty);

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
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
            if(xSemaphoreTake(_SSDBlnkSttngMutex, portMAX_DELAY) == pdTRUE){
               if(xSemaphoreTake(_SSDWaitSttngMutex, portMAX_DELAY) == pdTRUE){

                  if(_isWaiting)
                     _noWait();
                  if(printOnBlink)
                     _noBlink();
                  for (uint8_t i{0}; i < _dspDigitsQty; ++i){
                     if(_dspUndrlHwCommAnode){
                        if(*(_dspBuffPtr + i) != temp7SegData[i] & tempDpData[i]){
                           *(_dspBuffPtr + i) = temp7SegData[i] & tempDpData[i];
                           dspCntnChng = true;   
                        }             
                     }
                     else{
                        if(*(_dspBuffPtr + i) != temp7SegData[i] | tempDpData[i]){
                           *(_dspBuffPtr + i) = temp7SegData[i] | tempDpData[i];
                           dspCntnChng = true;   
                        }             
                     }
                  }
                  if(printOnBlink)
                     _blink();
                  if(dspCntnChng)
                     _setDspBuffChng();

                  xSemaphoreGive(_SSDWaitSttngMutex);
               }
               xSemaphoreGive(_SSDBlnkSttngMutex);
            }
            xSemaphoreGive(_SSDBffrMutex);
         }
         xSemaphoreGive(_SSDAuxBffMutex);
      }
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
               readOut = pad.substring(0, (_dspDigitsQty + 1) - (readOut.length() - 1)) + readOut.substring(1);
            else
               readOut = pad.substring(0, (_dspDigitsQty + 1) - (readOut.length())) + readOut;
            readOut = readOut.substring(0, _dspDigitsQty + 1);
         }
      }
      displayable = print(readOut);
   }

   return displayable;
}

void SevenSegDisplays::resetBlinkMask(){

   if(xSemaphoreTake(_SSDBlnkMskMutex,portMAX_DELAY) == pdTRUE){
      for (uint8_t i{0}; i < _dspDigitsQty; i++)
         *(_blinkMaskPtr + i) = true;
   xSemaphoreGive(_SSDBlnkMskMutex);
   }

   return;
}

void SevenSegDisplays::_restoreDspBuff(){ //!< Must ALWAYS be invoked from a _SSDAuxBffMutex and _SSDBffrMutex protected caller
   if(memcmp(_dspBuffPtr, _dspAuxBuffPtr, _dspDigitsQty) != 0){
      memcpy(_dspBuffPtr, _dspAuxBuffPtr, _dspDigitsQty);   // destPtr, srcPtr, size
      _setDspBuffChng();
   }

   return;
}

void SevenSegDisplays::_saveDspBuff(){ //!< Must ALWAYS be invoked from a _SSDAuxBffMutex and _SSDBffrMutex protected caller
   memcpy(_dspAuxBuffPtr, _dspBuffPtr, _dspDigitsQty);

   return;
}

void SevenSegDisplays::_setAttrbts(){
   if (_dspDigitsQty > 1){ // Calculate the minimum integer value displayable with the available digits
      _dspValMin = 1;
      for (uint8_t i{0}; i < (_dspDigitsQty - 1); i++)
         _dspValMin *= 10;
      _dspValMin = -(--_dspValMin); //_dspValMin = (-1)*(_dspValMin - 1);
   }
   else
      _dspValMin = 0;

   _dspValMax = 1; // Calculate the maximum integer value displayable with this display's available digits, create a Zero and a Space padding string for right alignment printing
   for (uint8_t i{0}; i < _dspDigitsQty; i++) {
      _dspValMax *= 10;
      _zeroPadding += "0";
      _spacePadding += " ";
      *(_blinkMaskPtr + i) = true;
   }
   --_dspValMax;

   _dspUndrlHwCommAnode = _dspUndrlHwPtr->getCommAnode();

   if (!_dspUndrlHwCommAnode) {
      _waitChar = ~_waitChar;
      _space = ~_space;
      _dot = ~_dot;
      for (int i{0}; i < (int)_charSet.length(); i++)
         _charLeds[i] = ~(_charLeds[i]);
   }

   return;
}

void SevenSegDisplays::setBlinkMask(const bool* newBlnkMsk){
   if(xSemaphoreTake(_SSDBlnkMskMutex, portMAX_DELAY) == pdTRUE){
      memcpy(_blinkMaskPtr, newBlnkMsk, _dspDigitsQty);   // destPtr, srcPtr, size
      xSemaphoreGive(_SSDBlnkMskMutex);
   }
   return;
}

bool SevenSegDisplays::setBlinkRate(const uint32_t &newOnRate, const uint32_t &newOffRate){
   bool result {false};
   long unsigned tmpOffRate{newOffRate};
   BaseType_t tmrModResult {pdFAIL};

   if (tmpOffRate == 0)
      tmpOffRate = newOnRate;
   if ((newOnRate != _blinkOnRate) || (tmpOffRate != _blinkOffRate)) {
      if(xSemaphoreTake(_SSDBlnkSttngMutex, portMAX_DELAY) == pdTRUE){
         if ((newOnRate >= _minBlinkRate) && (newOnRate <= _maxBlinkRate)) { //The new ON rate is in the valid range
            if ((tmpOffRate >= _minBlinkRate) && (tmpOffRate <= _maxBlinkRate)) {    //The new OFF rate is in the valid range or is equal to 0 to set a symmetric blink
               if(_blinkOnRate != newOnRate)
                  _blinkOnRate = newOnRate;
               if(_blinkOffRate != tmpOffRate)
                  _blinkOffRate = tmpOffRate;
               if(_blinkOnRate == _blinkOffRate)
                  _blinkRatesGCD = _blinkOnRate;
               else
                  _blinkRatesGCD = _blinkTmrGCD(_blinkOnRate, _blinkOffRate);
               result =  true;

               if(_isBlinking){ // If it's active and running modify the timer taking care of the blinking               
                  tmrModResult = xTimerChangePeriod(_blinkTmrHndl, 
                                 pdMS_TO_TICKS(_blinkRatesGCD),
                                 portMAX_DELAY
                                 );
                  if(tmrModResult == pdFAIL)
                     result = false;
               }
            }
         }
         xSemaphoreGive(_SSDBlnkSttngMutex);
      }
   }
   else{
      result = true; //There's no need to change the current values, but as those were valid, they are still valid
   }

   return result;  
}

bool SevenSegDisplays::setBrghtnssLvl(const uint8_t &newBrghtnssLvl){
   bool result{false};

   if(getDspIsDmmbl())
      result = _dspUndrlHwPtr->setBrghtnssLvl(newBrghtnssLvl);
   
   return result;
}

void SevenSegDisplays::_setDspBuffChng(){
   _ntfyToHwBuffChng();

   return;
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

bool SevenSegDisplays::setWaitRate(const uint32_t &newWaitRate){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if(_waitRate != newWaitRate){
      if ((newWaitRate >= _minBlinkRate) && newWaitRate <= _maxBlinkRate) {//if the new waitRate is within the accepted range, set it
         if(xSemaphoreTake(_SSDWaitSttngMutex, portMAX_DELAY) == pdTRUE){
            _waitRate = newWaitRate;
            result =  true;

            if(_isWaiting){  // If it's active and running modify the timer taking care of the blinking            
               tmrModResult = xTimerChangePeriod(_waitTmrHndl, 
                              pdMS_TO_TICKS(_waitRate),
                              portMAX_DELAY
                              );
               if(tmrModResult == pdFAIL)
                  result =  false;
            }
            xSemaphoreGive(_SSDWaitSttngMutex);
         }
      }
   }
   return result;
}

void SevenSegDisplays::tmrCbBlink(TimerHandle_t blinkTmrCbArg){
   SevenSegDisplays* thisDisplay = (SevenSegDisplays*)pvTimerGetTimerID(blinkTmrCbArg);
   thisDisplay->_updBlinkState();

   return;
}

void SevenSegDisplays::tmrCbWait(TimerHandle_t waitTmrCbArg){
   SevenSegDisplays* thisDisplay = (SevenSegDisplays*)pvTimerGetTimerID(waitTmrCbArg);
   thisDisplay-> _updWaitState();

   return;
}

void SevenSegDisplays::turnOff(){
   _dspUndrlHwPtr->turnOff();

   return;
}

void SevenSegDisplays::turnOn(){
   _dspUndrlHwPtr->turnOn();

   return;
}

void SevenSegDisplays::turnOn(const uint8_t &newBrghtnssLvl){
   _dspUndrlHwPtr->turnOn(newBrghtnssLvl);

   return;
}

void SevenSegDisplays::_updBlinkState(){
   bool mainBuffChng{false};
   BaseType_t tmrModResult{};

   //The use of a xTimer that keeps flip-flopping the _blinkShowOn value is better suited for symmetrical blinking, but not for asymmetrical cases.
   if (_isBlinking == true){
      if (_blinkShowOn == false) {
         if (_blinkTimer == 0){  // The turn-Off display stage of the blinking started, copy the dspBuff contents to the dspAuxBuff before blanking the appropriate ports            
            _saveDspBuff();            
            for (int i{0}; i < _dspDigitsQty; i++){   // Turn off the digits by placing a space to each corresponding position of the buffer
               if(*(_blinkMaskPtr + i)){
                  *(_dspBuffPtr + i) = _space;
                  if(!mainBuffChng){
                     mainBuffChng = true;
                  }
               }
            }
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS; //Starts the count for the blinkRate control            
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer)> _blinkOffRate){
            _blinkTimer = 0;
            _blinkShowOn = true;
         }
      }
      else{
         if (_blinkTimer == 0){  //The turn-On display stage of the blinking started, restore the dspBuff contents from the dspAuxBuff            
            _restoreDspBuff();
            mainBuffChng = true;
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS;
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer) > _blinkOnRate){
            _blinkTimer = 0;
            _blinkShowOn = false;
         }
      }
   }
   else{ 
      tmrModResult = xTimerStop(_blinkTmrHndl, portMAX_DELAY);
   }
   if(mainBuffChng){
      _setDspBuffChng();   //Signal for the hardware refresh mechanism
   }

   return;
}

void SevenSegDisplays::_updWaitState(){
   if (_isWaiting == true){
      if (_waitTimer == 0){
         _saveDspBuff();            
         clear();
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
      else if((xTaskGetTickCount()/portTICK_RATE_MS - _waitTimer) > _waitRate){
         memset(_dspBuffPtr,_space, _dspDigitsQty - _waitCount);
         memset((_dspBuffPtr + _dspDigitsQty - _waitCount), _waitChar, _waitCount);
         _setDspBuffChng(); // Notify underlying display the change of buffer data
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

   if (_isBlinking)
      noBlink();
   if(!_isWaiting){   //If the display is waiting the blinking option is blocked out as they are mutually exclusive, as both simultaneous has no logical use!
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
            if(xSemaphoreTake(_SSDWaitSttngMutex, portMAX_DELAY) == pdTRUE){
               if (!_waitTmrHndl){         
                  String waitTmrName{""}; //Create a valid unique Name for identifying the Wait timer created
                  String dspSerialNumStr {"000" + String(_dspSerialNbr)};
                  dspSerialNumStr = dspSerialNumStr.substring(dspSerialNumStr.length() - 3, dspSerialNumStr.length());
                  waitTmrName = "Disp" + dspSerialNumStr + "wait_tmr";

                  _waitTmrHndl = xTimerCreate(
                     waitTmrName.c_str(),  // Timer name
                     pdMS_TO_TICKS(_waitRate),
                     pdTRUE,  // Autoreload
                     _dspInstance,   // TimerID, data to be passed to the callback function
                     tmrCbWait  // Callback function
                  );
               }
               if(_waitTmrHndl){   // The timer was created,check it wasn't started. Start the timer            
                  if((!xTimerIsTimerActive(_waitTmrHndl))){  // The timer was created, but it wasn't running. Start the timer  
                     tmrModResult = xTimerStart(_waitTmrHndl, portMAX_DELAY);
                     if (tmrModResult == pdPASS){
                        result = true;
                     }
                  }
                  else{
                     result = true;
                  }
               }
               if(result){
                  if(_dspAuxBuffPtr == nullptr){
                     _dspAuxBuffPtr = new uint8_t[_dspDigitsQty];
                  }
                  if(_dspAuxBuffPtr){
                     _saveDspBuff();
                     _waitCount = 0;
                     _waitTimer = 0;  //Start the waiting pace timer...
                     _isWaiting = true;
                  }
                  else{
                     result = false;
                  }
               }
               xSemaphoreGive(_SSDWaitSttngMutex);
            }
            xSemaphoreGive(_SSDBffrMutex);
         }
         xSemaphoreGive(_SSDAuxBffMutex);
      }
      else{
         result = true;
      }
   }
   
   return result;
}

bool SevenSegDisplays::wait(const uint32_t &newWaitRate){
   bool result {true};
   
   if (!_isWaiting){
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
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};
   bool writeOnBlink{false};
    
   if (port < _dspDigitsQty){
      if(xSemaphoreTake(_SSDAuxBffMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SSDBffrMutex, portMAX_DELAY) == pdTRUE){
            if(xSemaphoreTake(_SSDBlnkSttngMutex, portMAX_DELAY) == pdTRUE){
               writeOnBlink = _isBlinking;
               if(writeOnBlink)
                  _noBlink();               
               if(*(_dspBuffPtr + port) != segments){
                  *(_dspBuffPtr + port) = segments;
                  _setDspBuffChng(); // Notify underlying display the change of buffer data
               }
               if(writeOnBlink)
                  _blink();               
               xSemaphoreGive(_SSDAuxBffMutex);
               xSemaphoreGive(_SSDBffrMutex);
               xSemaphoreGive(_SSDBlnkSttngMutex);               
            }
         }
      }
      result = true;
   }   
       
   return result;
}

bool SevenSegDisplays::write(const String &character, const uint8_t &port){
   int position {-1};
   bool result {false};
    
   if (port < _dspDigitsQty){
      position = _charSet.indexOf(character);
      if (position > -1)   // Character found for translation
         result = write(_charLeds[position], port);
   }

   return result;
}
