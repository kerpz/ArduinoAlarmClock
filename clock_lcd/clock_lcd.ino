/*
 Author:
 - Philip Bordado (kerpz@yahoo.com)

 Hardware:
 - Arduino UNO (Compatible board)
 - Tact switch
 - Piezo Buzzer
 - LCD 16x2 and 10k Potentiometer
 
 Software:
 - Arduino 1.6.9
 - LiquidCrystal (Library)
   
 Arduino Pin Mapping:

 - 00 = Serial RX
 - 01 = Serial TX
 - 02
 - 03
 - 04 = LCD D7
 - 05 = LCD D6
 - 06 = LCD D5
 - 07 = LCD D4
 - 08 = LCD EN
 - 09 = LCD RS
 - 10
 - 11
 - 12
 - 13 = Piezo buzzer (+)

 - 14 = (A0)
 - 15 = (A1)
 - 16 = (A2)
 - 17 = (A3) Navigation Button
 - 18 = (A4) I2C
 - 19 = (A5) I2C

 Potentiometer
 - END = 5V
 - MID = LCD VO
 - END = GND
  
*/

#define DS3231_I2C_ADDRESS 0x68

//#include <EEPROM.h>
#include "Wire.h"


#define LCD_i2c TRUE // Using LCD 16x2 I2C mode

#if defined(LCD_i2c)
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7);
#else
#include <LiquidCrystal.h>
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
#endif

//#include <DHT.h>
//DHT dht(7, DHT11);

//#include <IRremote.h>
//IRrecv irrecv(11);
//decode_results results;


//float temperature, humidity, heat_index;

// the 8 arrays that form each segment of the custom numbers
byte bar1[8] = 
{
        B11110,
        B11110,
        B11110,
        B11110,
        B11110,
        B11110,
        B11110,
        B11110
};
byte bar2[8] =
{
        B01111,
        B01111,
        B01111,
        B01111,
        B01111,
        B01111,
        B01111,
        B01111
};
byte bar3[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};
byte bar4[8] =
{
        B11110,
        B11110,
        B00000,
        B00000,
        B00000,
        B00000,
        B11110,
        B11110
};
byte bar5[8] =
{
        B01111,
        B01111,
        B00000,
        B00000,
        B00000,
        B00000,
        B01111,
        B01111
};
byte bar6[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};
byte bar7[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B01111,
        B01111
};
byte bar8[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};

byte incomingByte = 0;   // for incoming serial data

byte second = 0, minute = 0, hour = 0, weekday = 0, day = 21, month = 6, feb = 28;
int year = 2016;

byte a1_second = 0, a1_minute = 0, a1_hour = 0, a1_duration = 0; // alarm 1
byte a2_second = 0, a2_minute = 0, a2_hour = 0, a2_duration = 0; // alarm 2
//bool a1_enabled = 0; // 0=off,1=on
//bool a2_enabled = 0; // 0=off,1=on

bool a_signal = 0; // 0=off,1=on

byte adj_select = 0;   // 0=none,1=month,2=day,3=year,4=hour,5=minute,6=second,7=hour_mode
byte pag_select = 0;   // page
//byte a_setmode = 0;   // 0=none,1-7=a_weekbits,8=a_hour,9=a_minute,10=a_duration

bool hour_mode = 1;   // 0=24hr,1=12hr

const char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char *weekdays[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char *hour_12s[]={"Am","Pm"};

float Temperature;

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void writeRTC()
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(weekday)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(day)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year - 2000)); // set year (0 to 99)
  Wire.endTransmission();
}
void readRTC()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  second = bcdToDec(Wire.read() & 0x7f);
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0x3f);
  weekday = bcdToDec(Wire.read());
  day = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read()) + 2000;
}
void readTEMP() {
  // Checks the internal thermometer on the DS3231 and returns the 
  // temperature as a floating-point value.

  // Updated / modified a tiny bit from "Coding Badly" and "Tri-Again"
  // http://forum.arduino.cc/index.php/topic,22301.0.html
  
  byte tMSB, tLSB;
  
  // temp registers (11h-12h) get updated automatically every 64s
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);

  // Should I do more "if available" checks here?
  if(Wire.available()) {
    tMSB = Wire.read(); //2's complement int portion
    tLSB = Wire.read(); //fraction portion

    Temperature = ((((short)tMSB << 8) | (short)tLSB) >> 6) / 4.0;
  }
  else {
    Temperature = -9999; // Some obvious error value
  }
}

