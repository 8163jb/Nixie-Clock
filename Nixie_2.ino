/* ------------------------------------------------------------------- 
  Nixie Tube Clock project: - 6 IN-12 Nixie tubes
                             - 6 74141 Driver chips
                             - 3 74HC595 Shift Registers
                             - 1 Arduino on the board
  
  Released into the public domain under the open-source GNU license.
  https://www.gnu.org/copyleft/gpl.html
  Summary of the above statement in real words that non-lawyers can
  understand: Take my code, project, ideas, design files etc... and
  make them even more awesome! Do whatever the hell you like with
  them! 
 
  Blog about this project:
 
  -------------------------------------------------------------------- */
  
#include <SoftI2C.h>       //Include all the relevent libraries
#include <DS3232RTC.h>
#include <avr/pgmspace.h>
#include <string.h>
#define data 2    //Define the shift register interface pins
#define clock 3
#define latch 4
#define bottom 5
#define middle 6
#define top 7

SoftI2C i2c(A4, A5);  //Initialize I2C line 
DS3232RTC rtc(i2c);   //Define rtc as an object for the DS3232 (with communication over I2C)

byte fNumbers[11] = { //Binary for the numbers, when given to the first shift register in each pair - 11th number is a blank
  B00000000,
  B00010000,
  B00100000,
  B00110000,
  B01000000,
  B01010000,
  B01100000,
  B01110000,
  B10000000,
  B10010000,
  B11110000 };
  
byte sNumbers[11] = { //Binary for the numbers, when given to the latter shift register in each pair - 11th number is a blank
  B00000000,
  B00000001,
  B00000010,
  B00000011,
  B00000100,
  B00000101,
  B00000110,
  B00000111,
  B00001000,
  B00001001,
  B00001111 };

byte dNumbers[3] = { //Numbers to display - temporary storage for whatever numbers we are currently displaying, one byte per shift register
  B00000000,
  B00000000,
  B00000000 };

void setup() 
{
  pinMode(data, 1);  //Set up the data, clock, and latch pins as outputs
  pinMode(clock, 1);
  pinMode(latch, 1);
  pinMode(top, 0);
  pinMode(middle, 0);
  pinMode(bottom, 0);
  digitalWrite(top, 1);
  digitalWrite(middle, 1);
  digitalWrite(bottom, 1);
}

void loop()
{
  RTCTime time;                 //Create object time - which will hold the data from the DS3232
  rtc.readTime(&time);          //Read the time from the DS3232 into the object 'time'
  int a = (time.hour / 10),     //Set the first segment to the tens unit of the hour variable eg. for 21 it would be 21/10 = 2
      b = (time.hour % 10),     //Set the second segment to the remainder when dividing the hour by 10 eg. for 21 it would be 21/10 = 2 remainder 1, so it would be 1
      c = (time.minute / 10),   //Above but for minutes
      d = (time.minute % 10),
      e = (time.second / 10),   //Above but for seconds
      f = (time.second % 10);   
  displayNumber(a, b, c, d, e, f);  //Display the numbers (custom subroutine)
  delay(100);                     //Delay 1/10th of a second
  if(digitalRead(middle) == LOW)
  {
    while(digitalRead(middle) == LOW);
    delay(100);
    setTime();
  }
}

void displayNumber(int a, int b, int c, int d, int e, int f) //Used to display the numbers
{
  dNumbers[0] = (sNumbers[a] | fNumbers[b]); //Creates on 8 bit binary number of the first two digits - for the first shift register
  dNumbers[1] = (sNumbers[c] | fNumbers[d]); //Above - but with the third and fourth digits - for the second shift register
  dNumbers[2] = (sNumbers[e] | fNumbers[f]); //Above - but with the fifth and sixth digits - for the third shift register
  digitalWrite(latch, 0);                           //Hold the latch low while we're shifting out data
  for(int i = 3; i >= 0; i--)                       //Shift everything out in reverse, because that's how they're daisychained/hooked up to the nixie drivers
  {
    shiftOut(data, clock, MSBFIRST, dNumbers[i]);
  }
  digitalWrite(latch, 1); //Pull the latch high again
}

void setTime()
{  
  int timeToSet[3] = {0, 0, 0}; 
  for(int j = 0; j < 3; j++)
  {
    timeToSet[j] = 0;
    while(digitalRead(middle) == HIGH)
    {
      if(digitalRead(top) == LOW)
      {
        while(digitalRead(top) == LOW);
        delay(100);
        timeToSet[j]++;
        if ((j == 0) && (timeToSet[j] > 23)) timeToSet[j] = 0;
        if (timeToSet[j] > 59) timeToSet[j] = 0;
      }
      if(digitalRead(bottom) == LOW)
      {
        while(digitalRead(bottom) == LOW);
        delay(100);
        timeToSet[j]--;
        if ((j == 0) && (timeToSet[j] < 0)) timeToSet[j] = 23;
        if (timeToSet[j] < 0) timeToSet[j] = 59;
      }
      switch (j) {
        case 0: 
          displayNumber(timeToSet[0] / 10, timeToSet[0] % 10, 10, 10, 10, 10);
          break;
        case 1:
          displayNumber(timeToSet[0] / 10, timeToSet[0] % 10, timeToSet[1] / 10, timeToSet[1] % 10, 10, 10);
          break;
        case 2:
          displayNumber(timeToSet[0] / 10, timeToSet[0] % 10, timeToSet[1] / 10, timeToSet[1] % 10, timeToSet[j] / 10, timeToSet[j] % 10);
          break;
        default:
          break;
      }
    }
    while(digitalRead(middle) == LOW);
    delay(100);
  }
  RTCTime time;
  time.hour = timeToSet[0];
  time.minute = timeToSet[1];
  time.second = timeToSet[2];
  rtc.writeTime(&time);
}
