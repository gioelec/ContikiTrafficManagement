/*
 * Broadcast.c
 *
 *  Created on: Apr 25, 2018
 *      Author: user
 */


PROCESS(traffic_light, "Traffic Light");

AUTOSTART_PROCESSES(&traffic_light);
static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno){
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){
}

static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};
static struct runicast_conn runicast;

PROCESS_THREAD(traffic_light, ev, data){
  static struct etimer timer;
  
  etimer_set(&timer,CLOCK_SECOND*cycles);

  PROCESS_BEGIN();


  PROCESS_END();
}
