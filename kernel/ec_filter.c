#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/module.h>

#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_offsched.h"
#include "ec_net.h"
#include "ec_device.h"
#include "ec_cmd.h"

e_slave *eslave = 0;
static struct nf_hook_ops ec_filterops;

static int returning_pkt(struct sk_buff *skb)
{
	if (skb->dev == eslave->intr[RX_INT_INDEX]->dev)
		return 0;
	if (skb->dev == eslave->intr[TX_INT_INDEX]->dev)
		return 1;

	/* if we have more than 2 interfaces and got grabage somehow just drop packet*/
	return -1; 
}

void ec_process_pkt(struct sk_buff* skb)
{
	int ret;
	/* 
	 * first check if packet is traversing back.
	 * we do not check for for outgoing as it filtered
	 * out by the filter driver. 
	*/
	ret = returning_pkt(skb);
	if (ret == 1){
		struct ec_device * device = eslave->intr[RX_INT_INDEX];
    		ret = device->dev->netdev_ops->ndo_start_xmit(skb, device->dev);
		if (ret == NETDEV_TX_OK) {
			device->tx_count++;
			return;
		} 
		device->tx_errors++;
		return;
	}
	if (ret == 0) {
		struct ec_device *device = eslave->intr[RX_INT_INDEX];
		device->processed_skb = skb;
		ec_process_datagrams(eslave, skb->len, skb->data);
	}
}

unsigned int ec_hook(unsigned int hooknum,	/* */
		  struct sk_buff *skb,	/* */
		  const struct net_device *in,	/* if so , the nic receiving skb */
		  const struct net_device *out,	/* if so , the nic used to transmit skb */
		  int (*okfn) (struct sk_buff *))
{				/* invoked when all function said NF_ACCEPT for this skb */
	
	if (out) {/* do not process outgoing packets */
		return NF_ACCEPT;
	}
     	if (eth_hdr(skb)->h_proto != htons(EC_ECATTYPE))
                return NF_ACCEPT;
	ec_process_pkt(skb);
	return NF_DROP;
}

int 	ec_filter_init(e_slave *ecs)
{
	eslave = ecs;
	ec_filterops.hook = ec_hook;
	ec_filterops.hooknum = NF_INET_PRE_ROUTING;
	ec_filterops.pf = PF_INET;
	ec_filterops.priority = NF_IP_PRI_FIRST;
	return nf_register_hook(&ec_filterops);
}

void    ec_filter_cleanup(void)
{
	nf_unregister_hook(&ec_filterops);
}
