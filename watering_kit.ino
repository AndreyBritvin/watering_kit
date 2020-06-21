#include <Wire.h>
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);    // I2C
#include "Wire.h"
#include "RTClib.h"
RTC_DS1307 RTC;


//unsigned long read_value_delay = 120000; //ms
int Mode = 1; //0=autoMode 1=3days of 15sec watering
int prevMode = 1;
//unsigned long timing = 0; //for millis
String from_MQTT="";

int nextDayOfTheWeek = 1;
bool isWateringToday = false;
int days_in_mode1 = 3;//days
int read_value_delay = 1;//hours
int nextHour = 1;

//for mode 0 watering
int time_relay1_mode0 = 15000;
int time_relay2_mode0 = 10000;
int time_relay3_mode0 = 10000;
int time_relay4_mode0 = 10000;

//for mode 1 watering
int time_relay1_mode1= 15000;
int time_relay2_mode1 = 10000;
int time_relay3_mode1 = 10000;
int time_relay4_mode1 = 10000;

//for manual watering
int time_relay1_man = 15000;
int time_relay2_man = 10000;
int time_relay3_man = 10000;
int time_relay4_man = 10000;



// set all moisture sensors PIN ID
int moisture1 = A0;
int moisture2 = A1;
int moisture3 = A2;
int moisture4 = A3;

// declare moisture values
int moisture1_value = 0 ;
int moisture2_value = 0;
int moisture3_value = 0;
int moisture4_value = 0;

// set water relays
int relay1 = 6;
int relay2 = 8;
int relay3 = 9;
int relay4 = 10;

// set water pump
int pump = 4;

// set button
int button = 12;

//pump state    1:open   0:close
int pump_state_flag = 0;

//relay1 state    1:open   0:close
int relay1_state_flag = 0;

//relay2 state   1:open   0:close
int relay2_state_flag = 0;

//relay3 state  1:open   0:close
int relay3_state_flag = 0;

//relay4 state   1:open   0:close
int relay4_state_flag = 0;

static unsigned long currentMillis_send = 0;
static unsigned long  Lasttime_send = 0;

char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat",};
unsigned long nowtime;
unsigned long endtime;
unsigned long nowtimeNext;
unsigned long nowtime1;
unsigned long endtime1;
unsigned long nowtimeNext1;
unsigned long nowtime2;
unsigned long endtime2;
unsigned long nowtimeNext2;
unsigned long nowtime3;
unsigned long endtime3;
unsigned long nowtimeNext3;


// good flower
unsigned char bitmap_good[] U8G_PROGMEM = {

  0x00, 0x42, 0x4C, 0x00, 0x00, 0xE6, 0x6E, 0x00, 0x00, 0xAE, 0x7B, 0x00, 0x00, 0x3A, 0x51, 0x00,
  0x00, 0x12, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x06, 0x40, 0x00, 0x00, 0x06, 0x40, 0x00,
  0x00, 0x04, 0x60, 0x00, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x08, 0x30, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0xE0, 0x0F, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0xC1, 0x00, 0x00, 0x0E, 0x61, 0x00,
  0x00, 0x1C, 0x79, 0x00, 0x00, 0x34, 0x29, 0x00, 0x00, 0x28, 0x35, 0x00, 0x00, 0x48, 0x17, 0x00,
  0x00, 0xD8, 0x1B, 0x00, 0x00, 0x90, 0x1B, 0x00, 0x00, 0xB0, 0x09, 0x00, 0x00, 0xA0, 0x05, 0x00,
  0x00, 0xE0, 0x07, 0x00, 0x00, 0xC0, 0x03, 0x00
};

