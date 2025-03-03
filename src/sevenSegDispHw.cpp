#include "Arduino.h"
#include "sevenSegDispHw.h"

/*Prototype for a SevenSegDispHw classes and SUBClasses timer callback function
static void  tmrStaticCbBlink(TimerHandle_t blinkTmrCbArg){
    SevenSegDispHw* SevenSegUndrlHw = (SevenSegDispHw*) blinkTmrCbArg;

    //***************

    return;
}*/

const int MAX_DIGITS_PER_DISPLAY{8};
const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t noName4Bits[4] {0, 1, 2, 3};

//---------------------------------------------------------------
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
//============================================================> Class methods separator

SevenSegDispHw::SevenSegDispHw() {}

SevenSegDispHw::SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:_ioPins{ioPins}, _digitPosPtr{new uint8_t[dspDigits]}, _dspDigitsQty {dspDigits}, _commAnode {commAnode}
{
    _dspHwInstNbr = _dspHwSerialNum++;
    for (uint8_t i{0}; i < _dspDigitsQty; i++){
        *(_digitPosPtr + i) = i;
    }
}

SevenSegDispHw::~SevenSegDispHw() {
    delete [] _digitPosPtr;
}

bool SevenSegDispHw::getCommAnode(){

    return _commAnode;
}

uint8_t* SevenSegDispHw::getDspBuffPtr(){
    
    return _dspBuffPtr;
}

uint8_t SevenSegDispHw::getDspDigits(){

    return _dspDigitsQty;
}

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
    bool result{true};

    for(int i {0}; i < _dspDigitsQty; i++){
        if (*(newOrderPtr + i) >= _dspDigitsQty){
            result = false;
            break;
        }   
    }
    if (result){
        for(int i {0}; i < _dspDigitsQty; i++){
            *(_digitPosPtr + i) = *(newOrderPtr + i);
        }
    }

    return result;
}

void SevenSegDispHw::setDspBuffPtr(uint8_t* newDspBuffPtr){
    _dspBuffPtr = newDspBuffPtr;

    return;
}

//============================================================> Class methods separator

SevenSegDynamic::SevenSegDynamic(){}

SevenSegDynamic::~SevenSegDynamic(){}

