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

#include "ec_offsched.h"
static struct nf_hook_ops ec_filterops;

/*
  NF_ACCEPT - may continue climb up the network stack
  NF_DROP   - packet was rejected and kernel should drop 
  NF_QUEUE  - pass packet to user space
  NF_STOLE  - kernel should forget this packet.
  NF_REPEAT - do not continue to next filter ( if exist ) but re-call this hook
*/
unsigned int ec_hook(unsigned int hooknum,	/* */
		  struct sk_buff *skb,	/* */
		  const struct net_device *in,	/* if so , the nic receiving skb */
		  const struct net_device *out,	/* if so , the nic used to transmit skb */
		  int (*okfn) (struct sk_buff *))
{				/* invoked when all function said NF_ACCEPT for this skb */
	
	if (out)
		return NF_ACCEPT;
     	if (eth_hdr(skb)->h_proto != htons(ETH_P_IP))
                return NF_ACCEPT;
	ec_deliver_to_offsched(skb);
	return NF_DROP;
}

int 	ec_filter_init_main(void)
{
	ec_filterops.hook = ec_hook;
	ec_filterops.hooknum = NF_INET_PRE_ROUTING;
	ec_filterops.pf = PF_INET;
	ec_filterops.priority = NF_IP_PRI_FIRST;
	return nf_register_hook(&filterops);
}

void    ec_filter_cleanup(void)
{
	nf_unregister_hook(&ec_filterops);
}
