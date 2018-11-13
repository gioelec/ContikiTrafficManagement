#include "header.h"
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
//---RUNICAST CALLBACK
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}


//------RUNICAST STRUCT
static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};
//------BROADCAST STRUCT
static struct broadcast_conn broadcast;


//------FUNCTIONS
void initialize(){
	etimer_set(&toggleTimer,SEMI_PERIOD*CLOCK_SECOND);
	batteryLevel=100;
	battery = FULL;
	road = EMPTYROAD;
	otherRoad = EMPTYROAD;
	sensingPeriod = FULL_SENSE;
	scheduleTimerRunning = false;
}

void consumeBattery(int v){
	batteryLevel-=v;
	if(batteryLevel <= 0){
		batteryLevel=0;
		battery = EMPTY;
		sensingPeriod = EMPTY_SENSE;
		return;
	}
	if(batteryLevel <= LOW_TH){
		battery = LOW;
		sensingPeriod = LOW_SENSE;
		return;
	}
	if(batteryLevel <= HALF_TH){
		battery = HALF;
		sensingPeriod = HALF_SENSE;
		return;
	}
	if(batteryLevel > HALF_TH){
		battery = FULL;
		sensingPeriod = FULL_SENSE;
		return;
	}
}
void rechargeBattery(){
	consumeBattery(batteryLevel-FULL_TH); //negative consume consists in a recharge
	etimer_set(&senseTimer,sensingPeriod); //to avoid sensing with a lower rate the first time
	leds_off(LEDS_BLUE);
	if(blueStarted){					  //if we were in the low state we should disable bluetoggling
		etimer_stop(&blueTimer);
		blueStarted = false;
	}
}
/*
void sendNext(const linkaddr_t* recv){	 //tells to corresponding g sensor that another car can come
	printf("SENDING NEXT\n");
	gone = false;
	char c = 'n';
	road=EMPTYROAD;
	if(!runicast_is_transmitting(&runicast)) {
	    packetbuf_copyfrom(&c, sizeof(c));
	    printf("%u.%u: sending runicast to address %u.%u\n", recv->u8[0], recv->u8[1], g1Address.u8[0], g1Address.u8[1]);
	    runicast_send(&runicast, recv, MAX_RETRANSMISSIONS);
	}

}*/
void toggleLights(){
	leds_toggle(LEDS_GREEN|LEDS_RED);
	etimer_set(&toggleTimer,CLOCK_SECOND*SEMI_PERIOD);
	consumeBattery(TOGGLE_COST);
}
void toggleBlue(){
	leds_toggle(LEDS_BLUE);
	etimer_set(&blueTimer,CLOCK_SECOND*BLUE_PERIOD);
	//printf("Toggled blue leds\n");
}
void closeAll(){	//closes all the opened connection
	runicast_close(&runicast);
	broadcast_close(&broadcast);
}
void turnGreen(){
	leds_off(LEDS_RED);
	leds_on(LEDS_GREEN);
	gone = true;							//one vehicle has left the stop
	road=EMPTYROAD;
	//printf("GREEN vehicle can proceed\n");
}
void turnRed(){
	leds_off(LEDS_GREEN);
	leds_on(LEDS_RED);
	otherRoad = EMPTYROAD;
	gone = false;
	//printf("RED vehicle cannot proceed\n");

}
void scheduleTraffic(){
	//printf("SCHEDULE TRAFFIC()\n");
	if(otherRoad == EMPTYROAD && road == EMPTYROAD){
		//printf("Both roads are empty\n");
		//printf("calling toggle toggleLights\n");
		leds_off(LEDS_RED|LEDS_GREEN);
		toggleLights();
		return;  	}
	else if(otherRoad==EMPTYROAD && road != EMPTYROAD){
		//printf("other road empty\n");
		turnGreen();
	}
	else if(otherRoad!=EMPTYROAD && road == EMPTYROAD){
		//printf("my road is empty\n");
		turnRed();
	}
	else if(otherRoad == road){					//both normal or both emergency
		//printf("roads in the same state\n");
		if(mainRoad)
			turnGreen();
		else
			turnRed();
	}
	else if(road == EMERGENCY && otherRoad != EMERGENCY){ //emergency has an higher priority
		//printf("my road has an emergency\n");
		turnGreen();
	}
	else if(road != EMERGENCY && otherRoad == EMERGENCY){ //emergency has an higher priority
		//printf("other road has an emergency\n");
		turnRed();
	}
	process_post(&traffic_light, PROCESS_EVENT_MSG, NULL);
}
