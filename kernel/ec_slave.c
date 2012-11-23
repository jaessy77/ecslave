#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_device.h"
#include "globals.h"
#include "fsm_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"

static struct fsm_slave fsm_slave;
static e_slave ecs;

void ecs_module_cleanup(void)
{
	
}

int ecs_module_init(void)
{
	struct ec_device* device;

  	if (ec_net_init(&ecs) < 0){
		return -1;
	}
	ec_init_regs(&ecs);
	init_sii(&ecs);
	if (init_process_data(&ecs) < 0){
		printk("Illegal pdo configuration\n");
		return -1;
	}
	ecs.fsm = &fsm_slave;
	device = ecs.intr[RX_INT_INDEX];
	ecs.dgram_processed = 
		device->tx_skb[device->tx_ring_index]->data;
	ecs.dgrams_cnt = 0;
	return 0;
}

module_init(ecs_module_init);
module_exit(ecs_module_cleanup);
