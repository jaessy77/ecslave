#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/uaccess.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <linux/can.h>
#include <linux/can/core.h>
#include <linux/ratelimit.h>
#include <net/net_namespace.h>
#include <net/sock.h>

#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_offsched.h"
#include "ec_net.h"
#include "ec_device.h"
#include "ec_cmd.h"


e_slave *eslave = 0;

static int returning_pkt(struct sk_buff *skb)
{
	if (skb->dev == eslave->intr[RX_INT_INDEX]->dev)
		return 0;
	if (skb->dev == eslave->intr[TX_INT_INDEX]->dev)
		return 1;

	/* if we have more than 2 interfaces and got grabage somehow just drop packet */
	return -1;
}

void ecat_process_pkt(struct sk_buff *skb)
{
	int ret;
	/* 
	 * first check if packet is traversing back.
	 * we do not check for for outgoing as it is filtered
	 * out by the filter driver. 
	 */
	ret = returning_pkt(skb);
	if (ret == 1) {
		ec_device_send(eslave->intr[RX_INT_INDEX], skb);
		return;
	}
	if (ret == 0) {
		struct ec_device *device = eslave->intr[RX_INT_INDEX];
		device->processed_skb = skb;
		ec_process_datagrams(eslave, skb->len, skb->data);
	}
}

static int ecat_rcv(struct sk_buff *skb, struct net_device *dev, 
                      struct packet_type *pt, struct net_device *orig_dev) 
{
	if (eth_hdr(skb)->h_proto != htons(ETH_P_ECAT))
		return   NET_RX_DROP;
	ecat_process_pkt(skb);
	return NET_RX_SUCCESS;
}

static struct packet_type ecat_packet __read_mostly = {
        .type = cpu_to_be16(ETH_P_ECAT),
        .dev  = NULL,
        .func = ecat_rcv,
};

int ecat_proto_init(e_slave * ecs)
{
	eslave = ecs;
	dev_add_pack(&ecat_packet);
	return 0;
}

void ecat_proto_cleanup(void)
{
	dev_remove_pack(&ecat_packet);
}