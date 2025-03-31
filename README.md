# **SevenSegDisplays_ESP32: A Seven Segments displays easy to use library**  

**The ultimate goal: Any seven segment display. Every driving technology. A single library.**    

## [Complete SevenSegDisplays_ESP32 library documentation](https://gabygold67.github.io/SevenSegDisplays_ESP32/)

Originally developed to easily display numeric and text data in unattended manner on the cheap and popular "4-Bits LED Digital Tube Module", **a 7-segment 4 digits led display dynamically driven by two 74HC595 shift register**, the main focus was set on:  
- Ease of use.  
- Flexibility.
- Basic prevention of 'misrepresentation' errors.

As the library grew in services and benefits to manage and easily display data it became obvious that other models and technical driving solutions for seven segments displays could make use of the same services, but most important, it would make transparent to the developer the technology behind the display he's required to use, the API and amenities would be the same.  

So the original [4-Bit Led Digital Tube](https://github.com/GabyGold67/FourBitLedDigitalTube) library started an evolution process, including:  
- Extending the number of digits handled to 8 (the maximum known managing capabilities of a **two** 74HC595 dynamic driven display module).
- Extending the quantity of displays to be managed simultaneously.
- Adding a redirection level for the different hardware implementations, as different displays wire each individual digit to a different shift register pins, mixing the output display.  
- Adding other driving technologies:  
   - Static shift registers displays.  
   - Dedicated chip drivers displays.  

# But the main objectives remain the same:    
## Ease of use:  
- Instance the hardware class passing as little parameters as required: connection pins, display digits quantity, type of seven segment display connection (common anode, common cathode)  
- Instance the managing class using just the instantiated hardware class as parameter.  
- start ``.print()``ing the data to the display.  
There's no need of even setting the pin modes.  

## Flexibility:
Integers, floating point or strings they'll show as long as the display is capable of doing so in a trustworthy way. If you need to represent a percentage or level of completeness a ``.gauge()`` and a ``.doubleGauge()`` methods are included to represent them in a "Old Motorola brick cell phones' style". The library is capable of managing a correct representation even in differently wired displays by letting configure the order in which the digits are connected to the registers.  

## Trustworthy representation basic checking:  
The representation of different types of data in this kind of displays is limited, and many implementations of the libraries to drive them take arbitrary or personally biased decisions on how to handle the problem.  
The danger of misrepresenting values in the display are usually ignored so when a value can't be faithfully represented by the display, the data is rounded, floored, ceiled, sliced, characters are replaced by others or whatever criteria the developer defined. When trying to display the value **"90153"** through a 4 digits  module, displaying **"9015"** is no better, nor worse, than displaying **"0153"**, those are **misrepresentations**. This library returns a boolean value indicating if it was able to display a trustworthy representation of the value, as long as it is able to. If a trustworthy representation was not possible it will return a **false** value and blank the display.  

## Unattended display update and refreshing:
Some of the displays technologies need to be periodically refreshed, as they can actively turn on only one digit at a time, so to keep all de digits visible the user must activate periodically each digit one by one independently to generate a "cinematic effect". The library documentation will refer to them as **Dynamic Displays**, as is the case of the original **4-Bit Led Digital Tube** with two 74HC595 shift registers. 
Other technologies have the same shift registers set up in a different pattern so that they can hold the display active without refreshing constantly, assigning one shift register per digit. The library documentation will refer to them as **Static Dumb Displays**.    
Finally, being the seven segments displays so popular and usually used, specially for industrial and heavy duty uses, several chips have been developed to receive the data to exhibit and behavior commands through a standard communication protocols (I2C and SPI being the most popular choices), and keep the displays updated with the information received. The library documentation will refer to them as **Smart Displays**.    


The library takes care of this, and offers two solutions to do so.  
* The first is to attach the refreshing methods to a timer interrupt service (ISR) of the microcontroller, or to a Software Timer Service provided by the O.S., in this case the FreeRTOS timer daemon.  
* The second is through methods that the user can call periodically from the main code.  

The first mechanism frees the user from the load of calling the refreshing methods periodically, specially considering that long looping times (when executing **`for`**, **`while`** and **`do`** loops included), or the use of **`delay()`** type of commands could make the display flicker or simply stop until next refresh. The second option is given for the development of special display schemes. In any case the library is capable of working in any platform, using one way when possible, or the other always.  

# **Included Methods for SevenSegDisplays class**

|Method | Parameters|
|---|---|
|**_SevenSegDisplays_** |SevenSegDispHw* **dspUndrlHwPtr**|
|**blink()**|None|
||unsigned long **onRate** (,unsigned long **offRate**)|
|**clear()**|None|
|**doubleGauge()**|int **levelLeft**, int **levelRight** (, char **labeLeft**(, char **labelRight**))|
|**gauge()**|int **level** (, char **label**)|
||double **level** (, char **label**)|
|**getDigitsQty()**|None|
|**getDspCount()**|None|
|**getDspUndrlHwPtr()**|None|
|**getDspValMax()**|None|
|**getDspValMin()**|None|
|**getInstanceNbr()**|None|
|**getMaxBlinkRate()**|None|
|**getMinBlinkRate()**|None|
|**getSerialNbr()**|None|
|**isBlank()**|None|
|**isBlinking()**|None|
|**isWaiting()**|None|
|**noBlink()**|None|
|**noWait()**|None|
|**print()**|String **text**|
||int **value** (, bool **rgtAlgn** (, bool **zeroPad**))|
||double **value** (, unsigned int **decPlaces** (, bool **rgtAlgn** (, bool **zeroPad**)))|
|**resetBlinkMask()**|None|
|**setBlinkMask()**|bool **blnkPort[]**|
|**setBlinkRate()**|unsigned long **newOnRate**, (unsigned long **newOffRate**)|
|**setWaitChar()**|char **newWaitChar**|
|**setWaitRate()**|unsigned long **newWaitRate**|
|**wait()**|(unsigned long **waitRate**)|
|**write()**|uint8_t **segments**, uint8_t **port**|
||String **character**, uint8_t **port**|  

---  

# **Included Methods for SevenSegDispHw subclasses**

|Method | Parameters|
|---|---|
|**begin()**|None|
|**end()**|None|
|**getCommAnode()**|None|
|**getDspBuffPtr()**|None|
|**getHwDspDigitsQty()**|None|
|**ntfyUpdDsply()**|None|
|**setDigitsOrder()**|uint8_t* **newOrder**, uint8_t **newOrderSize**|
|**setDspBuffPtr()**|uint8_t* **newDspBuffPtr**|

---  

# **Included Methods for SevenSegTM163X subclasses**

|Method | Parameters|
|---|---|
|**getBrghtnssLvl()**|None|
|**getBrghtnssMaxLvl()**|None|
|**getBrghtnssMinLvl()**|None|
|**setBrghtnssLvl()**|uint8_t **newBrghtnssLvl**|


# **Included Methods for SevenSegDynHC595 class**