// bad flower
unsigned char bitmap_bad[] U8G_PROGMEM = {
  0x00, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xE0, 0x0D, 0x00, 0x00, 0xA0, 0x0F, 0x00,
  0x00, 0x20, 0x69, 0x00, 0x00, 0x10, 0x78, 0x02, 0x00, 0x10, 0xC0, 0x03, 0x00, 0x10, 0xC0, 0x03,
  0x00, 0x10, 0x00, 0x01, 0x00, 0x10, 0x80, 0x00, 0x00, 0x10, 0xC0, 0x00, 0x00, 0x30, 0x60, 0x00,
  0x00, 0x60, 0x30, 0x00, 0x00, 0xC0, 0x1F, 0x00, 0x00, 0x60, 0x07, 0x00, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xC7, 0x1C, 0x00,
  0x80, 0x68, 0x66, 0x00, 0xC0, 0x33, 0x7B, 0x00, 0x40, 0xB6, 0x4D, 0x00, 0x00, 0xE8, 0x06, 0x00,
  0x00, 0xF0, 0x03, 0x00, 0x00, 0xE0, 0x00, 0x00
};

// Elecrow Logo
  static unsigned char bitmap_logo[] U8G_PROGMEM ={
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xE0,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x04,0xF8,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x08,0xFE,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x10,0x1F,0xE0,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xB0,0x07,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xE0,0x03,0x00,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xC0,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x80,0x01,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x60,0x23,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0xC7,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x9E,0x0F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x3C,0xFE,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x78,0xF8,0x7F,0xF0,0x9F,0x07,0xFE,0x83,0x0F,0xFF,0x00,0x77,0x3C,0x18,0x1C,
  0x70,0xF0,0xE1,0x3F,0xF1,0x9F,0x07,0xFE,0xE1,0x1F,0xFF,0xC3,0xF7,0x3C,0x38,0x0C,
  0x70,0xE0,0x87,0x8F,0xF1,0xC0,0x07,0x1E,0x70,0x3C,0xCF,0xE3,0xE1,0x7D,0x3C,0x0E,
  0x70,0xD0,0x1F,0xC0,0xF1,0xC0,0x03,0x1F,0x78,0x3C,0xCF,0xE3,0xE1,0x7D,0x3C,0x06,
  0xF0,0xB0,0xFF,0xF1,0xF0,0xC0,0x03,0x0F,0x78,0x3C,0xCF,0xF3,0xE0,0x7B,0x3E,0x06,
  0xF0,0x60,0xFF,0xFF,0xF0,0xC6,0x03,0xEF,0x3C,0x80,0xEF,0xF1,0xE0,0x7B,0x3E,0x03,
  0xF0,0xE1,0xFC,0xFF,0xF8,0xCF,0x03,0xFF,0x3C,0x80,0xFF,0xF0,0xE0,0x7B,0x7B,0x01,
  0xE0,0xC3,0xF9,0x7F,0x78,0xC0,0x03,0x0F,0x3C,0x80,0xF7,0xF1,0xE0,0xF9,0xF9,0x01,
  0xE0,0x83,0xE3,0x7F,0x78,0xE0,0x03,0x0F,0x3C,0xBC,0xE7,0xF1,0xE0,0xF9,0xF9,0x00,
  0xC0,0x0F,0x8F,0x3F,0x78,0xE0,0x81,0x0F,0x3C,0x9E,0xE7,0xF1,0xE0,0xF1,0xF8,0x00,
  0x80,0x3F,0x1E,0x00,0x78,0xE0,0x81,0x07,0x38,0x9E,0xE7,0xF1,0xF0,0xF0,0x78,0x00,
  0x80,0xFF,0xFF,0x00,0xF8,0xEF,0xBF,0xFF,0xF8,0xCF,0xE7,0xE1,0x7F,0x70,0x70,0x00,
  0x00,0xFF,0xFF,0x0F,0xF8,0xEF,0xBF,0xFF,0xE0,0xC3,0xE3,0x81,0x1F,0x70,0x30,0x00,
  0x00,0xFC,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xF8,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xE0,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };


static unsigned char bitmap_T[] U8G_PROGMEM = {
  0xF7, 0x01, 0x1D, 0x03, 0x0B, 0x02, 0x0C, 0x02, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x08, 0x02,
  0x18, 0x03, 0xF0, 0x01
};

