ArduinoAlarmClock
===========

An arduino alarm clock.

Features
--------
* Date and Time
* Alarm 1: Mon - Fri
* Alarm 2: Sat - Sun
* 12 hr / 24 hr Mode

Optional
--------
* IR reciever
* Temperature and Humidity (DHT11)


Files
-----
* clock_lcd - implements Alarm clock on a 16x2 LCD display (parallel mode).
* clock_u8g - implements Alarm clock on a 128x64 LCD display ST7920 (spi mode).


Wiring for clock_lcd
--------------------
    LCD 16x2                  Arduino Uno               
    RS ---------------------- Pin9
    Enable ------------------ Pin8
    D4 ---------------------- Pin7
    D5 ---------------------- Pin6
    D6 ---------------------- Pin5
    D7 ---------------------- Pin4

Wiring for clock_u8g
--------------------
    LCD 128x64                Arduino Uno               
    EN ---------------------- Pin6
    R/W --------------------- Pin5
    RS ---------------------- Pin4

    
TODO
-----
