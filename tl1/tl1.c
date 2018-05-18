#include "../tl.h"
//------VARIABLES
roadState state;
batteryState battery=FULL_BATTERY;
int batteryLevel = 100;




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
	int temp,hum;
	printf("The Rime address of TL1 mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
	static struct etimer senseTimer;
	initialize(&senseTimer);
	PROCESS_BEGIN();
	while(true){
		PROCESS_WAIT_EVENT();
	  	if (etimer_expired(&senseTimer)){
	  		temp = getTemperature();
	  		hum = getHumidity();
	  		printf("Sensed  temp: %d hum: %d \n",temp,hum );
	  		etimer_reset(&senseTimer);///to remove;
	  	}
	}


	PROCESS_END();
}
