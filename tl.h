#include "header.h"
#include "dev/leds.h"
#define SEMI_PERIOD 	1
#define ON_PERIOD 		5
#define FULL_TH    		100
#define HALF_TH    		50
#define LOW_TH			20
#define TOGGLE_COST		5
#define SENSE_COST 		10
#define BLUE_PERIOD		2
#define HALF_SENSE		10 //defines period of sensing with half battery
#define FULL_SENSE		5
#define LOW_SENSE 		20

enum batteryState{
	FULL_BATTERY,HALF_BATTERY,LOW_BATTERY,NULL_BATTERY
};
static void initialize(static struct etimer*senseTimer){
	etimer_set(senseTimer,CLOCK_SECOND*cycles);
	batteryLevel = 100;
	batteryState = FULL_BATTERY;
	roadState = EMPTY;
}