int get_weekday(int year, int month, int day)
{
  int adjustment, mm, yy;

  adjustment = (14 - month) / 12;
  mm = month + 12 * adjustment - 2;
  yy = year - adjustment;
  return (day + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

void updateClock() { // call this every second
  second++;
  if (second > 59) { // second
    minute++;
    second = 0;
    if (minute > 59) { // minute
      hour++;
      minute = 0;
      if (hour > 23) { // hour
        weekday++;
        day++;
        hour = 0;
        if (weekday > 6) { // weekday
          weekday = 0;
        }
        if (day > 31 || (day > 30 && month == (4|6|9|11)) || (day > feb && month == 2))  { // day
          day = 1;
          month++;
          if (month > 12) { // month
            year++;
            if (year%400 == 0 || (year%100 != 0 && year%4 == 0)) { // leap year
              feb = 29;
            }
          }
        }
      }
    }
  }
}

void checkAlarm() { // call this every second
  static unsigned short int alarmTick = 0;
  if (!a_signal) {
    if (hour == a1_hour && minute == a1_minute && second == 0 > 0 && a1_duration > 0 && weekday < 6 && weekday > 0) { // mon - fri
      a_signal = true;
    }
    else if (hour == a2_hour && minute == a2_minute && second == 0 > 0 && a2_duration > 0 && (weekday == 6 || weekday == 0)) { // sat - sun
      a_signal = true;
    }
  }
  else { // 400 ms delay
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
    delay(50);
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
    delay(50);
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
    delay(50);
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    alarmTick++;
    if (alarmTick >= a1_duration && weekday < 6 && weekday > 0) {
      alarmTick = 0;
      a_signal = false;
    }
    else if (alarmTick >= a2_duration && (weekday == 6 || weekday == 0)) {
      alarmTick = 0;
      a_signal = false;
    }
  }
}

void custom0(int col)
{ // uses segments to build the number 0
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(8); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(2);  
  lcd.write(6);  
  lcd.write(1);
}

void custom1(int col)
{
  lcd.setCursor(col,0);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
  lcd.setCursor(col,1);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
}

void custom2(int col)
{
  lcd.setCursor(col,0);
  lcd.write(5);
  lcd.write(3);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(2);
  lcd.write(6);
  lcd.write(6);
}

void custom3(int col)
{
  lcd.setCursor(col,0);
  lcd.write(5);
  lcd.write(3);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(7);
  lcd.write(6);
  lcd.write(1); 
}

void custom4(int col)
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(6);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}

void custom5(int col)
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(3);
  lcd.write(4);
  lcd.setCursor(col, 1);
  lcd.write(7);
  lcd.write(6);
  lcd.write(1);
}

void custom6(int col)
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(3);
  lcd.write(4);
  lcd.setCursor(col, 1);
  lcd.write(2);
  lcd.write(6);
  lcd.write(1);
}

void custom7(int col)
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(8);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}

void custom8(int col)
{
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(3); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(2);  
  lcd.write(6);  
  lcd.write(1);
}

void custom9(int col)
{
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(3); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(7);  
  lcd.write(6);  
  lcd.write(1);
}

void custom_blank(int col)
{
  lcd.setCursor(col, 0); 
  lcd.write(32);  
  lcd.write(32);  
  lcd.write(32);  
  lcd.setCursor(col, 1); 
  lcd.write(32);  
  lcd.write(32);  
  lcd.write(32);  
}

void custom_colon(int col)
{
  lcd.setCursor(col, 0); 
  lcd.write(7);  
  lcd.setCursor(col, 1); 
  lcd.write(7);
}

