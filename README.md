Casio Can project {#mainpage}
============

Documentation for casio-can.
-------------

The casio-can is a clock that works with CAN communication, to configurate the clock you have to send
a message through CAN with the following structure:

CAN-ID Message id

Byte 0 Single frame code

Byte 1 Message type

Byte 2 Parameter 1

Byte 3 Parameter 2

Byte 4 Parameter 3

Byte 5 Parameter 4

Byte 6 NA

Byte 7 NA

The value of the message ID has to be 0x111


**The value of message type will indicate the type of function to be programmed in the clock**

1 - Time, 2- Date, 3 - Alarm

**In the case of time**
Parameter 1 will indicate the hours, Parameter 2 will indicate the minutes and Parameter 3 will indicate the seconds in BCD format

**In the case of the date**

Parameter 1 the day of the month, Parameter 2 will indicate the month and Parameter 3 will indicate the two most significant figures of the year and finally Parameter 4 will indicate the two least significant figures of the year in BCD format

**In the case of alarm**

Parameter 1 will indicate the hours, Parameter 2 will indicate the minutes in BCD format. Parameter 3 and 4 will not be used


```
