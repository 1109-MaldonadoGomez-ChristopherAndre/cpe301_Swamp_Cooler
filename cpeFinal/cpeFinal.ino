//Authors: Christopher Maldonado
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
int state = DISABLED;
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

volatile unsigned char *PIN_D = (unsigned char *)0x29;
volatile unsigned char *DDR_D = (unsigned char *)0x2A;
volatile unsigned char *PORT_D = (unsigned char *)0x2B;

volatile unsigned char *PIN_E = (unsigned char *)0x2C;
volatile unsigned char *DDR_E = (unsigned char *)0x2D;
volatile unsigned char *PORT_E = (unsigned char *)0x2E;
//adc
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
//interupts
#define ONBUTTON 2 // port b4
#define RESET 18
#define OFFBUTTON 19 
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
  // set interupt pins to output 
  *DDR_E &= ~(0x01 << 4);//pin 2
  *DDR_D &= ~(0x01 << 2);//pin 19
  *DDR_D &= ~(0x01 << 3);//pin 18
  previousState=DISABLED;
  // LCD pins <--> Arduino pins
  const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
  int right=0,up=0;
  int dir1=0,dir2=0;
  attachInterrupt(digitalPinToInterrupt(ONBUTTON), toggleIsOn, RISING);
  attachInterrupt(digitalPinToInterrupt(OFFBUTTON), toggleIsOff, RISING);
  attachInterrupt(digitalPinToInterrupt(RESET), reset, RISING);
}

void loop() {
 //check what state we should be in and then use switch case to run state
  //set the state
  if(isOn){
    state = IDLE;
  }else{
    state = DISABLED;
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
  isOn=true;
}void toggleIsOff(){
  isOn=false;
}
void reset(){
  if(isOn){
    state=IDLE;
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
void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