void printNumber(int value, int col) {
  if (value == 0) {
    custom0(col);
  } if (value == 1) {
    custom1(col);
  } if (value == 2) {
    custom2(col);
  } if (value == 3) {
    custom3(col);
  } if (value == 4) {
    custom4(col);
  } if (value == 5) {
    custom5(col);
  } if (value == 6) {
    custom6(col);
  } if (value == 7) {
    custom7(col);
  } if (value == 8) {
    custom8(col);
  } if (value == 9) {
    custom9(col);
  }      
}  

void adjClock(byte adj) { // 0 --, 1 ++
  switch (adj_select) {
    case 1: // hour
      if (adj == 1) {
        hour++;
        if (hour > 23) hour = 0;
      }
      else {
        hour--;
        if (hour > 23) hour = 23;
      }
      break;
    case 2: // minute
      if (adj == 1) {
        minute++;
        if (minute > 59) minute = 0;
      }
      else {
        minute--;
        if (minute > 59) minute = 59;
      }
      break;
    case 3: // second
      if (adj == 1) {
        second++;
        if (second > 59) second = 0;
      }
      else {
        second--;
        if (second > 59) second = 59;
      }
      //updateClock();
      break;
    case 4: // hour mode
      hour_mode = !hour_mode;
      break;
    case 5: // month
      if (adj == 1) {
        month++;
        if (month > 12) month = 1;
      }
      else {
        month--;
        if (month < 1) month = 12;
      }
      break;
    case 6: // day
      if (adj == 1) {
        day++;
      }
      else {
        day--;
        if (day > 31) day = 31;
      }
      break;
    case 7: // year
      if (adj == 1) {
        year++;
      }
      else {
        year--;
      }
      break;

    case 8: // a1_hour
      if (adj == 1) {
        a1_hour++;
        if (a1_hour > 23) a1_hour = 0;
      }
      else {
        a1_hour--;
        if (a1_hour > 23) a1_hour = 23;
      }
      break;
    case 9: // a1_minute
      if (adj == 1) {
        a1_minute++;
        if (a1_minute > 59) a1_minute = 0;
      }
      else {
        a1_minute--;
        if (a1_minute > 59) a1_minute = 59;
      }
      break;
    case 10: // a1_duration
      if (adj == 1) {
        a1_duration++;
        if (a1_duration > 90) a1_duration = 1;
      }
      else {
        a1_duration--;
        if (a1_duration > 90) a1_duration = 90;
      }
      break;

    case 11: // a2_hour
      if (adj == 1) {
        a2_hour++;
        if (a2_hour > 23) a2_hour = 0;
      }
      else {
        a2_hour--;
        if (a2_hour > 23) a2_hour = 23;
      }
      break;
    case 12: // a2_minute
      if (adj == 1) {
        a2_minute++;
        if (a2_minute > 59) a2_minute = 0;
      }
      else {
        a2_minute--;
        if (a2_minute > 59) a2_minute = 59;
      }
      break;
    case 13: // a2_duration
      if (adj == 1) {
        a2_duration++;
        if (a2_duration > 30) a2_duration = 0;
      }
      else {
        a2_duration--;
        if (a2_duration > 30) a2_duration = 30;
      }
      break;
    default:
      break;
  }
  weekday = get_weekday(year, month, day);
}


