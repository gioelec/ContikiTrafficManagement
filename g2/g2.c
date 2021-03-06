#include "../g.h"
#include "dev/serial-line.h"
#include <stdlib.h>


#define SENSE_PERIOD 5

PROCESS(g2,"g2_process");

AUTOSTART_PROCESSES(&g2);
//------VARIABLES

//------FUNCTIONS

//------RUNICAST CALLBACK
/*
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
	void * dataReceived = packetbuf_dataptr();
	printf("runicast message received from %d.%d, seqno %d\n", from->u8[0], from->u8[1], seqno);
	if(((char *)dataReceived)[0]=='n'){   
		printf("RECEIVED: next\n");
		stopped = false;
		return;
	}
}
*/
//------RUNICAST STRUCT
static const struct runicast_callbacks runicast_calls = {};


void closeAll(){
	//closes all the opened connection
	runicast_close(&runicast);
	broadcast_close(&broadcast);
}

//------PROCESS G2

PROCESS_THREAD(g2, ev, data){
  	PROCESS_EXITHANDLER(closeAll());
  	//have to be together
  	PROCESS_BEGIN();
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	runicast_open(&runicast, 144, &runicast_calls);
  	SENSORS_ACTIVATE(button_sensor);
  	etimer_set(&senseTimer,SENSE_PERIOD*CLOCK_SECOND); // every 5 seconds connected to the elecrticity supply
  	while(true){
  		//printf("waiting for an event-----G2\n");
  		PROCESS_WAIT_EVENT();
  		//if (!stopped && ev == sensors_event && data == &button_sensor){
  		if (ev == sensors_event && data == &button_sensor){
  			//stopped = true;
  			etimer_set(&secondPressTimer,SECOND_PRESS*CLOCK_SECOND);
  			//printf("First time the button is pressed\n");
			PROCESS_WAIT_EVENT();
  			if (ev == sensors_event && data == &button_sensor){
  				printf("Emegency vehicle detected\n");
  				etimer_stop(&secondPressTimer);
  				emergency = true;
		  	}else if(etimer_expired(&secondPressTimer)){
			  	printf("Normal vehicle detected\n");
		  	}
		  	sendBroadcast();
		}
		
		if (etimer_expired(&senseTimer)){
	  		sample.temp = getTemperature();
	  		sample.hum = getHumidity();
	  		etimer_reset(&senseTimer);
	  		//printf("Sensed  temp: %d hum: %d \n",sample.temp,sample.hum);
	  		sendData(sample);
	  	}
   	}
  	SENSORS_DEACTIVATE(button_sensor);
  	PROCESS_END();
}

