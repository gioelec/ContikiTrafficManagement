#include "../tl.h"





PROCESS(traffic_light, "Traffic Light");

AUTOSTART_PROCESSES(&traffic_light);
//---RUNICAST CALLBACK
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}
//------BROADCAST CALLBACK 
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
}

static void broadcast_sent(struct broadcast_conn *c, int status, int num_tx){
}
//------BROADCAST STRUCT
static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent}; //Be careful to the order
static struct broadcast_conn broadcast;

//------RUNICAST STRUCT
static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};
static struct runicast_conn runicast;

//------FUNCTIONS





//------PROCESS
PROCESS_THREAD(traffic_light, ev, data){
	
	PROCESS_BEGIN();
	int temp,hum;
	printf("The Rime address of TL1 mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
	initialize();
	printf("process beginned\n");
	etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	while(true){
		PROCESS_WAIT_EVENT();
	  	if (etimer_expired(&senseTimer)){
	  		temp = getTemperature();
	  		hum = getHumidity();
	  		consumeBattery(SENSE_COST);
	  		printf("New battery level %d\n", batteryLevel );
	  		etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	  		printf("Sensed  temp: %d hum: %d \n",temp,hum);
	  	}
	}


	PROCESS_END();
}
