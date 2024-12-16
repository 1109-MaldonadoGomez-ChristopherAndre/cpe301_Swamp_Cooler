#include <LiquidCrystal.h>
#include <DHT11.h>
#include <Stepper.h>
#include <RTClib.h>
#include <string.h>
#include <Wire.h>
//states
#define DISABLED 0
#define IDLE 1
#define RUNNING 2
#define ERROR 3
int previousState;
//real time clock
RTC_DS1307 rtc;
//Serial io
 #define RDA 0x80
 #define TBE 0x20  
 volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
 volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
 volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
 volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
 volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
  //ports
volatile unsigned char *PIN_B = (unsigned char *)0x23;
volatile unsigned char *DDR_B = (unsigned char *)0x24;
volatile unsigned char *PORT_B = (unsigned char *)0x25;

volatile unsigned char *PIN_E = (unsigned char *)0x2C;
volatile unsigned char *DDR_E = (unsigned char *)0x2D;
volatile unsigned char *PORT_E = (unsigned char *)0x2E;
//interupts
#define ONBUTTON 2 // port b4
bool isOn=false;
//stepper motor
const int stepsPerRevolution = 2038;
const int stepSize = 340;
void setup() {
  U0init(9600);
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //set port b to output pins
  *DDR_B |= 0x01;
  *DDR_B |= 0x01 << 1;
  *DDR_B |= 0x01 << 2;
  *DDR_B |= 0x01 << 3;

  *DDR_E &= ~(0x01 << 1);
  previousState=DISABLED;
  attachInterrupt(digitalPinToInterrupt(ONBUTTON), toggleIsOn, RISING);
}

void loop() {
 //check what state we should be in and then use switch case to run state
  int state = DISABLED;
  if(isOn){
    state = IDLE;
  }
  setLEDs(state);
  switch(state){
    case 1:
    displayTimeUpdate();
       break;
  }

}
void displayTimeUpdate(){
  char date [50];
  DateTime now = rtc.now();
  now.timestamp().toCharArray(date, 50);
  UARTPrint(date);
  U0putchar('\n');

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
void UARTPrint(unsigned char a[]){
  for(int i =0; a[i] != '\0'; i++){
    U0putchar(a[i]);
  }
}

void write_pb(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *PORT_B &= ~(0x01 << pin_num);
  }
  else
  {
    *PORT_B |= 0x01 << pin_num;
  }
}
//isr
void toggleIsOn(){
  if(isOn==false){
    isOn=true;
  }else{
    isOn=false;
  }
}
void setLEDs(int num){
  switch(num){
    case 0:
      write_pb(DISABLED, 1);
      write_pb(IDLE, 0);
      write_pb(ERROR, 0);
      write_pb(RUNNING, 0);
      break;
    case IDLE:
      write_pb(DISABLED, 0);
      write_pb(IDLE, 1);
      write_pb(ERROR, 0);
      write_pb(RUNNING, 0);
      break;
    case ERROR:
      write_pb(DISABLED, 0);
      write_pb(IDLE, 0);
      write_pb(ERROR, 1);
      write_pb(RUNNING, 0);
      break;
    case RUNNING:
      write_pb(DISABLED, 0);
      write_pb(IDLE, 0);
      write_pb(ERROR, 0);
      write_pb(RUNNING, 1);
      break;
  }
}
