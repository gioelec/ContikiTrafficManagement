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
#define SCHEDULE_PERIOD 5


enum batteryState{
	FULL,HALF,LOW,EMPTY
};
enum roadState{
	EMPTYROAD,NORMAL,EMERGENCY
};
//------VARIABLES
struct etimer senseTimer;
struct etimer blueTimer;
struct etimer toggleTimer;
struct etimer scheduleTimer;


enum batteryState battery = FULL;
int batteryLevel          = 100;
enum roadState road       = EMPTYROAD;
enum roadState otherRoad  = EMPTYROAD;
int sensingPeriod		  = FULL_SENSE;
bool blueStarted 		  =	false;
struct sampleData sample  = {0,0};
//---RUNICAST CALLBACK
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}
//------BROADCAST CALLBACK
static void broadcast_sent(struct broadcast_conn *c, int status, int num_tx){
}


//------RUNICAST STRUCT
static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};
static struct runicast_conn runicast;
//------BROADCAST STRUCT
static struct broadcast_conn broadcast;


//------FUNCTIONS
void initialize(){
	etimer_set(&toggleTimer,CLOCK_SECOND);
	batteryLevel=100;
	battery = FULL;
	road = EMPTYROAD;
	otherRoad = EMPTYROAD;
	sensingPeriod = FULL_SENSE;
}
void consumeBattery(int v){
	batteryLevel-=v;
	if(batteryLevel <= 0){
		battery = EMPTY;
		sensingPeriod = LOW_SENSE;//what TODO here
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
	consumeBattery(batteryLevel-FULL_TH);
	if(blueStarted){
		etimer_stop(&blueTimer);
		blueStarted = false;
		leds_off(LEDS_BLUE);
	}
}
void sendData(){
	if(!runicast_is_transmitting(&runicast)) {
	    packetbuf_copyfrom(&sample, sizeof(sample));
	    printf("%u.%u: sending runicast to address %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], g1Address.u8[0], g1Address.u8[1]);
	    runicast_send(&runicast, &g1Address, MAX_RETRANSMISSIONS);
	}
}
void toggleLights(){
	leds_toggle(LEDS_GREEN|LEDS_RED);
	etimer_set(&toggleTimer,CLOCK_SECOND);
	consumeBattery(5);
}
void toggleBlue(){
	leds_toggle(LEDS_BLUE);
	etimer_set(&blueTimer,CLOCK_SECOND*BLUE_PERIOD);
	printf("Toggled blue leds\n");
}
void closeAll(){
	//closes all the opened connection
	runicast_close(&runicast);
	broadcast_close(&broadcast);
}
void turnGreen(){
	leds_off(LEDS_RED);
	leds_on(LEDS_GREEN);
}
void turnRed(){
	leds_off(LEDS_GREEN);
	leds_on(LEDS_RED);
}
void scheduleTraffic(){
	etimer_set(&scheduleTimer,CLOCK_SECOND*SCHEDULE_PERIOD);
	if(otherRoad==EMPTYROAD && road != EMPTYROAD)
		turnGreen();
}