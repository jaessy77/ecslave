#include "../include/xgeneral.h"
#include "../stack/ethercattype.h"
#include "../stack/fsm_slave.h"
#include "../stack/ecs_slave.h"
#include "../stack/ec_sii.h"
#include "../stack/ec_regs.h"
#include "../include/ec_net.h"
#include "../stack/ec_process_data.h"
#include "../include/ec_com.h"

/*
 * scan network device list, find the interfaces by name and 
 *  grab the interface(s) ;
*/
int ec_net_init(e_slave* ecs)
{ 
	ecs->intr[RX_] = kmalloc(GFP_KERNEL,);
	ec_device_init(&ecs->intr[RX]);
	ec_device_init(&ecs->intr[TX]);
	return 0;
}

void tx_packet(uint8_t *buf, int size, ec_interface *intr)
{
	ec_device_send(
		intr->tx_skb[intr->tx_ring_index], size);
}

int ec_is_nic_link_up(e_slave *ecs, ec_interface *intr)
{
	return intr->link_state;
}
