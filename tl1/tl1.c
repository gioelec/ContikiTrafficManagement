#include "../tl.h"





PROCESS(traffic_light, "Traffic Light");

AUTOSTART_PROCESSES(&traffic_light);


//------FUNCTIONS




//------PROCESS
PROCESS_THREAD(traffic_light, ev, data){
	PROCESS_EXITHANDLER(runicast_close(&runicast));
	PROCESS_BEGIN();
	runicast_open(&runicast, 144, &runicast_calls);
	printf("The Rime address of TL1 mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
	initialize();
	printf("process beginned\n");
	etimer_set(&senseTimer,sensingPeriod*CLOCK_SECOND);
	SENSORS_ACTIVATE(button_sensor);
	while(true){
		PROCESS_WAIT_EVENT();
		if (etimer_expired(&blueTimer) && battery == LOW){    ///DO I HAVE TO REDUCE BATT HERE???
	  		leds_toggle(LEDS_BLUE);
	  		etimer_set(&blueTimer,CLOCK_SECOND*BLUE_PERIOD);
	  		printf("Toggled blue leds\n");
	  	}
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
	  	}
	  	if (ev == sensors_event && data == &button_sensor){
	  		rechargeBattery();
	  	}
	}
	SENSORS_DEACTIVATE(button_sensor);



	PROCESS_END();
}
