#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "stdio.h"


#define MAX_RETRANSMISSIONS 5
#define true 				1
#define false 				0


typedef int bool;

struct sampleData {
	int temp;
	int hum;
};
struct sampleData sample  = {0,0};
const int tl1Index = 0;
const int tl2Index = 1;
const int g2Index  = 3;
/*
const linkaddr_t tl1Address = {{2,0}};
const linkaddr_t tl2Address = {{45,0}};
const linkaddr_t g1Address  = {{50,0}};
const linkaddr_t g2Address  = {{51,0}};*/
//COOJA ADDRESSES

const linkaddr_t tl1Address = {{1,0}};
const linkaddr_t tl2Address = {{2,0}};
const linkaddr_t g1Address  = {{3,0}};
const linkaddr_t g2Address  = {{4,0}};

struct etimer senseTimer;



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

void sendData(){
	printf("SENDING DATA\n");
	if(!runicast_is_transmitting(&runicast)) {
	    packetbuf_copyfrom(&sample, sizeof(sample));
	    printf("%u.%u: sending runicast to address %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], g1Address.u8[0], g1Address.u8[1]);
	    runicast_send(&runicast, &g1Address, MAX_RETRANSMISSIONS);
	}
}
