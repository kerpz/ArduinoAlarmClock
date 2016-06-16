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
* images - screenshots


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

Serial control for setting the date, time, and alarm. (LCD 16x2)
---------------
* Press [Enter] for toggling setting/normal mode.
* Press [Left] [Right] for navigation and [Up] [Down] for values adjustment.
    
Serial control for setting the date, time, and alarm. (LCD 128x64)
---------------
* Press [Left] [Right] for navigating the pages (Main Page, Date/Time Settings, Alarm Settings).
* Press [Enter] for toggling setting/normal mode (for Date/Time Page and Alarm Page).
* Press [Left] [Right] for navigation and [Up] [Down] for values adjustment.

Alarm Settings
---------------
* A1 / Alarm 1 = Alarm for Mon - Fri
* A2 / Alarm 2 = Alarm for Sat - Sun

    
Screenshots (LCD 16x2)
---------------

![Alt text](https://raw.github.com/kerpz/ArduinoAlarmClock/master/images/arduino_atmega328_lcd_16x2_01.png "Main Display")
![Alt text](https://raw.github.com/kerpz/ArduinoAlarmClock/master/images/arduino_atmega328_lcd_16x2_02.png "Date Time Settings")
![Alt text](https://raw.github.com/kerpz/ArduinoAlarmClock/master/images/arduino_atmega328_lcd_16x2_03.png "Alarm Settings")

Screenshots (Serial control for setting the date, time, and alarm)
---------------

![Alt text](https://raw.github.com/kerpz/ArduinoAlarmClock/master/images/arduino_atmega328_serial_01.png "Serial Control")

Screenshots (LCD 128x64)
---------------

![Alt text](https://raw.github.com/kerpz/ArduinoAlarmClock/master/images/arduino_atmega328_lcd_128x64_03.png "Main Display")

TODO
-----
