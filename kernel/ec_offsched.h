#ifndef __OFFSCHED_H__
#define __OFFSCHED_H__

struct sk_buff;

void ec_offsched_cleanup(void);
int  ec_offsched_init(void);
void ec_process_pkt(struct sk_buff* skb);

#endif
