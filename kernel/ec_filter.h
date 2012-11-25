#ifndef __EC_FILTER_H__
#define __EC_FILTER_H__

int  ec_filter_init(e_slave *ecs);
void ec_filter_cleanup(void);
void ec_process_pkt(struct sk_buff*);

#endif
