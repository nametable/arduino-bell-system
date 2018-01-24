//This code by Logan Bateman - not publicly released
#include <math.h>

class TimeAndDOW {
public:
  TimeAndDOW (char hour, char min, char sec, unsigned char dow, char effect);
	TimeAndDOW (char hour, char min, char sec, unsigned char dow);
	TimeAndDOW (char hour, char min, char sec);
  TimeAndDOW (char hour, char min);
	  unsigned char day_of_week() const { return dow2; }
    char hour() const        { return hh; }
    char minute() const      { return mm; }
    char second() const      { return ss; }
    char effect() const      { return fx; }
protected:
	char hh, mm, ss, fx;
  unsigned char dow2;
};

static char DOWenum(char dow6){
	//Serial.println(dow6, DEC);
	unsigned char i=1 << dow6; //+1; //No need for +1 I think
	return i;}
