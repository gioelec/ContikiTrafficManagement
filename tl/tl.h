#include "../header.h"
#include "dev/leds.h"

//toggling semiperiod
#define SEMI_PERIOD 	1
//battery costants
#define FULL_TH    		100
#define HALF_TH    		50
#define LOW_TH			20
#define TOGGLE_COST		5
#define SENSE_COST 		10
//blue leds toggling period
#define BLUE_PERIOD		2
//sensing costants
#define HALF_SENSE		10 
#define FULL_SENSE		5
#define LOW_SENSE 		20
#define EMPTY_SENSE     60
//scheduling traffic period
#define SCHEDULE_PERIOD 5


//------PROCESS STRUCTURE


PROCESS(traffic_light, "Traffic Light");

AUTOSTART_PROCESSES(&traffic_light);



enum batteryState{
	FULL,HALF,LOW,EMPTY
};
enum roadState{
	EMPTYROAD,NORMAL,EMERGENCY
};
//------VARIABLES
struct etimer blueTimer;
struct etimer toggleTimer;
struct etimer scheduleTimer;
	

enum batteryState battery = FULL;
int batteryLevel          = 100;
enum roadState road       = EMPTYROAD;
enum roadState otherRoad  = EMPTYROAD;
int sensingPeriod		  = FULL_SENSE; //variable modified when battery is decremented
bool blueStarted 		  =	false;
bool mainRoad 			  = false;		//states wether the road is the main one or not
bool gone				  = false;
bool scheduleTimerRunning = false;

static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno);
static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions);
static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions);
bool isMainRoad();
void initialize();
void consumeBattery(int v);
void rechargeBattery();
void toggleLights();
void toggleBlue();
void closeAll();
void turnGreen();
void turnRed();
void scheduleTraffic();
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);


