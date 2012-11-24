/******************************************************************************
 *
 *  $Id: device.c,v f461dc0d145a 2010/07/31 14:45:10 fp $
 *
 *  Copyright (C) 2006-2008  Florian Pose, Ingenieurgemeinschaft IgH
 *
 *  This file is part of the IgH EtherCAT Master.
 *
 *  The IgH EtherCAT Master is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License version 2, as
 *  published by the Free Software Foundation.
 *
 *  The IgH EtherCAT Master is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the IgH EtherCAT Master; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 *****************************************************************************/

/**
   \file
   EtherCAT device methods.
*/

/*****************************************************************************/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include "ethercattype.h"


#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_device.h"
#include "ec_debug.h"

#ifdef EC_DEBUG_RING
#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif

/** List of intervals for frame statistics [s].
 */
static const unsigned int rate_intervals[] = {
    1, 10, 60
};

/*****************************************************************************/

/** Constructor.
 * 
 * \return 0 in case of success, else < 0
 */
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

/*****************************************************************************/

/** Destructor.
 */
void ec_device_clear(
        ec_device_t *device /**< EtherCAT device */
        )
{
    unsigned int i;

    if (device->open) ec_device_close(device);
    for (i = 0; i < EC_TX_RING_SIZE; i++)
        dev_kfree_skb(device->tx_skb[i]);
}

/*****************************************************************************/

/** Associate with net_device.
 */
void ec_device_attach(
        ec_device_t *device, /**< EtherCAT device */
        struct net_device *net_dev, /**< net_device structure */
        ec_pollfunc_t poll, /**< pointer to device's poll function */
        struct module *module /**< the device's module */
        )
{
    unsigned int i;
    struct ethhdr *eth;

    ec_device_detach(device); // resets fields

    device->dev = net_dev;
    device->poll = poll;
    device->module = module;

    for (i = 0; i < EC_TX_RING_SIZE; i++) {
        device->tx_skb[i]->dev = net_dev;
        eth = (struct ethhdr *) (device->tx_skb[i]->data);
        memcpy(eth->h_source, net_dev->dev_addr, ETH_ALEN);
    }

#ifdef EC_DEBUG_IF
    ec_debug_register(&device->dbg, net_dev);
#endif
}

/*****************************************************************************/

/** Disconnect from net_device.
 */
void ec_device_detach(
        ec_device_t *device /**< EtherCAT device */
        )
{
    unsigned int i;

#ifdef EC_DEBUG_IF
    ec_debug_unregister(&device->dbg);
#endif

    device->dev = NULL;
    device->poll = NULL;
    device->module = NULL;
    device->open = 0;
    device->link_state = 0; // down

    ec_device_clear_stats(device);

    for (i = 0; i < EC_TX_RING_SIZE; i++)
        device->tx_skb[i]->dev = NULL;
}

/*****************************************************************************/

/** Opens the EtherCAT device.
 *
 * \return 0 in case of success, else < 0
 */
int ec_device_open(
        ec_device_t *device /**< EtherCAT device */
        )
{
    int ret;

    if (!device->dev) {
        EC_NODE_ERR(device->ecat_node, "No net_device to open!\n");
        return -ENODEV;
    }

    if (device->open) {
        EC_NODE_WARN(device->ecat_node, "Device already opened!\n");
        return 0;
    }

    device->link_state = 0;

    ec_device_clear_stats(device);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
    ret = device->dev->netdev_ops->ndo_open(device->dev);
#else
    ret = device->dev->open(device->dev);
#endif
    if (!ret)
        device->open = 1;

    return ret;
}

/*****************************************************************************/

/** Stops the EtherCAT device.
 *
 * \return 0 in case of success, else < 0
 */
int ec_device_close(
        ec_device_t *device /**< EtherCAT device */
        )
{
    int ret;

    if (!device->dev) {
        EC_NODE_ERR(device->ecat_node, "No device to close!\n");
        return -ENODEV;
    }

    if (!device->open) {
        EC_NODE_WARN(device->ecat_node, "Device already closed!\n");
        return 0;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
    ret = device->dev->netdev_ops->ndo_stop(device->dev);
#else
    ret = device->dev->stop(device->dev);
#endif
    if (!ret)
        device->open = 0;

    return ret;
}

/*****************************************************************************/

/** Returns a pointer to the device's transmit memory.
 *
 * \return pointer to the TX socket buffer
 */
uint8_t *ec_device_tx_data(
        ec_device_t *device /**< EtherCAT device */
        )
{
    /* cycle through socket buffers, because otherwise there is a race
     * condition, if multiple frames are sent and the DMA is not scheduled in
     * between. */
    device->tx_ring_index++;
    device->tx_ring_index %= EC_TX_RING_SIZE;
    return device->tx_skb[device->tx_ring_index]->data + ETH_HLEN;
}

/*****************************************************************************/

/** Sends the content of the transmit socket buffer.
 *
 * Cuts the socket buffer content to the (now known) size, and calls the
 * start_xmit() function of the assigned net_device.
 */
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
    if (device->dev->netdev_ops->ndo_start_xmit(skb, device->dev) ==
            NETDEV_TX_OK)
#else
    if (device->dev->hard_start_xmit(skb, device->dev) == NETDEV_TX_OK)
#endif
    {
        device->tx_count++;
        device->tx_bytes += ETH_HLEN + size;
#ifdef EC_DEBUG_IF
        ec_debug_send(&device->dbg, skb->data, ETH_HLEN + size);
#endif
#ifdef EC_DEBUG_RING
        ec_device_debug_ring_append(
                device, TX, skb->data + ETH_HLEN, size);
#endif
    } else {
        device->tx_errors++;
    }
}

/*****************************************************************************/

/** Clears the frame statistics.
 */
void ec_device_clear_stats(
        ec_device_t *device /**< EtherCAT device */
        )
{
    unsigned int i;

    // zero frame statistics
    device->tx_count = 0;
    device->rx_count = 0;
    device->tx_errors = 0;
    device->tx_bytes = 0;
    device->last_tx_count = 0;
    device->last_tx_bytes = 0;
    device->last_loss = 0;
    for (i = 0; i < EC_RATE_COUNT; i++) {
        device->tx_frame_rates[i] = 0;
        device->tx_byte_rates[i] = 0;
        device->loss_rates[i] = 0;
    }
}

/*****************************************************************************/
