# **SevenSegDisplays_ESP32: A Seven Segments displays easy to use library**  

# "One library to rule them all..."  
**(Or at least as many as I could find to test them)**  

The library goal is to provide a common consistent API to display data on **seven segments LED displays** independently of the hardware used: the display modules and the driving electronics.  

With a very long time in the market, **many** different techniques and technologies have been developed to display information in seven segment displays, but like any other technical resource, massification contributed to the development of specific standard hardware, both for the display and the display driving sides.  

This library is oriented to the management of the most usual combinations of those components, **even providing management for displays with one of the components missing** (see the "dummy" and "PinDrv" SevenSegHw subclasses).  

## [Complete SevenSegDisplays_ESP32 library documentation HERE!](https://gabygold67.github.io/SevenSegDisplays_ESP32/)

Originally developed to easily display numeric and text data in unattended manner on the cheap and popular "4-Bits LED Digital Tube Module", **a 7-segment 4 digits led display dynamically driven by two 74HC595 shift register**, the main focus was set on:  
- Ease of use.  
- Flexibility.
- Basic prevention of 'misrepresentation' errors.

As the library grew in services and amenities to manage and easily display data it became obvious that other models and technical driving solutions for seven segments displays could make use of the same benefits, but most important, it would make transparent to the developer the technology behind the display he's required to use, the API and amenities would be the same.  

So the original [4-Bit Led Digital Tube](https://github.com/GabyGold67/FourBitLedDigitalTube) library started an evolution process, including:  
- Extending the number of digits handled to 8 (the maximum known managing capabilities of a **two** 74HC595 dynamic driven display module).
- Extending the quantity of displays to be managed simultaneously.
- Adding a redirection level for the different hardware implementations, as different displays wire each individual digit to a different shift register pins, mixing the output display.  
- Adding other driving technologies:  
   - Static shift registers displays.  
   - Dedicated chip drivers displays.  
   - No driver displays (direct pin connection).

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
The danger of misrepresenting values in the display are usually ignored so when a value can't be faithfully represented by the display, the data is rounded, floored, ceiled, sliced, characters are replaced by others or whatever criteria the developer defined. When trying to display the value **"90153"** through a 4 digits  module, displaying **"9015"** is no better, nor worse, than displaying **"0153"**, those are **misrepresentations**. This library returns a boolean value indicating if it was able to display a trustworthy representation of the value, as long as it is able to. If a trustworthy representation is not possible it will return a **false** value and blank the display.  

## Unattended display update and refreshing:
Some of the displays technologies need to be periodically refreshed, as they can actively turn on only one digit at a time, so to keep all de digits visible the user must activate periodically each digit one by one independently to generate a "cinematic effect". The library documentation will refer to them as **Dynamic Displays**, as is the case of the original **4-Bit Led Digital Tube** with two 74HC595 shift registers or the case of the **direct pin connection** displays.  
Other technologies have the same shift registers set up in a different pattern so that they can hold the display active without constant refreshing, assigning one shift register per digit. The library documentation will refer to them as **Static Dumb Displays**.  
Finally, being the seven segments displays so popular and usually used, specially for industrial and heavy duty equipment, several chips have been developed to receive the data to exhibit and behavior commands through communication protocols (standard I2C and SPI being the most popular choices, but non-standard protocols are used too), and keep the displays updated with the information received. The library documentation will refer to them as **Smart Displays**.  

## The library is modeled in two layers:  
- The **Seven Segment Display Hardware** "lower" level, modeled in the **SevenSegDispHw** class and subclasses
- The **Seven Segment Display API** "upper" level, modeled in the **SevenSegDisplays** class.  

The relation between classes is a ***Composition***.  
The standard procedure to instantiate a working display in quite simple:
- Instantiate a SevenSegDispHw class object.  
- Invoke any required method to configure the instantiated object **ONLY IF NEEDED** before it might be used.  
- Instantiate a SevenSegDisplays object using a pointer to the SevenSegDispHw class object as unique parameter.  
- Start sending data to the display.  

If no modification of the hardware object is needed this becomes a single line command such as:  
`SevenSegDisplays myLedDisp(new SevenSegDynHC595 (myDispIOPins, 4, true));`  // Easy to understand reading from right to left, as usual!!  

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


