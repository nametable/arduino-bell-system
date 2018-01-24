//*************************
//* Alarm Software - Revision 4
//* by Logan Bateman - Last updated 01-24-2018
//* Tested on an Arduino Uno compatible with a relay module and a DS1307 RTC
//*************************
//* RTC library from https://github.com/adafruit/RTClib/
//* DigitalTube library (7seg display) Frankie.Chu at Seeed Studio.
//*
//* Added functionality to have alarms for specific days
//* Changed reset to happen every morning between 12AM and 12:01AM
//* Added countdown functionality for next alarm
//*************************

#include <Wire.h>
#include <RTClib.h>
#include <TM1637.h>
#include "resources.h"

enum DOW //day of week
{
  SUNDAY=1,
  MONDAY=2,
  TUESDAY=4,
  WEDNESDAY=8,
  THURSDAY=16,
  FRIDAY=32,
  SATURDAY=64,
  EVERYDAY=127
};
RTC_DS1307 RTC;


//Define number of alarms -- NOTE: I would not recommend more than 192 alarms total
//                                 even 192 causes a low memory available
//                                 warning on a Uno with 2048 bytes of RAM
#define NUM_OF_ALARMS     20
#define ALARM_PIN         2
#define CLK 6         //pins definitions for TM1637 and can be changed to other ports        
#define DIO 7
TM1637 tm1637(CLK,DIO);
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

