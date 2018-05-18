#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/sht11/sht11-sensor.h"



#define true 		1
#define false 		0

enum roadState{
	EMPTY,
	NORMAL,
	VEHICLE
};
typedef int bool;

const linkaddr_t tl1Address = {{1,0}};
const linkaddr_t tl2Address = {{2,0}};
const linkaddr_t g1Address  = {{3,0}};
const linkaddr_t g2Address  = {{4,0}};
//----FUNCTIONS TO BE CALLED EXACTLY IN THIS SEQUENCE----------
int getTemperature(){
	SENSORS_ACTIVATE(sht11-sensor);
	return ((sht11_sensor.value(SHT11_SENSOR_TEMP)/10-396)/10);
}
int getHumidity(){
	int h = sht11_sensor.value(SHT11_SENSOR_HUMIDITY)/41;
	SENSORS_DEACTIVATE(sht11-sensor);
	return h;
}