static unsigned char bitmap_H[] U8G_PROGMEM = {
  0x00, 0x00, 0x80, 0x01, 0xC0, 0x03, 0xE0, 0x07, 0xF0, 0x0F, 0xF8, 0x1F, 0xF8, 0x1F, 0xFC, 0x3F,
  0xFC, 0x3F, 0xFE, 0x7F, 0xEE, 0x7F, 0xB3, 0xF7, 0xBB, 0xFB, 0xBB, 0xFD, 0xBB, 0xFD, 0xC7, 0xFE,
  0x7F, 0xC3, 0x3F, 0xDD, 0xBF, 0xFD, 0xDF, 0xDD, 0xEE, 0x5B, 0xFE, 0x7F, 0xFC, 0x3F, 0xF8, 0x1F,
  0xE0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void setup()
{
  //Serial1.begin(9600);
  draw_elecrow();
 delay(2000);
  Wire.begin();
  RTC.begin();
 Serial.begin(9600);
 Serial1.begin(9600);
  // declare relay as output
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  // declare pump as output
  pinMode(pump, OUTPUT);
  // declare switch as input
  pinMode(button, INPUT);
  DateTime now = RTC.now();
  nextDayOfTheWeek=now.dayOfTheWeek();
  nextHour = now.hour();
//  Serial.println(nextDayOfTheWeek);

  if(now.hour()>=10 and now.minute()>57)
  {
    nextDayOfTheWeek+=3;
  }
  
  //pinMode(ROTARY_ANGLE_SENSOR, INPUT);
  // water_flower();
}

void loop()
{
from_MQTT="";
  // read the value from the moisture sensors:
  
  while(Serial1.available())
  {
 // Serial.print(Serial.available());
    from_MQTT = from_MQTT+(char)Serial1.read();
    //Serial.print("qqq ");
    
   
  }
 
  if(from_MQTT!="")
  
  {//Serial.print("aaa"); 
  //Serial.println(from_MQTT);
 // Serial.println(from_MQTT);
  change_value(from_MQTT);
  }
  
/*  int button_state = digitalRead(button);
  if (button_state == 1)
  {
   // read_value();
    u8g.firstPage();
    do
    {
      drawTH();
      drawflower();
     
    } while ( u8g.nextPage() );
  }
  else
  {
    u8g.firstPage();
    do
    {
      drawtime();
      u8g.drawStr(8, 55 , "www.elecrow.com");
    } while (u8g.nextPage());
  }*/
  
    DateTime now = RTC.now();
    //Serial.print(millis());
    //Serial.print(" ");
    //Serial.print(timing);
    //Serial.print(" ");
    //Serial.println(millis()-timing>read_value_delay);
    if(nextHour==now.hour())
    {
      getNextHour();
      
    
   //  Serial.println("aaa");
     read_value(); 
     
     isWateringToday = false;
     water_flower();
     isWateringToday = true;
    }
  //  Serial1.println("test/test:data0|test/test2:data1");
    //delay(1000);
 // delay(read_value_delay*60*60*1000);//ms
  
  
  water_flower();
  delay(500);
}
void getNextHour()
{
  nextHour = nextHour + read_value_delay;
      if(nextHour == 24)
      {
        nextHour = 0;
      }
      else if(nextHour>24)
      {
        nextHour = nextHour%24;
      }
}
void getNextDayOfTheWeek()
{
  nextDayOfTheWeek = nextDayOfTheWeek+days_in_mode1;
  if(nextDayOfTheWeek>7)
  {
    nextDayOfTheWeek = nextDayOfTheWeek%7;
  }
}
void sendLog(int numRelay, int m)
{
  DateTime now = RTC.now();
  String toSend = "";
  if(m == 0){
  toSend+="home/watering/flower"+String(numRelay)+"/open/"+String(now.hour())+"."+String(now.minute())+"."+String(now.second());}
  else{
  toSend+="home/watering/flower"+String(numRelay)+"/close/"+String(now.hour())+"."+String(now.minute())+"."+String(now.second());}
  
  Serial1.println(toSend);
}

void change_value(String str)
{
 // Serial.print(str);
  String constant = getValue(str, ':', 0);
  String value = getValue(str, ':', 1);
 // Serial.print(constant);
 // Serial.println(value);
  if(constant == "mode")
  {
      prevMode = Mode;
      Mode = value.toInt();
      isWateringToday = false;
      
     // Serial.println(Mode);
  }
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){

    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";

}


//Set moisture value
void read_value()
{
/**************These is for resistor moisture sensor***********
 float value1 = analogRead(A0);
  moisture1_value = (value1 * 120) / 1023; delay(20);
  float value2 = analogRead(A1);
  moisture2_value = (value2 * 120) / 1023; delay(20);
  float value3 = analogRead(A2);
  moisture3_value = (value3 * 120) / 1023; delay(20);
  float value4 = analogRead(A3);
  moisture4_value = (value4 * 120) / 1023; delay(20);
 **********************************************************/
/************These is for capacity moisture sensor*********/
 float value1 = analogRead(A0);
  moisture1_value =map(value1,590,360,0,100); delay(20);
  if(moisture1_value<0){
    moisture1_value=0;
  }
  float value2 = analogRead(A1);
  moisture2_value =map(value2,600,360,0,100); delay(20);
  if(moisture2_value<0) {
    moisture2_value=0;
  }
  float value3 = analogRead(A2);
  moisture3_value =map(value3,600,360,0,100); delay(20);
  if(moisture3_value<0){
    moisture3_value=0;
  }
  float value4 = analogRead(A3);
  moisture4_value =map(value4,600,360,0,100); delay(20);
  if(moisture4_value<0) {
    moisture4_value=0;
  }
  String hum = "";
  hum+="home/watering/flower/1/humidity:"+String(moisture1_value)+"|";
  hum+="home/watering/flower/2/humidity:"+String(moisture2_value)+"|";
  hum+="home/watering/flower/3/humidity:"+String(moisture3_value)+"|";
  hum+="home/watering/flower/4/humidity:"+String(moisture4_value);
  //Serial.println(hum);
  Serial1.println(hum);
//  delay(500);
}

void water_flower()

{
  //Serial.println(Mode);
  DateTime now = RTC.now();
  
 // Serial.println(now.hour());
 if(Mode==0 and !isWateringToday){
  if(now.hour()<=7 or now.hour()>=21){
  }
  else{
  if (moisture1_value < 30)
  {
    digitalWrite(relay1, HIGH);
    sendLog(1, 0);
    relay1_state_flag = 1;
    delay(50);
    if (pump_state_flag == 0)
    {
      digitalWrite(pump, HIGH);
      sendLog(5, 0);
      pump_state_flag = 1;
      delay(50);
    }
    delay(time_relay1_mode0);
//  }
  //else if (moisture1_value > 55)
  //{
    digitalWrite(relay1, LOW);
    sendLog(1, 1);
    relay1_state_flag = 0;
    delay(50);
    if ((relay1_state_flag == 0) && (relay2_state_flag == 0) && (relay3_state_flag == 0) && (relay4_state_flag == 0))
    {
      digitalWrite(pump, LOW);
      sendLog(5, 1);
      pump_state_flag = 0;
      delay(50);
    }
  }

  if (moisture2_value < 30)
  {
    digitalWrite(relay2, HIGH);
    sendLog(2, 0);
    relay2_state_flag = 1;
    delay(50);
    if (pump_state_flag == 0)
    {
      digitalWrite(pump, HIGH);
      sendLog(5, 0);
      pump_state_flag = 1;
      delay(50);
    }
    delay(time_relay2_mode0);
  //}
  //else if (moisture2_value > 55)
  //{
    digitalWrite(relay2, LOW);
    sendLog(2, 1);
    relay2_state_flag = 0;
    delay(50);
    if ((relay1_state_flag == 0) && (relay2_state_flag == 0) && (relay3_state_flag == 0) && (relay4_state_flag == 0))
    {
      digitalWrite(pump, LOW);
      sendLog(5, 1);
      pump_state_flag = 0;
      delay(50);
    }
  }

  if (moisture3_value < 30)
  {
    digitalWrite(relay3, HIGH);
    sendLog(3, 0);
    relay3_state_flag = 1;
    delay(50);
    if (pump_state_flag == 0)
    {
      digitalWrite(pump, HIGH);
      sendLog(5, 0);
      pump_state_flag = 1;
      delay(50);
    }
    delay(time_relay3_mode0);
  //}
  //else if (moisture3_value > 55)
  //{
    digitalWrite(relay3, LOW);
    sendLog(3, 1);
    relay3_state_flag = 0;
    delay(50);
    if ((relay1_state_flag == 0) && (relay2_state_flag == 0) && (relay3_state_flag == 0) && (relay4_state_flag == 0))
    {
      digitalWrite(pump, LOW);
      sendLog(5, 1);
      pump_state_flag = 0;
      delay(50);
    }
  }

  if (moisture4_value < 30)
  {
    digitalWrite(relay4, HIGH);
    sendLog(4, 0);
    relay4_state_flag = 1;
    delay(50);
    if (pump_state_flag == 0)
    {
      digitalWrite(pump, HIGH);
      sendLog(5, 0);
      pump_state_flag = 1;
      delay(50);
    }
    delay(time_relay4_mode0);
  //}
  //else if (moisture4_value > 55)
  //{
    digitalWrite(relay4, LOW);
    sendLog(4, 1);
    relay4_state_flag = 0;
    delay(50);
    if ((relay1_state_flag == 0) && (relay2_state_flag == 0) && (relay3_state_flag == 0) && (relay4_state_flag == 0))
    {
      digitalWrite(pump, LOW);
      sendLog(5, 1);
      pump_state_flag = 0;
      delay(50);
    }
  }

}}
else if(Mode==1)
{
  if(nextDayOfTheWeek==now.dayOfTheWeek() and now.hour()==10 and !isWateringToday)
  {
  digitalWrite(relay1, HIGH);
  delay(50);
  digitalWrite(pump, HIGH);
  delay(time_relay1_mode1);
  digitalWrite(relay1,LOW);
  
  digitalWrite(relay2, HIGH);
  delay(50);
  
  delay(time_relay2_mode1);
  digitalWrite(relay2,LOW);
  
  digitalWrite(relay3, HIGH);
  delay(50);
  
  delay(time_relay3_mode1);
  digitalWrite(relay3,LOW);
  
  digitalWrite(relay4, HIGH);
  delay(50);
  
  delay(time_relay4_mode1);
  digitalWrite(relay4,LOW);
  digitalWrite(pump, LOW);
  
  
  isWateringToday = true;
  }
  else if(now.hour()!=10)
  {
    isWateringToday = false;
  }
  //delay(days_in_mode1*24*60*60*1000);
}
  else if(Mode==2)
  {
    digitalWrite(relay1, HIGH);
  delay(50);
  digitalWrite(pump, HIGH);
  delay(time_relay1_man);
  digitalWrite(relay1,LOW);
  
  digitalWrite(relay2, HIGH);
  delay(50);
  
  delay(time_relay2_man);
  digitalWrite(relay2,LOW);
  
  digitalWrite(relay3, HIGH);
  delay(50);
  
  delay(time_relay3_man);
  digitalWrite(relay3,LOW);
  
  digitalWrite(relay4, HIGH);
  delay(50);
  getNextDayOfTheWeek();
  delay(time_relay4_man);
  digitalWrite(relay4,LOW);
  digitalWrite(pump, LOW);
  Mode = prevMode;//you must change it from mqtt to 2 that open manual mode(mode2)
  }
  else if(Mode==3)
  {
    digitalWrite(relay1, HIGH);
    delay(50);
    digitalWrite(pump, HIGH);
  }
}



  void draw_elecrow(void){

  u8g.setFont(u8g_font_gdr9r);
  u8g.drawStr(8,55 , "www.elecrow.com");
  u8g.drawXBMP(0, 5,128,32, bitmap_logo);
  }


void drawtime(void)
{
  int x = 5;
  float i = 25.00;
  float j = 54;
  DateTime now = RTC.now();
  //Serial.print(now.year(), DEC);
  if (! RTC.isrunning())
  {
    u8g.setFont(u8g_font_6x10);
    u8g.setPrintPos(5, 20);
    u8g.print("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  else
  {
    u8g.setFont(u8g_font_7x13);
    u8g.setPrintPos(x, 11);
    u8g.print(now.year(), DEC);
    u8g.setPrintPos(x + 80, 11);
    u8g.print(daysOfTheWeek[now.dayOfTheWeek()]);
    u8g.setPrintPos(x + 28, 11);
    u8g.print("/");
    u8g.setPrintPos(x + 33, 11);
    u8g.print(now.month(), DEC);
    if (now.month() < 10)
      x -= 7;
    u8g.setPrintPos(x + 47, 11);
    u8g.print("/");
    u8g.setPrintPos(x + 53, 11);
    u8g.print(now.day(), DEC);
    u8g.setFont(u8g_font_8x13);
    int x = 35;
    u8g.setPrintPos(x, 33);
    u8g.print(now.hour(), DEC);
    if (now.hour() < 10)
      x -= 7;
    u8g.setPrintPos(x + 15, 33);
    u8g.print(":");
    u8g.setPrintPos(x + 21, 33);
    u8g.print(now.minute(), DEC);
    if (now.minute() < 10)
      x -= 7;
    u8g.setPrintPos(x + 36, 33);
    u8g.print(":");
    u8g.setPrintPos(x + 42, 33);
    u8g.print(now.second(), DEC);
  }
}

void drawLogo(uint8_t d)
{
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(8 + d, 30 + d, "E");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(30 + d, 30 + d, "l");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(40 + d, 30 + d, "e");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(55 + d, 30 + d, "c");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(70 + d, 30 + d, "r");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(85 + d, 30 + d, "o");
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(100 + d, 30 + d, "w");
}


//Style the flowers     bitmap_bad: bad flowers     bitmap_good:good  flowers
void drawflower(void)
{
  if (moisture1_value < 30)
  {
    u8g.drawXBMP(0, 0, 32, 30, bitmap_bad);
  }
  else
  {
    u8g.drawXBMP(0, 0, 32, 30, bitmap_good);
  }
  if (moisture2_value < 30)
  {
    u8g.drawXBMP(32, 0, 32, 30, bitmap_bad);
  }
  else
  {
    u8g.drawXBMP(32, 0, 32, 30, bitmap_good);
  }
  if (moisture3_value < 30)
  {
    u8g.drawXBMP(64, 0, 32, 30, bitmap_bad);
  }
  else
  {
    u8g.drawXBMP(64, 0, 32, 30, bitmap_good);
  }
  if (moisture4_value < 30)
  {
    u8g.drawXBMP(96, 0, 32, 30, bitmap_bad);
  }
  else
  {
    u8g.drawXBMP(96, 0, 32, 30, bitmap_good);
  }

}


void drawTH(void)
{
  int A = 0;
  int B = 0;
  int C = 64;
  int D = 96;
  char moisture1_value_temp[5] = {0};
  char moisture2_value_temp[5] = {0};
  char moisture3_value_temp[5] = {0};
  char moisture4_value_temp[5] = {0};
 // read_value();
  itoa(moisture1_value, moisture1_value_temp, 10);
  itoa(moisture2_value, moisture2_value_temp, 10);
  itoa(moisture3_value, moisture3_value_temp, 10);
  itoa(moisture4_value, moisture4_value_temp, 10);
  u8g.setFont(u8g_font_7x14);
  u8g.setPrintPos(9, 60);
  u8g.print("A0");
  if (moisture1_value < 10)
  {
    //u8g.setPrintPos(A + 14, 45 );
    u8g.drawStr(A + 14, 45, moisture1_value_temp);
    delay(20);
    u8g.drawStr(A + 14, 45, moisture1_value_temp);
    
  }
  else if (moisture1_value < 100)
  {
    //u8g.setPrintPos(A + 7, 45);
    u8g.drawStr(A + 7, 45, moisture1_value_temp);
    delay(20);
    u8g.drawStr(A + 7, 45, moisture1_value_temp);
   
  }
  else
  {
    //u8g.setPrintPos(A + 2, 45 );
    moisture1_value = 100;
    itoa(moisture1_value, moisture1_value_temp, 10);
    u8g.drawStr(A + 2, 45, moisture1_value_temp);
  }
  //u8g.print(moisture1_value);
  u8g.setPrintPos(A + 23, 45 );
  u8g.print("%");
  u8g.setPrintPos(41, 60 );
  u8g.print("A1");
  if (moisture2_value < 10)
  {
    //u8g.setPrintPos(B + 46, 45 );
    u8g.drawStr(B + 46, 45, moisture2_value_temp); 
    delay(20);
    u8g.drawStr(B + 46, 45, moisture2_value_temp); 
  }
  else if (moisture2_value < 100)
  {
    //u8g.setPrintPos(B + 39, 45);
    u8g.drawStr(B + 39, 45, moisture2_value_temp);
    delay(20);
    u8g.drawStr(B + 39, 45, moisture2_value_temp);
  }
  else
  {
    //u8g.setPrintPos(B + 32, 45);
    moisture2_value = 100;
    itoa(moisture2_value, moisture2_value_temp, 10);
    u8g.drawStr(B + 32, 45, moisture2_value_temp);
  }
  // u8g.print(moisture2_value);
  u8g.setPrintPos(B + 54, 45);
  u8g.print("%");
  u8g.setPrintPos(73, 60);
  u8g.print("A2");
  if (moisture3_value < 10)
  {
    //u8g.setPrintPos(C + 14, 45 );
    u8g.drawStr(C + 14, 45, moisture3_value_temp);
    delay(20);
    u8g.drawStr(C + 14, 45, moisture3_value_temp);
    
  }
  else if (moisture3_value < 100)
  {
    // u8g.setPrintPos(C + 7, 45);
   u8g.drawStr(C + 7, 45, moisture3_value_temp);
    delay(20);
    u8g.drawStr(C + 7, 45, moisture3_value_temp);
    
  }
  else
  {
    // u8g.setPrintPos(C + 2, 45);
    moisture3_value = 100;
    itoa(moisture3_value, moisture3_value_temp, 10);
    u8g.drawStr(C + 2, 45, moisture3_value_temp);
  }
  //u8g.print(moisture3_value);
  u8g.setPrintPos(C + 23, 45);
  u8g.print("%");
  u8g.setPrintPos(105, 60);
  u8g.print("A3");
  if (moisture4_value < 10)
  {
    //u8g.setPrintPos(D + 14, 45 );
    u8g.drawStr(D + 14, 45, moisture4_value_temp);
    delay(20);
    u8g.drawStr(D + 14, 45, moisture4_value_temp);
   
  }
  else if (moisture4_value < 100)
  {
    // u8g.setPrintPos(D + 7, 45);
    u8g.drawStr(D + 7, 45, moisture4_value_temp);
    delay(20);
    u8g.drawStr(D + 7, 45, moisture4_value_temp);
  
  }
  else
  {
    //u8g.setPrintPos(D + 2, 45);
    moisture4_value = 100;
    itoa(moisture4_value, moisture4_value_temp, 10);
    u8g.drawStr(D + 2, 45, moisture4_value_temp);
  }
  //u8g.print(moisture4_value);
  u8g.setPrintPos(D + 23, 45);
  u8g.print("%");
}
