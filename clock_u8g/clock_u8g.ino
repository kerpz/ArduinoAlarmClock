#include <U8glib.h>
#include <DHT.h>
#include <IRremote.h>

U8GLIB_ST7920_128X64_4X u8g(6, 5, 4); // SPI Com: SCK = en = 6, MOSI = rw = 5, CS = di = 4
DHT dht(7, DHT11);

IRrecv irrecv(11);
decode_results results;

// speaker pwm (9/10/11)

#define runEvery(t) for (static typeof(t) _lasttime;(typeof(t))((typeof(t))millis() - _lasttime) > (t);_lasttime += (t))

float temperature, humidity, heat_index;

byte firstLine = 27;
byte firstLineA = 8;
byte firstLineB = 17;
byte firstLineC = 27;
byte secondLine = 38;
byte thirdLine = 50;
byte fourthLine = 62;
//byte secLpos;

byte incomingByte = 0;   // for incoming serial data

byte second = 0, minute = 0, hour = 0, weekday = 0, day = 5, month = 6, feb = 28;
int year = 2016;

byte a1_second = 0, a1_minute = 0, a1_hour = 0, a1_duration = 20; // alarm 1
byte a2_second = 0, a2_minute = 0, a2_hour = 0, a2_duration = 0; // alarm 2
bool a1_enabled = 0; // 0=off,1=on
bool a2_enabled = 0; // 0=off,1=on

bool a_signal = 0; // 0=off,1=on

byte adj_select = 0;   // 0=none,1=month,2=day,3=year,4=hour,5=minute,6=second,7=hour_mode
//byte a_setmode = 0;   // 0=none,1-7=a_weekbits,8=a_hour,9=a_minute,10=a_duration

bool hour_mode = 0;   // 0=24hr,1=12hr

const char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char *weekdays[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char *hour_modes[]={"24h","12h"};
const char *hour_12s[]={"Am","Pm"};

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
  if (hour == a1_hour && minute == a1_minute && second == 0 && weekday < 6 && weekday > 0) { // mon - fri
    a_signal = 1;
  }
  else if (hour == a2_hour && minute == a2_minute && second == 0 && (weekday == 6 || weekday == 0)) { // sat - sun
    a_signal = 1;
  }
}
/*
void setAlarm(byte adj) { // 0 --, 1 ++
  switch (a_setmode) {
    case 1: // sun bit
    case 2: // mon bit
    case 3: // tue bit
    case 4: // wed bit
    case 5: // thu bit
    case 6: // fri bit
    case 7: // sat bit
      if (bitRead(a_weekbits, a_setmode - 1)) {
        bitClear(a_weekbits, a_setmode - 1);
      }
      else {
        bitSet(a_weekbits, a_setmode - 1);
      }
      break;
    case 8: // hour
      if (adj == 1) {
        a_hour++;
        if (a_hour > 23) a_hour = 0;
      }
      else {
        a_hour--;
        if (a_hour > 23) a_hour = 23;
      }
      break;
    case 9: // minute
      if (adj == 1) {
        a_minute++;
        if (a_minute > 59) a_minute = 0;
      }
      else {
        a_minute--;
        if (a_minute > 59) a_minute = 59;
      }
      break;
    case 10: // duration
      if (adj == 1) {
        a_duration++;
        if (a_duration > 90) a_duration = 1;
      }
      else {
        a_duration--;
        if (a_duration > 90) a_duration = 90;
      }
      break;
    default:
      break;
  }
}
*/
void setClock(byte adj) { // 0 --, 1 ++
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
      updateClock();
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
        if (a2_duration > 90) a2_duration = 1;
      }
      else {
        a2_duration--;
        if (a2_duration > 90) a2_duration = 90;
      }
      break;
    default:
      break;
  }
  weekday = get_weekday(year, month, day);
}

