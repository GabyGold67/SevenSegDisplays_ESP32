# **SevenSegDisplays_ESP32: A Seven Segments displays easy to use library**  
## **Any display. Every driving technology. A single API**  

## [Complete SevenSegDisplays_ESP32 library documentation](https://gabygold67.github.io/SevenSegDisplays_ESP32/)

Originally developed to easily display numeric and text data in unatended manner on the cheap and popular "4-Bits LED Digital Tube Module", **a 7-segment 4 digits led display dynamically driven by two 74HC595 shift register**, the main focus was set on:  
- Ease of use.  
- Flexibility.
- Basic prevention of 'misrepresentation' errors.

As the library grew in services and benefits to manage and easily display data it became obvious that other models and technical driving solutions for seven segments displays could make use of the same services, but most important, it would make transparent to the developer the technology behind the display he's required to use, the API and ammenities will be the same.  

So the original [4-Bit Led Digital Tube](https://github.com/GabyGold67/FourBitLedDigitalTube) library started a evolution process, including:  
- Extending the number of digits handled to 8 (the maximum known managing capabilities of a **two** 74HC595 display module).
- Extending the quantity of displays to be managed simultaneously.
- Adding a redirection level for the different hardware implementations, as different displays wire each individual digit to a different shift register pins, mixing the output display.  

# But the main objetive remain the same:  
## Ease of use:  
- Instance the class passing as little parameters as required: connection pins, display digits quantity, type of seven segment display connection (common anode, common cathode)  
- Notify the object and it's ready to go.  
- start ``.print()``ing the data to the display.  
There's no need of even setting the pin modes.  

## Flexibility:
Integers, floating point or strings they'll show as long as the display is capable of doing so in a trustworthy way. If you need to represent a percentage or level of completeness a ``.gauge()`` and a ``.doubleGauge()`` methods are included to represent them in a "Old Motorola brick cell phones' style". The library is capable of managing a correct representation even in differently wired displays by letting configure the order in wich the digits are connected to the registers.  

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

The first mechanism frees the user from the load of calling the refreshing methods periodically, specially considering that long looping times (when executing **`for`**, **`while`** and **`do`** loops included), or the use of **`delay()`** type of commands could make the display flicker or simply stop until next refresh. The second option is given for the develpment of special display schemes. In any case the library is capable of working in any platform, using one way when possible, or the other always.  

# **Included Methods for SevenSeg74HC595 class**

|Method | Parameters|
|---|---|
|**_HC595_** |uint8_t **sclk**, uint8_t **rclk**, uint8_t **dio**(, bool **commAnode**(, uint8_t **dspDigits**))|
|**begin()**|None|
|**blink()**|None|
||unsigned long **onRate** (,unsigned long **offRate**)|
|**clear()**|None|
|**doubleGauge()**|int **levelLeft**, int **levelRight** (, char **labeLeft**(, char **labelRight**))|
|**fastRefresh()**|None|
|**fastSend()**|uint8_t **segments**, uint8_t **port**|
|**gauge()**|int **level** (, char **label**)|
||double **level** (, char **label**)|
|**getDigitsQty()**|None|
|**getDspValMax()**|None|
|**getDspValMin()**|None|
|**getInstanceNbr()**|None|
|**getMaxBlinkRate()**|None|
|**getMinBlinkRate()**|None|
|**isBlinking()**|None|
|**isWaiting()**|None|
|**noBlink()**|None|
|**noWait()**|None|
|**print()**|String **text**|
||int **value** (, bool **rgtAlgn** (, bool **zeroPad**))|
||double **value** (, unsigned int **decPlaces** (, bool **rgtAlgn** (, bool **zeroPad**)))|
|**refresh()**|None|
|**resetBlinkMask()**|None|
|**send()**|uint8_t **segments**, uint8_t **port**|
|**setBlinkMask()**|bool **blnkPort[]**|
|**setBlinkRate()**|unsigned long **newOnRate**, (unsigned long **newOffRate**)|
|**setDigitsOrder()**|uint8_t* **newOrder**, uint8_t **newOrderSize**|
|**setWaitChar()**|char **newWaitChar**|
|**setWaitRate()**|unsigned long **newWaitRate**|
|**stop()**|None|
|**wait()**|(unsigned long **waitRate**)|
|**write()**|uint8_t **segments**, uint8_t **port**|
||String **character**, uint8_t **port**|  

---  
## **Methods definition and use description**
---  

### **begin**();
### Description:
Attaches the display to the O.S. software timer service, which takes care of refreshing the display regularly. An unlimited amount of displays might be attached to the timer theoretically, as long as there's enough resources available for them, but in practice the refreshing work takes time, and the time taken is proportional to the quantity of displays attached. And as Timers consume time for other tasks done by the microcontroller, the time taken by the timers must be kept to minimal or the stability of the whole system will be compromised. As the time available to execute the refreshing time without risking the stability of the system depends on various factors, the number of supported displays has to be tested in each development environment at development time. To keep a safe limit, up to ten displays (including all classes defined in this library) can be attached to the timer service. If the stability of the system is compromised then the displays exceeding the number of tolerated units will have to be refreshed by periodically using the **`refresh()`** method. The maximum number of 10 might be modified in code.  
### Parameters:  
None
### Return value:  
true: If the display could be attached to the software timer service, or if the display was already attached to it. This not ensures system stability.  
false: If the display couldn't be attached to the software timer service, due to lack of free slots.  
### Use example:  
**`myLedDisp.begin();`**  

---

### **clear**();
### Use example:  
****

---
### **doubleGauge**(int **levelLeft**,int **levelRight**(, char **labelLeft**(, char **labelRight**)));
### Use example:  
**`myLedDisp.gauge(3, 2);`**  
**`myLedDisp.**gauge(2, 1, 'b', 't');`**  
**`myLedDisp.**gauge(1, 3, 'F');`**
**`myLedDisp.**gauge(4, 2,'d', 'b');`** //Error: 4 is outside the accepted values range  
**`myLedDisp.**gauge(3, 0, 'X');`** //Error: 'X' is not a "displayable" character  

---
### **refresh**();
### Description:
Refreshes the display, **only one digit per call**, the method takes care of registering which digit was redrawn last and move to the next until the last is reached and then restart from the first, and uses direct pin handling instead of using pre-built `shiftOut()` kind of methods. This working criteria has two consequences:
* The method works faster than redrawing all the digits each time and using the call to `shiftOut()` methods, so it is less time consuming and so is the most appropriate to be used within an , or software timer.
* When used by the developer to refresh the display from the code it must be called more frequently (starting with time needed to refresh each one of all the digits of the display, and then doing those calls periodically) to keep the display's cinematic effect. Failing to do so will be seen as display flickering, or some of the digits displayed more brighter than others.  
### Use example:  
**`myLedDisp.fastRefresh();`**

---
### **send**(uint8_t **segments**, uint8_t **port**);
### Description:
Sends one character to the display, using direct pin handling instead of using pre-built `shiftOut()` kind of methods. The parameters indicate which character and to which digit will be sent. This is the method used by fastRefresh() to send the digit when it has to be refreshed. **_Keep in mind_** that sending a character directly to the display has no connection to keep it displayed as it must be resent periodically to keep the cinematic effect. Also the refresh() and fastRefresh() methods will overwrite the character sent, explicitly called or by the ISR service if started by the **`begin()`** method. If the display is being kept by the provided timer (after a `.begin()`), the characters sent by this methods will be overwritten in the next ISR callback.  
### Parameters:  
**segments:** An unsigned short integer value representing which segments to turn on and which off to get the graphic representation of a character in the seven segment display, the corresponding value can be looked up in the **_charLeds[]** array definition in the header file of the library. In the case of a common cathode display the values there listed must be complemented.  
**port:** An unsigned short integer value representing the digit where the character will be sent, being the range of valid values 0 <= port <= dspDigits, the 0 value is the rightmost digit, the 1 value the second from the right and so on.
### Return value:  
None   
### Use example:  
**`myLedDisp.fastSend(0x88, 1);`** // Sends a capital A to the second digit from right to left.

---
### **gauge**(int **level**(, char **label**));

### Use example:  
**`myLedDisp.gauge(3);`**  
**`myLedDisp.**gauge(2, 'b');`**  
**`myLedDisp.**gauge(1, 'F');`**
**`myLedDisp.**gauge(4, 'd');`** //Error  
**`myLedDisp.**gauge(3, 'X');`** //Error  
**`myLedDisp.**gauge(4, 2,'d', 'b');`** //Error: 4 is outside the accepted values range  
**`myLedDisp.**gauge(3, 0, 'X');`** //Error: 'X' is not a "displayable" character  

---
### **gauge**(double **level**(, char **label**));

### Use example:  
**`myLedDisp.gauge(0.0);`**  
**`myLedDisp.gauge(0.4);`**  
**`myLedDisp.gauge(0.55, 'b');`**  
**`myLedDisp.gauge(1.0, 'F');`**  
**`myLedDisp.gauge(1.5, 'd');`** //Error: 4 is outside the accepted values range  
**`myLedDisp.gauge(3.0, 'X');`** //Error: 'X' is not a "displayable" character  

---
### **getDigitsQty**();
### Use example:  
**`uint8_t portsQty = myLedDisp.getDigistsQty();`**

---
### **getDspValMax**();
### Use example:  
**`long maxLimit = myLedDisp.getDspMax();`**

---
### **getDspValMin**();

### Use example:  
**`long minLimit = myLedDisp.getDspMin();`**

---
### **getInstanceNbr**();
### Use example:  
**`uint8_t dspNmbr = myLedDisp.getInstanceNbr();`**

---
### **getMaxBlinkRate**();

### Use example:  
**`myLedDisp.getMaxBlinkRate();`**

---
### **getMinBlinkRate**();

### Use example:  
**`myLedDisp.getMinBlinkRate();`**

---
### **isBlinking**();

### Use example:  
**`myLedDisp.isBlinking();`**

---
### **isWaiting**();

### Use example:  
**`myLedDisp.isWaiting();`**

---
### **noBlink**();

### Use example:  
**`myLedDisp.noBlink();`**

---
### **noWait**();
### Use example:  
**`myLedDisp.noWait();`**

---
### **print**(String **text**);

### Use example:  
**`myLedDisp.print("Hi");`** //Valid for 2 or more digits displays  
**`myLedDisp.print("Strt");`** //Valid for 4 or more digits displays  
**`myLedDisp.print("L.O.L.");`**  //Valid for 3 or more digits displays  
**`myLedDisp.print("36.70*");`** //Valid for 5 or more digits displays  
**`myLedDisp.print("........");`**  //Valid for 8 digits displays

---
### **print**(int **value**(, bool **rgtAlgn**(, bool **zeroPad**)));
### Use example:  
**`myLedDisp.print(12);`** //Displays '**``12  ``**' on a 4 digits display  
**`myLedDisp.print(12, true);`** //Displays '**``  12``**' on a 4 digits display  
**`myLedDisp.print(12, true, true);`** //Displays '**``0012``**' on a 4 digits display  
**`myLedDisp.print(-12);`** //Displays '**``-12     ``**' on a 8 digits display  
**`myLedDisp.print(-12, true);`** //Displays '**``-   12``**' on a 6 digits display  
**`myLedDisp.print(-12, true, true);`** //Displays '**``-012``**' on a 4 digits display    

---
### **print**(double **value**, unsigned int **decPlaces**(, bool **rgtAlgn**(, bool **zeroPad**)));
### Description:
### Use example (on a 4-bits display):  
**`myLedDisp.print(1.2, 2);`** //Displays '**``1.20 ``**'  
**`myLedDisp.print(1.2, 2, true);`** //Displays '**`` 1.20``**'  
**`myLedDisp.print(12, 2, true, true);`** //Displays '**``01.20``**'    
**`myLedDisp.print(-1.2, 2);`** //Displays '**``-1.20``**'  
**`myLedDisp.print(-1.28, 1, true);`** //Displays '**``- 1.2``**'  
**`myLedDisp.print(-1.28, 1, true, true);`** //Displays '**``-01.2``**'    
**`myLedDisp.print(-1.28, 3, true, true);`** //Error

---
### **refresh**();
### Description:
Refreshes the display, **all available digits per call**, the method takes care of registering which digit was redrawn first and each call starts from the next until the last is reached and then restart from the first, to minimize ghosting and keep all the digits brightness even, and uses pre-built **`shiftOut()`** kind   of methods. This working criteria has two consequences:
* The method works slower than the **`fastRefresh()`**, so it will take more time to execute.  
* When used by the developer to refresh the display from the code it will avoid ghosting or blinking effects being called less frequently to keep the display's cinematic effect.  
### Parameters:  
**None**  
### Return value:  
**None**   
### Use example:  
**`myLedDisp.refresh();`**  

---
### **resetBlinkMask**();
### Use example:  
**`myLedDisp.resetBlinkMask();`**

---
### **send**(uint8_t **segments**, uint8_t **port**);
### Description:
Sends one character to the display, using pre-built `shiftOut()` kind of methods, which takes unknown time to complete depending on the  implementation of the framework used to develop. The parameters indicate which character and to which digit will be sent. This is the method used by refresh() to send the digit when it has to be refreshed. **_Keep in mind_** that sending a character directly to the display has no connection to keep it displayed as it must be resent periodically to keep the cinematic effect. Also the refresh() and fastRefresh() methods will overwrite the character sent, explicitly called or by the Software Timer Service if started by the **`begin()`** method.
### Parameters:  
**segments:** An unsigned short integer value representing which segments to turn on and which off to get the graphic representation of a character in the seven segment display, the corresponding value can be looked up in the **_charLeds[]** array definition in the header file of the library. Any other uint8_t (char or unsigned short int are equivalent terms here) value is admissible, but the displayed result might not be easily recognized as a known ASCII character.  In the case of a common cathode display the values there listed must be complemented to calculate the value to send.    
**port:** An unsigned short integer value representing the digit where the character will be sent, being the range of valid values 0 <= port <= (dspDigits-1), the 0 value is the rightmost digit, the 1 value the second from the right and so on.
### Return value:  
None   
### Use example:  
**`myLedDisp.send(0x91, 2);`** // Sends a Y to the third digit from right to left.

---
### **setBlinkMask**(bool **blnkPort[]**);
### Use example:  
**`bool tstMask[4]{true, true, true, true};`**  
**`testResult = myLedDisp.blink();`**  //Sets all the  digits to blink in a 4 digits display  
**`tstMask[0] = true;`**  
**`tstMask[1] = false;`**  
**`tstMask[2] = false;`**  
**`tstMask[3] = false;`**  
**`myLedDisp.setBlinkMask(tstMask);`**  //Sets only the rightmost digit to blink in a 4 digits display  
**`tstMask[0] = false;`**  
**`tstMask[1] = true;`**  
**`tstMask[2] = true;`**  
**`myLedDisp.setBlinkMask(tstMask);`**    //Sets the two central digits to blink in a 4 digits display

---
### **setBlinkRate**(unsigned long **onRate**(,unsigned long **offRate**));
### Use example:  
**`myLedDisp.setBlinkRate(400);`** //Returns true and sets the blinking rate to 400 millisecs on, 400 millisecs off (symmetrical blink).  
**`myLedDisp.setBlinkRate(800, 200);`** //Returns true and sets the blinking rate to 800 millisecs on, 200 millisecs off (asymmetrical blink)  
**`myLedDisp.setBlinkRate(3000);`** //Returns false and the display blinking rate stays without change.  
**`myLedDisp.setBlinkRate(600, 3500);`** //Returns false and the display blinking rate stays without change.  

---
### **setDigitsOrder**(uint8_t* **newOrderPtr**);
### Description:
As different 7 segments dynamic displays based on two 74HC595 are differently wired, some implement the leftmost display port as the LSb of the shift register driving the port selection, some implement it as the MSb. When more than one display modules are used it adds a new level of hardware implementation that differs from one supplier to the other. The library implements a mechanism to provide the instantiated object to relate the positions of the display ports to the bits of the selection byte through an array. The array has the size of the display instantiated, and each array elment is meant to hold the number of the bit that selects the corresponding port, being the first element of the array (array[0]) the corresponding to the leftmost display digit, array[1], the next to it's right and so on. The array is default defined in the constructor as (0, 1, 2,...) that is the most usual implementation found. If the order needs to be changed the `.setDigitsOrder()` method is the way to set a new mapping.
### Parameters:  
**newOrderPtr**: pointer to an uint8_t array of **_dspDigits** lenght containing the position of the bit corresponding to each display port. Each value will be checked against the _dspDigits value to ensure that they are all in the range acceptable, 0 <= value <= _dspDigits - 1. If one of the values is out of the valid range no change will be done. Please note that no checking will be done to ensure all of the array values are different. A repeated value will be accepted.  
### Return value:  
true: All of the elements of the array were in the accepted range. The change was performed  
false: At least one of the values of the array passed were out of range. The change wasn't performed.  
### Use example:
**`uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};`** //Builds an array with the port order of the "DIY MORE 8-bit LED Display".  
**`myLedDisp.setDigitsOrder(diyMore8Bits);`** //Changes the display bit to port mapping according to the display characteristics.  

---
### **setWaitChar**(char **newWaitChar**);
### Use example:  
**`myLedDisp.setWaitRate('_');`** //Returns true and sets the wait character to '_'.  
**`myLedDisp.setWaitRate('#');`** //Error, returns false and the display wait character stays without change.  

---
### **setWaitRate**(unsigned long **newWaitRate**);
### Use example:  
**`myLedDisp.setWaitRate(400);`** //Returns true and sets the advancement rate to 400 millisecs.  
**`myLedDisp.setWaitRate(getMinBlinkRate() - 10);`** //Returns false and the display wait rate stays without change.  

---
### **stop**();
### Description:
Detaches the display from the Software Timer Service which takes care of refreshing the display regularly (if it was attached to it, if the display wasn't attached to the STS no modification is made). The method then checks the array (list) of active serviced displays, if none is left in that array, the timer service is stopped and deleted, and the array is deleted from the heap to free the allocated resources. This last actions are reversed when a new begin() method is executed in any display.  
### Parameters:  
**None**  
### Return value:  
true: The instance of the display was found and detached from the STS.  
false: The instance of the display wasn't found attached to the STS, no detach was carried as it wasn't needed.  
### Use example:  
**`myLedDisp.stop();`**  

---

### **wait**(unsigned long **newWaitRate**);
### Use example:  
**`myLedDisp.wait();`** //Returns true and shows advancement animation to the already set value.  
**`myLedDisp.wait(800);`** //Returns true and sets the advancement animation rate to 800 millisecs.  
**`myLedDisp.wait(getMaxBlinkRate() + 10);`** //Returns false and the display stays without change.  

---
### **write**(uint8_t **segments**, uint8_t **port**);
### Use example:  
**`myLedDisp.write(0xA4, 1);`** // Modifies the displayed data, placing a '2' in the second digit from right to left in a common anode display.

---

### **write**(String **character**, uint8_t **port**);

### Use example:  
**`myLedDisp.write("J", 1);`** // Modifies the displayed data, placing a 'J' in the second digit from right to left.  

