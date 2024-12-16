#include <LiquidCrystal.h>
#include <DHT11.h>
#include <Stepper.h>
#include <RTClib.h>
#include <string.h>
//real time clock
RTC_DS3231 rtc;
char daysOfTheWeek [7][12] = {"Sunday", "Monday", "Tuesday", "Wenesday", "Thursday", "Friday", "Saturday"};
//Serial io
 #define RDA 0x80
 #define TBE 0x20  
 volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
 volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
 volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
 volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
 volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
//stepper motor
const int stepsPerRevolution = 2038;
const int stepSize = 340;
void setup() {
  U0init(9600); //replace
  rtc.begin();

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
//sfa
void loop() {
 displayTimeUpdate();

}
void displayTimeUpdate(){
  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);

  //Serial.println('\n');
 U0putchar('\n');
  U0putchar('\n');
  delay(3000);

}

//Serial io
void U0init(unsigned long U0baud){
 unsigned long FCPU = 16000000; //16mhz
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

