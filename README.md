# Products_Inspection & tracking
This program detects presence of a product(In our case a bottle) on a conveyor then inspects wether it is capped or not(presence/absence of caps).
If a product(bottle) is capped , it is marked as good else is a bad product and will be ejected at ejection point.
The program uses an encoder to track the movement of products on  a conveyor.
Products are inspected on First In First Out bases (FIFO) hence our program uses FIFO based registers.
Program was developed and tested on Atmega 2560 mcu - But with slight configuration changes , we can get it to run on any AVR based MCUs.



