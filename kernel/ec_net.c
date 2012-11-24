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
#include "ec_debug.h"

static const unsigned int rate_intervals[] = {
    1, 10, 60
};


int ec_device_init(
        ec_device_t *device, /**< EtherCAT device */
        ecat_node_t *ecat_node /**< ethercat node owning the device */
        )
{
    int ret;
    unsigned int i;
    struct ethhdr *eth;

    device->ecat_node = ecat_node;
    device->tx_ring_index = 0;


    for (i = 0; i < EC_TX_RING_SIZE; i++)
        device->tx_skb[i] = NULL;

    for (i = 0; i < EC_TX_RING_SIZE; i++) {
        if (!(device->tx_skb[i] = dev_alloc_skb(ETH_FRAME_LEN))) {
            EC_NODE_ERR(ecat_node, "Error allocating device socket buffer!\n");
            ret = -ENOMEM;
            goto out_tx_ring;
        }

        // add Ethernet-II-header
        skb_reserve(device->tx_skb[i], ETH_HLEN);
        eth = (struct ethhdr *) skb_push(device->tx_skb[i], ETH_HLEN);
        eth->h_proto = htons(0x88A4);
        memset(eth->h_dest, 0xFF, ETH_ALEN);
    }

    ec_device_detach(device); // resets remaining fields
    return 0;

out_tx_ring:
    for (i = 0; i < EC_TX_RING_SIZE; i++)
        if (device->tx_skb[i])
            dev_kfree_skb(device->tx_skb[i]);
    return ret;
}


/*
 * scan network device list, find the interfaces by name and 
 *  grab the interface(s) ;
*/
int ec_net_init(e_slave* ecs, char* txdev, char *rxdev)
{ 

	ecs->intr[RX_INT_INDEX] = kmalloc(sizeof(struct ec_device), GFP_KERNEL);
	ec_device_init(ecs->intr[RX_INT_INDEX], ecs);

	ecs->intr[TX_INT_INDEX] = kmalloc(sizeof(struct ec_device) ,GFP_KERNEL);
	ec_device_init(ecs->intr[TX_INT_INDEX], ecs);
	return 0;
}

void ec_device_send(
        ec_device_t *device, /**< EtherCAT device */
        size_t size /**< number of bytes to send */
        )
{
    struct sk_buff *skb = device->tx_skb[device->tx_ring_index];

    // frame statistics
    if (unlikely(jiffies - device->stats_jiffies >= HZ)) {
        unsigned int i;
        u32 tx_frame_rate =
            (u32) (device->tx_count - device->last_tx_count) * 1000;
        u32 tx_byte_rate =
            (device->tx_bytes - device->last_tx_bytes);
        u64 loss = device->tx_count - device->rx_count;
        s32 loss_rate = (s32) (loss - device->last_loss) * 1000;
        for (i = 0; i < EC_RATE_COUNT; i++) {
            unsigned int n = rate_intervals[i];
            device->tx_frame_rates[i] =
                (device->tx_frame_rates[i] * (n - 1) + tx_frame_rate) / n;
            device->tx_byte_rates[i] =
                (device->tx_byte_rates[i] * (n - 1) + tx_byte_rate) / n;
            device->loss_rates[i] =
                (device->loss_rates[i] * (n - 1) + loss_rate) / n;
        }
        device->last_tx_count = device->tx_count;
        device->last_tx_bytes = device->tx_bytes;
        device->last_loss = loss;
        device->stats_jiffies = jiffies;
    }

    // set the right length for the data
    skb->len = ETH_HLEN + size;

    if (unlikely(device->ecat_node->debug_level > 1)) {
        EC_NODE_DBG(device->ecat_node, 2, "Sending frame:\n");
        ec_print_data(skb->data, ETH_HLEN + size);
    }

    // start sending
    if (device->dev->netdev_ops->ndo_start_xmit(skb, device->dev) ==
            NETDEV_TX_OK)
    {
        device->tx_count++;
        device->tx_bytes += ETH_HLEN + size;
    } else{
        device->tx_errors++;
    }
}

void tx_packet(uint8_t *buf, int size,struct ec_device *ecdev)
{
	ec_device_send(ecdev, size);
}

int ec_is_nic_link_up(e_slave *ecs,struct ec_device *intr)
{
	return intr->link_state;
}
