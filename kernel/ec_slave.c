#include "../include/xgeneral.h"
#include "../stack/ethercattype.h"
#include "../stack/fsm_slave.h"
#include "../stack/ecs_slave.h"
#include "../stack/ec_sii.h"
#include "../stack/ec_regs.h"
#include "../include/ec_net.h"
#include "../stack/ec_process_data.h"
#include "../include/ec_com.h"

static struct fsm_slave fsm_slave;
static e_slave ecs;


void ecs_module_cleanup(void)
{
	
}

int ec_capture(e_slave *ecs)
{
	return 0;
}

int ecs_module_init(void)
{
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
	ecs.dgram_processed = &ecs.pkt_head[0];
	ecs.dgrams_cnt = 0;
	ec_capture(&ecs);
	return 0;
}

module_init(ecs_module_init);
module_exit(ecs_module_cleanup);