void decodeSerial(byte incomingByte) {
  switch (incomingByte) {
    /*
    case 0x9: // tab // switch // for 2 buttons
      if (adj_select > 0) {
        adjClock(1);
      }
      else {
        pag_select++;
        if (pag_select > 2)
          pag_select = 0;
      }
       break;
    case 0x1B: // esc // edit // for 2 buttons
      if (pag_select == 1) {
        adj_select++;
        if (adj_select > 7)
          adj_select = 0;
      }
      else if (pag_select == 2) {
        if (adj_select == 0)
          adj_select = 8;
        else
          adj_select++;
        if (adj_select > 13)
          adj_select = 0;
      }
       break;
     */
    case 0xD: // enter // ok
      // set enabled
      if (pag_select == 1) {
        if (adj_select == 0)
          adj_select = 1;
        else
        {
          adj_select = 0;
          writeRTC();
        }
      }
      else if (pag_select == 2) {
        if (adj_select == 0)
          adj_select = 8;
        else
          adj_select = 0;
      }
      break;
    case 0x41: // up
      adjClock(1);
      break;
    case 0x42: // down
      adjClock(0);
      break;
    case 0x43: // right
      if (!adj_select) {
        if (pag_select < 2)
          pag_select++;
      }
      else if (pag_select == 1 && adj_select < 7) {
        adj_select++;
      }
      else if (pag_select == 2 && adj_select < 13) {
        adj_select++;
      }
      break;
    case 0x44: // left
      if (!adj_select) {
        if (pag_select > 0)
          pag_select--;
      }
      else if (pag_select == 1 && adj_select > 1) {
        adj_select--;
      }
      else if (pag_select == 2 && adj_select > 8) {
        adj_select--;
      }
      break;
    //case REPEAT:
      //Serial.println("Repeat");
      //break;
    default:
      Serial.print("Serial Received: 0x");
      Serial.println(incomingByte, HEX);
      break;
  }
}  
/*
void decodeIR(decode_results *results) {
  if (results->decode_type == NEC) { // pioneer remote
    switch (results->value) {
      case 0xFFE01F: // play, select, enter
        adj_select = !adj_select;
        break;
      case 0xB54A02FD: // up
      case 0xFF609F:
        adjClock(1);
        break;
      case 0xB54A827D: // down
      case 0xFF22DD:
        adjClock(0);
        break;
      case 0xB54A42BD: // left
      case 0xFFE21D:
          if (adj_select > 1 && adj_select != 0) {
            adj_select--;
          }
          break;
      case 0xB54AC23D: // right
      case 0xFF02FD:
        if (adj_select < 13 && adj_select != 0) {
          adj_select++;
        }
        break;
      //case 0xF50A0FF0: // 0
      //  break;
      //case 0xF50A8F70: // 1
      //  break;
      //case 0xF50A4FB0: // 2
      //  break;
      //case 0xF50ACF30: // 3
      //  break;
      //case 0xF50A2FD0: // 4
      //  break;
      //case 0xF50AAF50: // 5
      //  break;
      //case 0xF50A6F90: // 6
      //  break;
      //case 0xF50AEF10: // 7
      //  break;
      //case 0xF50A1FE0: // 8
      //  break;
      //case 0xF50A9F60: // 9
      //  break;
      //case REPEAT:
        //Serial.println("Repeat");
        //break;
      default:
        Serial.print("IR Received NEC: 0x");
        Serial.println(results->value, HEX);
        break;
    }
  }
}
*/

/*
void read_dht11(void) {
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);
  heat_index = dht.computeHeatIndex(temperature, humidity, false);
}
*/

void displayTime(bool bblink) {
  byte i;
  bool hour_12 = 0;
  
  if (hour_mode == 1) {
    if (hour == 0) {
      i = 12; // Am
      hour_12 = 0;
    }
    else if (hour > 12) {
      i = hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      i = hour; // Am
      hour_12 = 0;
    }
  }
  else {
    i = hour;
  }

  if (i/10 > 0)
    printNumber(i/10, 0); // hour
  else
    custom_blank(0);
  i %= 10;
  printNumber(i, 3); // hour
  
  if (adj_select == 0 && bblink) {
    lcd.setCursor(6, 0);
    lcd.write(32);
    lcd.setCursor(6, 1);
    lcd.write(32);
  }
  else {
    custom_colon(6);
  }

  i = minute;
  printNumber(i/10, 7); // minute
  i %= 10;
  printNumber(i, 10); // minute

  lcd.setCursor(13, 0);
  lcd.print(" ");
  if (hour_mode == 1) {
    lcd.print(hour_12s[hour_12]);
  }
  else {
    lcd.print("Mi");
  }

  lcd.setCursor(13, 1);
  lcd.print(" ");
  if (second < 10) {
    lcd.print("0");
  }
  lcd.print(second);
}  

