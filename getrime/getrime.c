
#include "stdio.h"
#include "contiki.h"
#include "net/rime/rime.h"

PROCESS(GetRime, "Process to get the Rime address of the node");

AUTOSTART_PROCESSES(&GetRime);

PROCESS_THREAD(GetRime, ev, data){

  PROCESS_BEGIN();

  printf("The Rime address of this mote is: %u.%u\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
  
  PROCESS_END();
}