bool SevenSegDynamic::begin(){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
    if (!_svnSgDynTmrHndl){
        //Create a valid unique Name for identifying the timer created
        char rfrshTmrName[18];
        char dspSerialNumChar[3]{};
        sprintf(dspSerialNumChar, "%0.2d", (int)_dspHwInstNbr);
        strcpy(rfrshTmrName, "DynDsp");
        strcat(rfrshTmrName, dspSerialNumChar);
        strcat(rfrshTmrName, "rfrsh_tmr");
      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
        _dspRfrshTmrHndl = xTimerCreate(
            rfrshTmrName,
            pdMS_TO_TICKS((int)(1000/(30 * _dspDigitsQty))),
            pdTRUE,  //Autoreload
            NULL,   //TimerID, data to be passed to the callback function
            tmrCbRefreshDyn  //Callback function
        );
        if((_dspRfrshTmrHndl != nullptr) && (!xTimerIsTimerActive(_dspRfrshTmrHndl))){
            tmrModResult = xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

/*void SevenSegDisplays::fastRefresh(){
   bool tmpLogic {true};

   updBlinkState();
   updWaitState();
   if ((_blinking == false) || (_blinkShowOn == true)) {
    //   send(*(_dspBuffPtr + _firstRefreshed), uint8_t(1) << *(_digitPosPtr + _firstRefreshed));
   }
   else if(_blinking && !_blinkShowOn){
      for(uint8_t i{0}; i<_dspHwPtr->getDspDigits(); i++)
         tmpLogic = tmpLogic && *(_blinkMaskPtr + i);
      if (!tmpLogic){   //At least one digit is set NOT TO BLINK
         if(!*(_blinkMaskPtr + _firstRefreshed))
            // send(*(_dspBuffPtr + _firstRefreshed), uint8_t(1) << *(_digitPosPtr + _firstRefreshed));
      }
   }
   ++_firstRefreshed;
   if (_firstRefreshed == _dspHwPtr->getDspDigits())
      _firstRefreshed = 0;

   return;
}*/

void SevenSegDynamic::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    for (int i {0}; i < _dspDigitsQty; i++){
        tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
        // send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

    return;
}

// void SevenSegDynamic::send(uint8_t content){    // Implementation is hardware dependant (subclasses) protocol!!
// 
//     return;
// }

// void SevenSegDynamic::send(const uint8_t &segments, const uint8_t &port){
// 
//    return;
// }

bool SevenSegDynamic::stop() {
    bool result {false};

    if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
        xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
        xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
        _dspRfrshTmrHndl = nullptr;
    }

    return result;
}

void SevenSegDynamic::tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg){
/*   SevenSegDisplays **argObj = (SevenSegDisplays**)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method
    
    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        // if (*(_instancesLstPtr + i) != nullptr)
            // (*(_instancesLstPtr + i)) -> fastRefresh();
    }    
*/
    return;
}

//============================================================> Class methods separator

SevenSegDynHC595::SevenSegDynHC595(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
{    
    // _ioPins = ioPins;
    // SevenSegDispHw(ioPins, dspDigits, commAnode);
    pinMode(*(_ioPins + _sclk), OUTPUT);
    pinMode(*(_ioPins + _rclk), OUTPUT);
    pinMode(*(_ioPins + _dio), OUTPUT);

    begin();
}

SevenSegDynHC595::~SevenSegDynHC595(){}

bool SevenSegDynHC595::begin(){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    _firstRefreshed = 0;
    //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
    if (!_svnSgDynTmrHndl){
        //Create a valid unique Name for identifying the timer created
        char rfrshTmrName[18];
        char dspSerialNumChar[3]{};
        sprintf(dspSerialNumChar, "%0.2d", (int)_dspHwInstNbr);
        strcpy(rfrshTmrName, "DynDsp");
        strcat(rfrshTmrName, dspSerialNumChar);
        strcat(rfrshTmrName, "rfrsh_tmr");
        //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Qty of digits * 30Hz)
        _dspRfrshTmrHndl = xTimerCreate(
            rfrshTmrName,
            pdMS_TO_TICKS((int)(1000/(30 * _dspDigitsQty))),
            pdTRUE,  //Autoreload
            this,   //TimerID, data to be passed to the callback function
            tmrCbRefreshHC595  //Callback function
        );
        if((_dspRfrshTmrHndl != NULL) && (!xTimerIsTimerActive(_dspRfrshTmrHndl))){
            tmrModResult = xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);
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
        send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

    return;
}
void SevenSegDynHC595::send(uint8_t content){
    for (int i {7}; i >= 0; i--){   //Send each of the 8 bits representing the character
        if (content & 0x80)
            digitalWrite(*(_ioPins + _dio), HIGH);
        else
            digitalWrite(*(_ioPins + _dio), LOW);
        content <<= 1;
        digitalWrite(*(_ioPins + _sclk), LOW);
        digitalWrite(*(_ioPins + _sclk), HIGH);
    }

    return;

}

void SevenSegDynHC595::send(const uint8_t &segments, const uint8_t &port){

    digitalWrite(*(_ioPins + _rclk), LOW);
    send(segments);
    send(port);
    digitalWrite(*(_ioPins + _rclk), HIGH);

   return;
}

bool SevenSegDynHC595::stop() {
    bool result {false};

    if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
        xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
        xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
        _dspRfrshTmrHndl = NULL;
    }

    return result;
}

void SevenSegDynHC595::tmrCbRefreshHC595(TimerHandle_t rfrshTmrCbArg){
    SevenSegDynHC595* SevenSegUndrlHC595 = (SevenSegDynHC595*) rfrshTmrCbArg;
    //Timer Callback to keep the display lit by calling this display's refresh() method
    
    SevenSegUndrlHC595->refresh();

    return;
}

//============================================================> Class methods separator

SevenSegDynDummy::SevenSegDynDummy(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
{
    _ioPins = ioPins;
    _dspDigitsQty = dspDigits;
    _commAnode = commAnode;
    Serial.begin(9600);

}

SevenSegDynDummy::~SevenSegDynDummy()
{
    
}

//============================================================> Class methods separator

// SevenSegStatic::SevenSegStatic() {}

SevenSegStatic::~SevenSegStatic() {}

//============================================================> Class methods separator

// SevenSegTM1637::SevenSegTM1637() {}

SevenSegTM1637::~SevenSegTM1637() {}

//============================================================> Class methods separator

SevenSegStatHC595::SevenSegStatHC595() {}

SevenSegStatHC595::~SevenSegStatHC595() {}

//============================================================> Class methods separator

SevenSegStatDummy::SevenSegStatDummy(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
{
    _ioPins = ioPins;
    _dspDigitsQty = dspDigits;
    _commAnode = commAnode;
    Serial.begin(9600);
}

SevenSegStatDummy::~SevenSegStatDummy(){}