void displayTimeSettings(bool bblink) {
  short int i;
  bool hour_12 = 0;

  if (hour_mode == 1) {
    if (hour == 0) {
      i = 12; // Am
      hour_12 = 0;
    }
    else if (hour > 12) {
      i = hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      i = hour; // Am
      hour_12 = 0;
    }
  }
  else {
    i = hour;
  }

  lcd.setCursor(0,0);
  if (adj_select == 1 && bblink) {
    lcd.print("  ");
  }
  else{
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print(":");
  if (adj_select == 2 && bblink) {
    lcd.print("  ");
  }
  else {
    i = minute;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print(":");
  if (adj_select == 3 && bblink) {
    lcd.print("  ");
  }
  else {
    i = second;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print("      ");
  
  if (adj_select == 4 && bblink) {
    lcd.print("  ");
  }
  else {
    if (hour_mode == 1) {
      lcd.print(hour_12s[hour_12]);
    }
    else {
      lcd.print("Mi");
    }
  }

  lcd.setCursor(0,1);
  if (adj_select == 5 && bblink) {
    lcd.print("  ");
  }
  else {
    i = month;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print("/");
  if (adj_select == 6 && bblink) {
    lcd.print("  ");
  }
  else {
    i = day;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print("/");
  if (adj_select == 7 && bblink) {
    lcd.print("    ");
  }
  else {
    i = year;
    lcd.print(i/1000);
    i %= 1000;
    lcd.print(i/100);
    i %= 100;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }

  lcd.print("   ");
  lcd.print(weekdays[weekday]);
}

void displayAlarmSettings(bool bblink) {
  short int i;
  bool hour_12 = 0;
  
  if (hour_mode == 1) {
    if (a1_hour == 0) {
      i = 12; // Am
      hour_12 = 0;
    }
    else if (a1_hour > 12) {
      i = a1_hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      i = a1_hour; // Am
      hour_12 = 0;
    }
  }
  else {
    i = a1_hour;
  }

  lcd.setCursor(0,0);
  lcd.print("A1: ");
  if (adj_select == 8 && bblink) {
    lcd.print("  ");
  }
  else{
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print(":");
  if (adj_select == 9 && bblink) {
    lcd.print("  ");
  }
  else {
    i = a1_minute;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }

  lcd.print(" ");
  if (hour_mode == 1) {
    lcd.print(hour_12s[hour_12]);
  }
  else {
    lcd.print("Mi");
  }
  lcd.print(" ");

  if (adj_select == 10 && bblink) {
    lcd.print("   ");
  }
  else {
    if (a1_duration > 0) {
      if (a1_duration < 10) {
        lcd.print("0");
      }
      lcd.print(a1_duration);
      lcd.print("s");
    }
    else {
      lcd.print("Off");
    }
  }
  
  if (hour_mode == 1) {
    if (a2_hour == 0) {
      i = 12; // Am
      hour_12 = 0;
    }
    else if (a2_hour > 12) {
      i = a2_hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      i = a2_hour; // Am
      hour_12 = 0;
    }
  }
  else {
    i = a2_hour;
  }

  lcd.setCursor(0,1);
  lcd.print("A2: ");
  if (adj_select == 11 && bblink) {
    lcd.print("  ");
  }
  else{
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }
  lcd.print(":");
  if (adj_select == 12 && bblink) {
    lcd.print("  ");
  }
  else {
    i = a2_minute;
    lcd.print(i/10);
    i %= 10;
    lcd.print(i);
  }

  lcd.print(" ");
  if (hour_mode == 1) {
    lcd.print(hour_12s[hour_12]);
  }
  else {
    lcd.print("Mi");
  }
  lcd.print(" ");

  if (adj_select == 13 && bblink) {
    lcd.print("   ");
  }
  else {
    if (a2_duration > 0) {
      if (a2_duration < 10) {
        lcd.print("0");
      }
      lcd.print(a2_duration);
      lcd.print("s");
    }
    else {
      lcd.print("Off");
    }
  }
}

void procClock() {
  static unsigned long msTick = millis();
  static bool dispblink = false;

  if (millis() - msTick >= 500) { // run every 500 ms
    msTick = millis();
    dispblink = !dispblink;

    switch (pag_select) {
      case 0: // clock main display
        displayTime(dispblink);
        break;
      case 1: // time/date settings
        displayTimeSettings(dispblink);
        break;
      case 2: // alarm settings
        displayAlarmSettings(dispblink);
        break;
      default:
        break;
    }
    
    if (dispblink) { // 1 sec
      updateClock();
      checkAlarm();
    }
  }
}

void procButtons() {
  static unsigned long buttonsTick = 0;
  static int button_press_old = HIGH;
  static byte cnt;

  int button_press_new = digitalRead(17);

  if (button_press_new != button_press_old) { // change state
    if (button_press_new == HIGH) { // on released
      if (millis() - buttonsTick >= 1000) { // long press 1s
        
      }
      else if (millis() - buttonsTick >= 5) { // short press 5ms
        if (adj_select == 0) {
          // change page
          pag_select++;
          if (pag_select > 2) {
            pag_select = 0;
          }
        }
        else {
          adjClock(1);
        }
      }
      buttonsTick = 0; // reset timer
    }
    else { // on pressed
      buttonsTick = millis(); // start timer
    }
    button_press_old = button_press_new;
  }

  if (button_press_new == LOW) { // while pressed
      if (millis() - buttonsTick == 5) { // beep @ 5 ms
        pushPinHi(13, 50); // beep 50ms
      }
      else if (millis() - buttonsTick == 1000) { // beep @  1s
        // set enabled
        if (pag_select == 1) {
          if (adj_select == 0)
            adj_select = 1;
          else
            adj_select = 0;
            displayTimeSettings(1);
        }
        else if (pag_select == 2) {
          if (adj_select == 0)
            adj_select = 8;
          else
            adj_select = 0;
            displayAlarmSettings(1);
        }
        pushPinHi(13, 50); // beep 50ms
      }
      // here
      if (adj_select > 0) {
          // haxx
          if (pag_select == 1)
            cnt = adj_select;
          else if (pag_select == 2)
            cnt = adj_select - 7;

        if (millis() - buttonsTick == (1000 * (cnt + 1))) { // every 1s change adj_select
          if (pag_select == 1) {
            if (adj_select < 7) {
              adj_select++;
            }
            else {
              adj_select = 0; // off
            }
            displayTimeSettings(1);
          }
          else if (pag_select == 2) {
            if (adj_select < 13) {
              adj_select++;
            }
            else {
              adj_select = 0; // off
            }
            displayAlarmSettings(1);
          }
          pushPinHi(13, 50); // beep 50ms
        }
      }
  }
}

void pushPinHi(byte pin, unsigned char delayms)
{
  digitalWrite(pin, HIGH);
  delay(delayms);
  digitalWrite(pin, LOW);
}

void setup()
{
  pinMode(13, OUTPUT); // piezo buzzer

  pinMode(9, OUTPUT); // ir led test
  digitalWrite(9, HIGH); // Configure internal pull-up resistor

  pinMode(17, INPUT); // Button
  digitalWrite(17, HIGH); // Configure internal pull-up resistor

  Serial.begin(115200);
  //irrecv.enableIRIn(); // Start the receiver

#if defined(LCD_i2c)
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH); // NOTE: You can turn the backlight off by setting it to LOW instead of HIGH
#endif

  lcd.begin(16, 2); // sets the LCD's rows and colums:

  // assignes each segment a write number
  lcd.createChar(1,bar1);
  lcd.createChar(2,bar2);
  lcd.createChar(3,bar3);
  lcd.createChar(4,bar4);
  lcd.createChar(5,bar5);
  lcd.createChar(6,bar6);
  lcd.createChar(7,bar7);
  lcd.createChar(8,bar8);

  // initial beep
  for (int i=0; i<3; i++) {
    pushPinHi(13, 50); // beep 50ms
    delay(80);
  }

  weekday = get_weekday(year, month, day);
  
  Wire.begin();
  readRTC();
}

void loop()
{
  procButtons();

  procClock();

  // Serial listen
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    decodeSerial(incomingByte);
  }
  /*
  // IR listen
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    decodeIR(&results);
    irrecv.resume(); // Receive the next value
  }
  */
}
