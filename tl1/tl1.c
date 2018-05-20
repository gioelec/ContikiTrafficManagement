#include "../tl.h"





PROCESS(traffic_light, "Traffic Light");

AUTOSTART_PROCESSES(&traffic_light);


//------FUNCTIONS

//------BROADCAST CALLBACK 
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
	bool isNormal = (((char *)packetbuf_dataptr())[0]=='n');
	printf("broadcast message received from %d.%d: '%s'\n", from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
	if(linkaddr_cmp(from,&g1Address)){
		if(isNormal){
			printf("My road ordinary vehicle\n");
			road = NORMAL;
		}else{
			printf("My road emergency vehicle\n");
			road = EMERGENCY;
		}
	}else if(linkaddr_cmp(from,&g2Address)){
		if(isNormal){
			printf("Other road ordinary vehicle\n");
			otherRoad = NORMAL;
		}else{
			printf("Other road emergency vehicle\n");
			otherRoad = EMERGENCY;
		}
	}
	printf("calling scheduleTraffic from receive\n");
	if(!scheduleTimerRunning && scheduleTraffic()){  //first time the time scheduler might not be running
		gone = true;
		printf("GREEN vehicle can proceed\n");
	}
}

//------BROADCAST STRUCT
static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent}; //Be careful to the order




//------PROCESS
PROCESS_THREAD(traffic_light, ev, data){
	mainRoad = true;
	PROCESS_EXITHANDLER(closeAll());
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	runicast_open(&runicast, 144, &runicast_calls);
	printf("The Rime address of TL1 mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
	initialize();
	printf("process beginned\n");
	etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	SENSORS_ACTIVATE(button_sensor);
	while(true){
		PROCESS_WAIT_EVENT();
		if (etimer_expired(&blueTimer) && battery == LOW){    ///DO I HAVE TO REDUCE BATT HERE???
			toggleBlue();
	  	}/*
	  	if (etimer_expired(&senseTimer)){
	  		sample.temp = getTemperature();
	  		sample.hum = getHumidity();
	  		consumeBattery(SENSE_COST);
	  		printf("New battery level %d\n", batteryLevel );
	  		etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	  		printf("Sensed  temp: %d hum: %d \n",sample.temp,sample.hum);
	  		if(battery == LOW && !blueStarted){
	  			etimer_set(&blueTimer,CLOCK_SECOND*BLUE_PERIOD);
	  			blueStarted = true;
	  		}
	  		sendData(sample);
	  	}*/
	  	if (ev == sensors_event && data == &button_sensor){
	  		rechargeBattery();
	  	}
	  	if(!scheduleTimerRunning&& etimer_expired(&toggleTimer))//&& otherRoad==EMPTYROAD && road == EMPTYROAD && batteryLevel>LOW_TH)
	  		toggleLights();
	  	if(etimer_expired(&scheduleTimer)){//&& )){
			printf("expired scheduleTimer\n");
	  		if(scheduleTimerRunning){
		  		if(gone)
		  			sendNext(&g1Address);
		  		scheduleTimerRunning=false;
		  		printf("calling schedule traffic from expiration\n");
		  		scheduleTraffic();
	  		}
	  	}
	  	
	}
	SENSORS_DEACTIVATE(button_sensor);
	PROCESS_END();
}
