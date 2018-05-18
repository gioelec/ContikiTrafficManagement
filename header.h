#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "stdio.h"


const linkaddr_t g1Address  = {{1,0}};
const linkaddr_t g2Address  = {{2,0}};
const linkaddr_t tl1Address = {{3,0}};
const linkaddr_t tl2Address = {{4,0}};

void linkaddr_set_node_addr(linkaddr_t *t){
  linkaddr_copy(&linkaddr_node_addr, t);
}