void decodeSerial(byte incomingByte) {
  switch (incomingByte) {
    case 0xD: // enter
      adj_select = !adj_select;
      break;
    case 0x41: // up
      setClock(1);
      break;
    case 0x42: // down
      setClock(0);
      break;
    case 0x43: // right
      if (adj_select < 13 && adj_select != 0) {
        adj_select++;
      }
      break;
    case 0x44: // left
      if (adj_select > 1 && adj_select != 0) {
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

void decodeIR(decode_results *results) {
  if (results->decode_type == NEC) { // pioneer remote
    switch (results->value) {
      case 0xFFE01F: // play, select, enter
        adj_select = !adj_select;
        break;
      case 0xB54A02FD: // up
      case 0xFF609F:
        setClock(1);
        break;
      case 0xB54A827D: // down
      case 0xFF22DD:
        setClock(0);
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
      /*
      case 0xF50A0FF0: // 0
        break;
      case 0xF50A8F70: // 1
        break;
      case 0xF50A4FB0: // 2
        break;
      case 0xF50ACF30: // 3
        break;
      case 0xF50A2FD0: // 4
        break;
      case 0xF50AAF50: // 5
        break;
      case 0xF50A6F90: // 6
        break;
      case 0xF50AEF10: // 7
        break;
      case 0xF50A1FE0: // 8
        break;
      case 0xF50A9F60: // 9
        break;
      */
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

void read_dht11(void) {
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);
  heat_index = dht.computeHeatIndex(temperature, humidity, false);
}

void draw_page_1(bool blink) {
  byte dhour;
  bool hour_12 = 0;
  
  if (hour_mode == 1) {
    if (hour == 0) {
      dhour = 12; // Am
      hour_12 = 0;
    }
    else if (hour > 12) {
      dhour = hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      dhour = hour; // Am
      hour_12 = 0;
    }
  }
  else {
    dhour = hour;
  }

  u8g.setFont(u8g_font_fub25n);   // BIG font
  // hour
  if (adj_select == 1 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.setPrintPos(0, firstLine);
  if (dhour < 10) {
    u8g.print("0");
    //u8g.setPrintPos(19, firstLine);  
  }
  u8g.print(dhour);
  u8g.setColorIndex(1);

  u8g.setPrintPos(35, firstLine);
  if (adj_select == 0 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.print(":");
  u8g.setColorIndex(1);

  // minute
  if (adj_select == 2 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.setPrintPos(47, firstLine);
  if (minute < 10) {
    u8g.print("0");
    //u8g.setPrintPos(65, firstLine);
  }
  u8g.print(minute);
  u8g.setColorIndex(1);

  u8g.setFont(u8g_font_helvB08);

  // Firstline C // second
  u8g.setPrintPos(85, firstLineC);
  if (adj_select == 0 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.print(":");
  u8g.setColorIndex(1);

  u8g.setPrintPos(88, firstLineC);
  if (adj_select == 3 && blink) {
    u8g.setColorIndex(0);
  }
  if (second < 10) {
    u8g.print("0");
  }
  u8g.print(second);
  u8g.setColorIndex(1);
  
  u8g.print(" ");

  if (adj_select == 4 && blink) {
    u8g.setColorIndex(0);
  }
  if (hour_mode == 1) {
    u8g.print(hour_12s[hour_12]);
  }
  else {
    u8g.print("Mi");
  }
  u8g.setColorIndex(1);

  // Firstline A // temperature
  u8g.setPrintPos(88, firstLineA);
  u8g.print(temperature);
  u8g.print("\xb0");
  u8g.print("C");

  // Firstline B // humidity
  u8g.setPrintPos(88, firstLineB);
  u8g.print(humidity);
  u8g.print(" %");
  //u8g.print(heat_index);
  //u8g.print("\xb0");
  //u8g.print("C");

  // Second line // date
  u8g.setPrintPos(0, secondLine);
  u8g.print(weekdays[weekday]);
  u8g.print("  ");

  //u8g.setPrintPos(22, secondLine);
  if (adj_select == 5 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.print(months[month-1]);
  u8g.setColorIndex(1);
  u8g.print(" ");

  //u8g.setPrintPos(44, secondLine);
  if (adj_select == 6 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.print(day);
  u8g.setColorIndex(1);

  //u8g.setPrintPos(50, secondLine);
  u8g.print(", ");
  if (adj_select == 7 && blink) {
    u8g.setColorIndex(0);
  }
  u8g.print(year);
  u8g.setColorIndex(1);

  if (hour_mode == 1) {
    if (a1_hour == 0) {
      dhour = 12; // Am
      hour_12 = 0;
    }
    else if (a1_hour > 12) {
      dhour = a1_hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      dhour = a1_hour; // Am
      hour_12 = 0;
    }
  }
  else {
    dhour = a1_hour;
  }

  // Third line // alarm 1
  u8g.setPrintPos(0, thirdLine);
  u8g.print("Alarm 1:");

  u8g.setPrintPos(44, thirdLine);
  if (adj_select == 8 && blink) {
    u8g.setColorIndex(0);
  }
  if (dhour < 10) {
    u8g.print("0");
    //u8g.setPrintPos(50, thirdLine);
  }
  u8g.print(dhour);
  u8g.setColorIndex(1);

  u8g.setPrintPos(57, thirdLine);
  u8g.print(":");

  u8g.setPrintPos(61, thirdLine);
  if (adj_select == 9 && blink) {
    u8g.setColorIndex(0);
  }
  if (a1_minute < 10) {
    u8g.print("0");
    //u8g.setPrintPos(67, thirdLine);
  }
  u8g.print(a1_minute);
  u8g.setColorIndex(1);

  //u8g.setPrintPos(75, thirdLine);
  u8g.print(" ");
  if (hour_mode == 1) {
    u8g.print(hour_12s[hour_12]);
  }
  else {
    u8g.print("Mi");
  }

  u8g.setPrintPos(110, thirdLine);
  if (adj_select == 10 && blink) {
    u8g.setColorIndex(0);
  }
  if (a1_duration > 0) {
    if (a1_duration < 10) {
      u8g.print("0");
    }
    u8g.print(a1_duration);
    u8g.print("s");
  }
  else {
    u8g.print("Off");
  }
  u8g.setColorIndex(1);

  if (hour_mode == 1) {
    if (a2_hour == 0) {
      dhour = 12; // Am
      hour_12 = 0;
    }
    else if (a2_hour > 12) {
      dhour = a2_hour - 12; // Pm
      hour_12 = 1;
    }
    else {
      dhour = a2_hour; // Am
      hour_12 = 0;
    }
  }
  else {
    dhour = a2_hour;
  }

  // Fourth line // alarm 2
  u8g.setPrintPos(0, fourthLine);
  u8g.print("Alarm 2:");

  u8g.setPrintPos(44, fourthLine);
  if (adj_select == 11 && blink) {
    u8g.setColorIndex(0);
  }
  if (dhour < 10) {
    u8g.print("0");
    //u8g.setPrintPos(50, fourthLine);
  }
  u8g.print(dhour);
  u8g.setColorIndex(1);

  u8g.setPrintPos(57, fourthLine);
  u8g.print(":");

  u8g.setPrintPos(61, fourthLine);
  if (adj_select == 12 && blink) {
    u8g.setColorIndex(0);
  }
  if (a2_minute < 10) {
    u8g.print("0");
    //u8g.setPrintPos(67, fourthLine);
  }
  u8g.print(a2_minute);
  u8g.setColorIndex(1);

  //u8g.setPrintPos(75, fourthLine);
  u8g.print(" ");
  if (hour_mode == 1) {
    u8g.print(hour_12s[hour_12]);
  }
  else {
    u8g.print("Mi");
  }

  u8g.setPrintPos(110, fourthLine);
  if (adj_select == 13 && blink) {
    u8g.setColorIndex(0);
  }
  if (a2_duration > 0) {
    if (a2_duration < 10) {
      u8g.print("0");
    }
    u8g.print(a2_duration);
    u8g.print("s");
  }
  else {
    u8g.print("Off");
  }
  u8g.setColorIndex(1);
} 

void setup()
{
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(13, OUTPUT); // piezo buzzer
  
  weekday = get_weekday(year, month, day);
}

void loop() {
  static bool dispblink = false;
  runEvery(500) {
    dispblink = !dispblink;
    u8g.firstPage();
    do {
      draw_page_1(dispblink);
    }
    while( u8g.nextPage() );
  }

  runEvery(1000) {    
    updateClock();
    read_dht11();
  }

  // Serial listen
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    decodeSerial(incomingByte);
  }
  // IR listen
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    decodeIR(&results);
    irrecv.resume(); // Receive the next value
  }
}

