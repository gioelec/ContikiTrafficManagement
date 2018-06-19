#include "header.h"

#define SECOND_PRESS 	0.5


struct etimer secondPressTimer;
bool stopped = false; //determines wether a vehicle is already at the stop
bool emergency=false;

//------BROADCAST STRUCT
static const struct broadcast_callbacks broadcast_call = {}; //Be careful to the order
static struct broadcast_conn broadcast;

//------FUNCTIONS
//from a boolean returns the corresponding letter n(ormal) or e(mergency)
char getEncoded(bool emergency){
	if(emergency)
		return 'e';
	return 'n';
}
void sendBroadcast(){
	char c = getEncoded(emergency); //in order to send less bytes int->char
	packetbuf_copyfrom(&c, sizeof(c));
    broadcast_send(&broadcast);
    emergency = false;
}
