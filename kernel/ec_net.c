#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_device.h"
#include "fsm_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"

/*
 * scan network device list, find the interfaces by name and 
 *  grab the interface(s) ;
*/
int ec_net_init(e_slave* ecs)
{ 

	ecs->intr[RX_INT_INDEX] = kmalloc(sizeof(struct ec_device), GFP_KERNEL);
	ec_device_init(ecs->intr[RX_INT_INDEX], ecs);

	ecs->intr[TX_INT_INDEX] = kmalloc(sizeof(struct ec_device) ,GFP_KERNEL);
	ec_device_init(ecs->intr[TX_INT_INDEX], ecs);
	return 0;
}

void tx_packet(uint8_t *buf, int size,struct ec_device *ecdev)
{
	ec_device_send(ecdev, size);
}

int ec_is_nic_link_up(e_slave *ecs,struct ec_device *intr)
{
	return intr->link_state;
}
