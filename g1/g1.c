#include "../header.h"
#include "dev/serial-line.h"

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
char* 	emergencyMsg;//= {"EMERGENCY MESSAGE "};
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
	printf("%s temp: %d hum: %d\n",emergencyMsg,t,h);
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
//------RUNICAST CALLBACK
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
	int sender = (int)from->u8[0];
	printf("runicast message received from %d.%d, seqno %d\n", from->u8[0], from->u8[1], seqno);
	struct sampleData* data = (struct sampleData*)packetbuf_dataptr();
	printf("RECEIVED: Temperature: %d, Humidity: %d\n",data->temp,data->hum);
	addToBuffer(data,sender-1);
	///if samples 4
	//process_post(&g1, PROCESS_EVENT_MSG, NULL);
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


//------PROCESS G1

PROCESS_THREAD(g1, ev, data){
  PROCESS_EXITHANDLER(runicast_close(&runicast));

  PROCESS_BEGIN();

  runicast_open(&runicast, 144, &runicast_calls);

  //This node is the receiver...IN THIS EXAMPLE, it waits forever and just reacts to received packets
  PROCESS_WAIT_EVENT_UNTIL(0);

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
			emergencyMsg = (char *)malloc((strlen(data)+1)*sizeof(char));
			strcpy(emergencyMsg,(char*)data);
			printf("you have inserted the following message%s\n", emergencyMsg);
		}
	}
  	PROCESS_END();


}