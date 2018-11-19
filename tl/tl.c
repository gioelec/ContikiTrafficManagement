#include "tl.h"

//------FUNCTIONS
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
bool isMainRoad(){
	bool mainRoad = get_index(&linkaddr_node_addr)==TL1_ADDR;
	if(mainRoad)
		printf("I am the main road\n");
	else
		printf("I am not the main road\n");
	return mainRoad;
}
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
	etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND); //to avoid sensing with a lower rate the first time
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


//------BROADCAST CALLBACK 
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
	linkaddr_t gAddress= mainRoad?g1Address:g2Address;
	linkaddr_t otherAddress = mainRoad?g2Address:g1Address;
	bool isNormal = (((char *)packetbuf_dataptr())[0]=='n');
	//printf("TL1 RECEIVE:broadcast message received from %d.%d: '%s'\n", from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
	if(linkaddr_cmp(from,&gAddress)){
		if(road!=EMPTYROAD){
	//		printf("Road already occupied, ignoring\n");
			return;
		}
		if(isNormal){
	//		printf("My road ordinary vehicle\n");
			road = NORMAL;
		}else{
	//		printf("My road emergency vehicle\n");
			road = EMERGENCY;
		}
	}else if(linkaddr_cmp(from,&otherAddress)){
		if(otherRoad!=EMPTYROAD){
	//		printf("Road already occupied, ignoring\n");
			return;
		}
		if(isNormal){
	//		printf("Other road ordinary vehicle\n");
			otherRoad = NORMAL;
		}else{
	//		printf("Other road emergency vehicle\n");
			otherRoad = EMERGENCY;
		}
	}
	if(!scheduleTimerRunning){  //we have to check that we are not already in a scheduled situation otherwise wait the scheduling timer to expire
		scheduleTraffic();
	}else{
	//	printf("SCHEDULING already ongoing\n");
	}	

}

//------BROADCAST STRUCT
static const struct broadcast_callbacks broadcast_call = {broadcast_recv}; //Be careful to the order




//------PROCESS
PROCESS_THREAD(traffic_light, ev, data){
	PROCESS_EXITHANDLER(closeAll());
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	runicast_open(&runicast, 144, &runicast_calls);
	mainRoad = isMainRoad();
	printf("The Rime address of TL1 mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
	initialize();
	SENSORS_ACTIVATE(button_sensor);
	while(true){
		PROCESS_WAIT_EVENT();
		if (etimer_expired(&blueTimer)){    
			if(battery==LOW){
				toggleBlue();
			}else if(battery==EMPTY)
				leds_on(LEDS_BLUE);
	  	}
	  	if (etimer_expired(&senseTimer)){
	  		sample.temp = getTemperature();
	  		sample.hum = getHumidity();
	  		consumeBattery(SENSE_COST);
	  		//printf("New battery level %d\n", batteryLevel );
	  		etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	  		//printf("Sensed  temp: %d hum: %d \n",sample.temp,sample.hum);
	  		if(battery == LOW && !blueStarted){
	  			etimer_set(&blueTimer,CLOCK_SECOND*BLUE_PERIOD);
	  			blueStarted = true;
	  		}
	  		printf("sendData\n");
	  		sendData(sample);
	  	}

	  	if (ev == sensors_event && data == &button_sensor){
	  		rechargeBattery();
	  	}
	  	if(ev==PROCESS_EVENT_MSG && !scheduleTimerRunning){
	  		scheduleTimerRunning = true;
			etimer_set(&scheduleTimer,SCHEDULE_PERIOD*CLOCK_SECOND);
			//if(gone)
			//	sendNext(&g1Address);
	  	}	  	
	  	if(!scheduleTimerRunning && etimer_expired(&toggleTimer)&& otherRoad==EMPTYROAD && road == EMPTYROAD )
	  		toggleLights();
	  	if(ev!=PROCESS_EVENT_MSG && etimer_expired(&scheduleTimer)&&scheduleTimerRunning){
		  	scheduleTimerRunning=false;
		  	scheduleTraffic();
	  	}
	}
	SENSORS_DEACTIVATE(button_sensor);
	PROCESS_END();
}
