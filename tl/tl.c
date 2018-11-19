#include "../tl.h"

//------FUNCTIONS

//------BROADCAST CALLBACK 
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
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
	}else if(linkaddr_cmp(from,&g2Address)){
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
	mainRoad = isMainRoad();
	linkaddr_t gAddress= mainRoad?g1Address:g2Address;
	linkaddr_t otherAddress = mainRoad?g2Address:g1Address;
	PROCESS_EXITHANDLER(closeAll());
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	runicast_open(&runicast, 144, &runicast_calls);
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
