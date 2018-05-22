#include "../g.h"
#include "dev/serial-line.h"
#include <stdlib.h>


#define QUEUESIZE 4

PROCESS(g1, "G1111111");
PROCESS(keyboard_process,"keyboard_process");

AUTOSTART_PROCESSES(&g1,&keyboard_process);

//------VARIABLES
int 	humStore[QUEUESIZE];
int 	tempStore[QUEUESIZE];
bool 	received[QUEUESIZE];
int 	remaining = 3;
int 	myAddr = 3-1;
char* 	emergencyMsg = NULL;//= {"EMERGENCY MESSAGE "};
char 	psw[] = {"NES"};

//------FUNCTIONS
void computeAverage(){
	int t=0,h =0,i=0;
	for (; i < QUEUESIZE; ++i){
		h += humStore[i];
		t += tempStore[i];
		received[i] = false; 
	}
	remaining = 3;
	t = t/4;
	h = h/4;
	if(emergencyMsg){
		printf("%s temp: %d hum: %d\n",emergencyMsg,t,h);
		free(emergencyMsg);
	}else
		printf("temp: %d hum: %d\n",t,h);

}
void addToBuffer(struct sampleData * data, int sender){
	tempStore[sender] = data->temp;
	humStore[sender] = data->hum;
	if(!received[sender]){
		received[sender] = true;
		remaining--;
		printf("REMAINING SAMPLES: %d\n",remaining );
		if(remaining<=0){
			tempStore[myAddr] = getTemperature();
			humStore[myAddr] = getHumidity();
			computeAverage();
		}
	}
}
void addMsg(char* data,int len){
	if(len<=1)					//VOID INPUT ? SHOULD BE HANDLED IN THIS WAY???
		return;
	emergencyMsg = (char *)malloc(len*sizeof(char));
	strcpy(emergencyMsg,data);
}
//------RUNICAST CALLBACK
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
	void * dataReceived = packetbuf_dataptr();
	printf("runicast message received from %d.%d, seqno %d\n", from->u8[0], from->u8[1], seqno);
	if(((char *)dataReceived)[0]=='n'){   
		printf("RECEIVED: next\n");
		stopped = false;
		return;
	}
	int s = (int)from->u8[0];
	int sender = (s==tl1Address.u8[0])?tl1Index:(s==tl2Address.u8[0])?tl2Index:g2Index;
	struct sampleData* data = (struct sampleData*)dataReceived;
	printf("RECEIVED: Temperature: %d, Humidity: %d\n",data->temp,data->hum);
	addToBuffer(data,sender);
	///if samples 4
	//process_post(&g1, PROCESS_EVENT_MSG, NULL);
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

//------RUNICAST STRUCT
static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};



void closeAll(){
	//closes all the opened connection
	runicast_close(&runicast);
	broadcast_close(&broadcast);
}

//------PROCESS G1

PROCESS_THREAD(g1, ev, data){
  	PROCESS_EXITHANDLER(closeAll());
  	//have to be together
  	PROCESS_BEGIN();
  	broadcast_open(&broadcast, 129, &broadcast_call);
  	runicast_open(&runicast, 144, &runicast_calls);
  	SENSORS_ACTIVATE(button_sensor);

  	while(true){
  		printf("waiting for an event-----G1\n");
  		PROCESS_WAIT_EVENT();
  		if (!stopped && ev == sensors_event && data == &button_sensor){
  			stopped = true;
  			etimer_set(&secondPressTimer,SECOND_PRESS*CLOCK_SECOND);
  			printf("First time the button is pressed\n");
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
   	}
  	SENSORS_DEACTIVATE(button_sensor);
  	PROCESS_END();
}

//------PROCESS keyboard_process
PROCESS_THREAD(keyboard_process,ev,data){
	PROCESS_BEGIN();
  	while(1){
		printf("Please, type the password\n");
		PROCESS_WAIT_EVENT_UNTIL(ev==serial_line_event_message);
		if(strcmp((char *)data, psw) != 0){
			printf("Incorrect password.\n");
			continue;
		}else{
			printf("Type in the Emergency Warning\n");
			PROCESS_WAIT_EVENT_UNTIL(ev==serial_line_event_message);
			addMsg(data,(strlen(data)+1));
			printf("you have inserted the following message%s\n", emergencyMsg);
		}
	}
  	PROCESS_END();
}