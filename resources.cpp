//This code by Logan Bateman - not publicly released
#include "resources.h"

TimeAndDOW::TimeAndDOW (char hour, char min, char sec, unsigned char dow, char effect) { //Specific day at specified time + effect
	dow2=dow;
	hh=hour;
	mm=min;
	ss=sec;
  fx=effect;
}
TimeAndDOW::TimeAndDOW (char hour, char min, char sec, unsigned char dow) { //Specific day at specified time
  dow2=dow;
  hh=hour;
  mm=min;
  ss=sec;
  fx=0;
}
TimeAndDOW::TimeAndDOW (char hour, char min, char sec) { //Everyday at this time
	dow2=127;
	hh=hour;
	mm=min;
	ss=sec;
  fx=0;
}
TimeAndDOW::TimeAndDOW (char hour, char min) {
  dow2=127;
  hh=hour;
  mm=min;
  ss=0;
  fx=101;
}
/*char DOWenum(char dow6)
{
	return 2^dow6;
}
*/
