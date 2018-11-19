#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "stdio.h"


#define MAX_RETRANSMISSIONS 5
#define true 				1
#define false 				0


#define COOJA

typedef int bool;

struct sampleData {
	int temp;
	int hum;
};
struct sampleData sample  = {0,0};

//indexes in the vector of each sensor
const int tl1Index = 0;
const int tl2Index = 1;
const int g2Index  = 3;

//addresses of each node
#ifndef COOJA
	const linkaddr_t tl1Address = {{2,0}};
	const linkaddr_t tl2Address = {{45,0}};
	const linkaddr_t g1Address  = {{4,0}};
	const linkaddr_t g2Address  = {{1,0}};
	#define TL1_ADDR			2
	#define TL2_ADDR			45
#else
	//COOJA ADDRESSES
	#define TL1_ADDR			1
	#define TL2_ADDR			2
	const linkaddr_t tl1Address = {{1,0}};
	const linkaddr_t tl2Address = {{2,0}};
	const linkaddr_t g1Address  = {{3,0}};
	const linkaddr_t g2Address  = {{4,0}};
#endif
struct etimer senseTimer;
bool runicastExist=true;



static struct runicast_conn runicast;

//----FUNCTIONS TO BE CALLED EXACTLY IN THIS SEQUENCE----------
int getTemperature(){
	SENSORS_ACTIVATE(sht11_sensor);
	return ((sht11_sensor.value(SHT11_SENSOR_TEMP)/10-396)/10);
}
int getHumidity(){
	int h = sht11_sensor.value(SHT11_SENSOR_HUMIDITY)/41;
	SENSORS_DEACTIVATE(sht11_sensor);
	return h;
}

void sendData(){ //Sends the sensed data in runicast to the sink
	if( !runicast_is_transmitting(&runicast)) {
	    packetbuf_copyfrom(&sample, sizeof(sample));
	    //printf("%u.%u: sending runicast to address %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], g1Address.u8[0], g1Address.u8[1]);
	    runicast_send(&runicast, &g1Address, MAX_RETRANSMISSIONS);
	}
}
int get_index(const linkaddr_t* link) {
	if(linkaddr_cmp(link,&tl1Address)) return TL1_ADDR;
	if(linkaddr_cmp(link,&tl2Address)) return TL2_ADDR;
	return -1;
}