#ifndef __EC_NET_H__
#define __EC_NET_H__

struct __ec_device__;
struct __e_slave__;

#define RX_INT_INDEX	0	/* port in the master side */
#define TX_INT_INDEX	1	/* port in the next side  */

int ec_is_nic_link_up(struct __e_slave__ *,struct __ec_device__ *);
struct __ec_interface__ * ec_tx_interface(struct __e_slave__ *);
struct __ec_interface__ * ec_rx_interface(struct __e_slave__ *);
int ec_net_init(struct __e_slave__ *);

/* is this last slave */
static inline int ec_is_nic_loop_closed(e_slave * esv)
{
	return esv->intr[TX_INT_INDEX] == esv->intr[RX_INT_INDEX];
}

static inline int ec_is_nic_signal_detected(e_slave * esv, struct __ec_device__ *intr)
{
	return ec_is_nic_link_up(esv, intr);
}

#endif
