#include "header.h"

#define SECOND_PRESS 	2


struct etimer secondPressTimer;
bool stopped = false; //determines wether a vehicle is already at the stop
bool emergency=false;

//------BROADCAST CALLBACK 
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
}

static void broadcast_sent(struct broadcast_conn *c, int status, int num_tx){
}
//------BROADCAST STRUCT
static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent}; //Be careful to the order
static struct broadcast_conn broadcast;

//------FUNCTIONS
char getEncoded(emergency){
	if(emergency)
		return 'e';
	return 'n';
}
void sendBroadcast(){
	char c = getEncoded(emergency); //in order to send less bytes
	packetbuf_copyfrom(&c, sizeof(c));
    broadcast_send(&broadcast);
    emergency = false;
}