//array for all of the times of the alarms - with day of week now (NEW)
// NOTE: please put them in order from earliest to latest (time)
//       if you don't, prepare for possible unexpected results
//       like maybe alarms not triggering or never stopping
const TimeAndDOW tdowAlarms[NUM_OF_ALARMS]=
{
	TimeAndDOW(7,57,0,EVERYDAY-SATURDAY-SUNDAY,102), // 3 min before
	TimeAndDOW(8,0,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 1st
	TimeAndDOW(8,50,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 1st
	TimeAndDOW(8,53,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 2nd
	TimeAndDOW(9,38,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 2nd
	TimeAndDOW(9,41,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 3rd
	TimeAndDOW(10,31,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 3rd
	TimeAndDOW(10,34,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 4th
	TimeAndDOW(11,14,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 4th/Lunch
	TimeAndDOW(11,39,0,TUESDAY+THURSDAY,102), // Chapel Start
	TimeAndDOW(12,3,0,EVERYDAY-SATURDAY-SUNDAY,102), // Lunch/Chapel End
	TimeAndDOW(12,6,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 5th
	TimeAndDOW(12,56,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 5th
	TimeAndDOW(12,59,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 6th
	TimeAndDOW(13,39,0,EVERYDAY-SATURDAY-SUNDAY,102), // End 6th
	TimeAndDOW(13,42,0,EVERYDAY-SATURDAY-SUNDAY,102), // Start 7th
	TimeAndDOW(14,15,0,FRIDAY,1), // Fri/Class End
	TimeAndDOW(14,32,0,EVERYDAY-FRIDAY-SATURDAY-SUNDAY,102), // End 7th
	TimeAndDOW(14,35,0,EVERYDAY-FRIDAY-SATURDAY-SUNDAY,102), // Start 8th
	TimeAndDOW(15,15,0,EVERYDAY-FRIDAY-SATURDAY-SUNDAY,1), // End School
};

//array for storing info about the current
bool bAlarmed[NUM_OF_ALARMS]= { 0 };
bool all_done=false;
int iNextAlarm=-1;

void setup() {
    //generate a random seed- from the value of A0
    randomSeed(analogRead(0));
    
    //the code below sets up pins 16 and 17 to act as 5v and GND to power the rtc
    pinMode(16, OUTPUT);
    pinMode(17, OUTPUT);
    digitalWrite(17, HIGH);
    digitalWrite(16, LOW);

    pinMode(13, OUTPUT); //sets 13 for testing
    pinMode(ALARM_PIN, OUTPUT); //pin to controls the relay
    
    //both pins are set to LOW to have the relay and light off
    digitalWrite(ALARM_PIN, HIGH);
    digitalWrite(13, LOW);

    //enable led 7 seg display
    tm1637.init();
    tm1637.set(0); //BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    tm1637.display(0,random(1,10));
    tm1637.display(1,random(1,10));
    tm1637.display(2,0);
    tm1637.display(3,4);
    
    //
    Serial.begin(57600);  //start serial communication
    Wire.begin();
    RTC.begin();  //init real time clock
    //RTC.adjust(DateTime(__DATE__, __TIME__)); //set time to computer's time
    //RTC.adjust(DateTime("Jan 24 2018", "12:28:00")); //set custom time with computer date
    if (! RTC.isrunning()) //check to see if the clock is working
    {
        Serial.println("RTC is NOT running!");
        tm1637.display(0,4);
        tm1637.display(1,0);
        tm1637.display(2,9);
        tm1637.display(3,9);
        abort();  //kill the program if the clock isn't working
    }
    delay(1000);
    Serial.println("Started !!!ver4");
    Serial.print("Num of alarms:");
    Serial.print(NUM_OF_ALARMS, DEC);
    tm1637.point(POINT_ON);    //Enable colon
    DateTime now = RTC.now();
    // the below code within the for loop will check for alarms that
    // have already passed and set bAlarmed[alarm#]=true so that they wont't alarm
    // again until being reset at midnight - or any time before 1 AM but after 12 AM
    for (int i=0; i<NUM_OF_ALARMS; i++)
    {
      if (now.hour()>tdowAlarms[i].hour())  //if the current hour is greater
      {
        bAlarmed[i]=true;
      }
      else if (now.hour()==tdowAlarms[i].hour()) // if the current hour is the same
      {
        if (now.minute()>tdowAlarms[i].minute()) // if the current minute is greater
        {
          bAlarmed[i]=true;
        }
        else if (now.minute()==tdowAlarms[i].minute()) //if the current minute is the same
        {
          if (now.second()>tdowAlarms[i].second()) //if the current second is greater
          {
            bAlarmed[i]=true;
          }
          else if (now.second()==tdowAlarms[i].second()) //if the second is the same there
                                                         //there should be an alarm
                                                         // unlikely
          {
            buzz(0,3000);
          }
        }
      }
      Serial.print(i, DEC);
      Serial.print(" ");
      Serial.print(bAlarmed[i], DEC);
      Serial.println();
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000); // poll for the time about every second
               // arduino timing - good enough since
               // that means alarm is always within one second of accuracy
  DateTime now = RTC.now(); //gets the current time
  //this is just info for knowing the current rtc time and day
  Serial.print(now.dayOfTheWeek(), DEC);
  Serial.print("d");
  Serial.print(DOWenum(now.dayOfTheWeek()), DEC);
  Serial.print("h");
  Serial.print(now.hour(), DEC);
  Serial.print("m");
  Serial.print(now.minute(), DEC);
  Serial.print("s");
  Serial.println(now.second(), DEC);
  //
  all_done=bAlarmed[0];
  //this loop loops through all of the alarms
  for (int i=0; i<NUM_OF_ALARMS; i++)
  {
    all_done=bAlarmed[i] && all_done; //compare all_done to alarm i to see if it is done
    //it will only continue if the alarm is set for today
    if (DOWenum(now.dayOfTheWeek()) & tdowAlarms[i].day_of_week())
    {
      //it will now only continue if the alarm has not already been alarmed today
      //this is done because an alarm is triggered after the specified exact time
      //and we don't want it to go continuously since it will always be after the
      //specified time until midnight the next day
      //Serial.print(i, DEC);
      if (bAlarmed[i]==false)
      {
        if (iNextAlarm==-1)
        {
          iNextAlarm=i;
          display_countdown(iNextAlarm, now);
        }
        else
        {
            //do nothing here yet
        }
        //Serial.println("FALSE");
        if (now.hour()>=tdowAlarms[i].hour()) //check if in or after specified hour
        {
          if (now.minute()>=tdowAlarms[i].minute()) //check if in or after specified minute
          {
            if (now.second()>=tdowAlarms[i].second()) //check if in or after specified second
            {
              Serial.print(i+1,DEC); //print the # of the alarm
              buzz(tdowAlarms[i].effect(),3000);          //buzz for 3000 miliseconds
              bAlarmed[i]=true;      //set this alarm as having been alarmed
            }
          }
        }
      }
      else //this alarm is already done
      {
        //iNextAlarm=-1;
        //Serial.println("TRUE");
        if((i==NUM_OF_ALARMS-1) && all_done) //End of the day -- done
        {
          display_random(now);
        }
      }
  
      if ((now.hour()<1) && (now.minute()<1) && (bAlarmed[0]==true)) //check if after midnight but before one
      {                                          //and if the first alarm has been triggered
        reset_alarms();all_done=false;           //this will prevent it from resetting constantly
      }                                          //during the hours of 12AM to 1AM
                                                                                  
    }
    else //if it's not the right day I mine as well say that it has
    {    //already been alarmed
      bAlarmed[i]=true;
      if((i==NUM_OF_ALARMS-1) && all_done) //End of the day -- done
      {
        display_random(now);
      }
    }
    //Serial.println(i,DEC);
    if (i==NUM_OF_ALARMS-1)
    {
      iNextAlarm=-1;
      //Serial.println("reset disp");
    }
  }
}

void buzz(char byEffect, int iTime)
{
  //this code causes the relay to be on for a variable time etc

  //
  Serial.println(" buzz!!");
  
  switch (byEffect)
  {
    case 0:                           //this is just the standard case - not sure what I will add next
      digitalWrite(13, HIGH);         //just pin 13 to see the led on the arduino
      digitalWrite(ALARM_PIN, LOW);   //this relay is on when output is low
      delay(iTime);   //this is the delay keeping the relay on
      digitalWrite(13, LOW);
      digitalWrite(ALARM_PIN, HIGH);
      break;
    case 1:
      digitalWrite(ALARM_PIN, LOW);
      delay(1000);
      digitalWrite(ALARM_PIN, HIGH);
      delay(250);
      digitalWrite(ALARM_PIN, LOW);
      delay(500);
      digitalWrite(ALARM_PIN, HIGH);
      delay(250);
      digitalWrite(ALARM_PIN, LOW);
      delay(500);
      digitalWrite(ALARM_PIN, HIGH);
      delay(250);
      digitalWrite(ALARM_PIN, LOW);
      delay(500);
      digitalWrite(ALARM_PIN, HIGH);
      break;
    case 2:
      digitalWrite(ALARM_PIN, LOW);
      delay(250);
      digitalWrite(ALARM_PIN, HIGH);
      delay(500);
      digitalWrite(ALARM_PIN, LOW);
      delay(250);
      digitalWrite(ALARM_PIN, HIGH);
      delay(500);
      digitalWrite(ALARM_PIN, LOW);
      delay(500);
      digitalWrite(ALARM_PIN, HIGH);
      break;
    case 100 ... 127:                 //this will specify seconds to be on ex 101 is one second while 127 is 27 seconds
      digitalWrite(13, HIGH);         //just pin 13 to see the led on the arduino
      digitalWrite(ALARM_PIN, LOW);   
      delay((byEffect-100)*1000);   
      digitalWrite(13, LOW);
      digitalWrite(ALARM_PIN, HIGH);      
      break;
    default:
      break;
  }
  Serial.println("done buzzing!!");   //once its done - let someone know
}

void reset_alarms()   //this function loops through the bAlarmed array
{                     //and resets each boolean to false for a new day
  for (int i=0; i<NUM_OF_ALARMS; i++)
  {
    bAlarmed[i]=false;
  }
  Serial.println("ALARMS RESET!!");
}

//this function uses an led to print the remaining time till the next alarm
//the difference is found between next alarm and current time
//carrying is included in case an hour or minute needs to be borrowed
void display_countdown(byte byAlarm, DateTime now)
{
  int h_diff = tdowAlarms[byAlarm].hour() - now.hour();
  int m_diff;
  int s_diff;
  if (now.minute()>tdowAlarms[byAlarm].minute())
  {
    h_diff--;
    m_diff = tdowAlarms[byAlarm].minute() - now.minute()+60;
  }
  else
  {
    m_diff = tdowAlarms[byAlarm].minute() - now.minute();
  }
  if (now.second()>tdowAlarms[byAlarm].second())
  {
    m_diff--;
    s_diff = tdowAlarms[byAlarm].second() - now.second()+60;
  }
  else
  {
    s_diff = tdowAlarms[byAlarm].second() - now.second();
  }
  Serial.print("TIME LEFT h");
  Serial.print(h_diff, DEC);
  Serial.print("m");
  Serial.print(m_diff, DEC);
  Serial.print("s");
  Serial.println(s_diff, DEC);
  if (h_diff*60+m_diff<=99)   //if the remaining time is less than or equal to 99 min
  {                           //time displayed min:seconds
    m_diff+=h_diff*60;
    TimeDisp[0] = m_diff / 10;
    TimeDisp[1] = m_diff % 10;
    TimeDisp[2] = s_diff / 10;
    TimeDisp[3] = s_diff % 10;
    tm1637.display(TimeDisp);
  }
  else                        //if the remaining time is over 99 min
  {                           //remaining seconds displayed in hexadecimal seconds
    m_diff+=h_diff*60;
    s_diff+=m_diff*60;
    //Serial.println(s_diff,DEC);
    TimeDisp[0] = s_diff/4096;
    TimeDisp[1] = (s_diff%4096)/256;
    TimeDisp[2] = ((s_diff%4096)%256)/16;
    TimeDisp[3] = ((s_diff%4096)%256)%16;
    tm1637.display(TimeDisp);
  }
}

//for the end of the day when there are no more alarms
//displays 4 random characters on the led display
void display_random(DateTime now)
{
  if (random(0,2))
  {
    TimeDisp[0] = now.hour()/10;
    TimeDisp[1] = now.hour()%10; 
    TimeDisp[2] = now.minute()/10; 
    TimeDisp[3] = now.minute()%10; 
  }
  else
  {
    TimeDisp[0] = random(0,25);
    TimeDisp[1] = random(0,25); 
    TimeDisp[2] = random(0,25); 
    TimeDisp[3] = random(0,25); 
  }
  tm1637.display(TimeDisp);
}

