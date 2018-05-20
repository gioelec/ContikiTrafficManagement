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
const int tl1Index = 0;
const int tl2Index = 1;
const int g2Index  = 3;
const linkaddr_t tl1Address = {{1,0}};
const linkaddr_t tl2Address = {{2,0}};
const linkaddr_t g1Address  = {{3,0}};
const linkaddr_t g2Address  = {{4,0}};
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


