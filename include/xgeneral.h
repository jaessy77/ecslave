#define EC_MAX_PORTS 2

#ifdef __KERNEL__

#include <linux/if_ether.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/list.h>

struct ether_header
{
  u_int8_t  ether_dhost[ETH_ALEN];	/* destination eth addr	*/
  u_int8_t  ether_shost[ETH_ALEN];	/* source ether addr	*/
  u_int16_t ether_type;		        /* packet type ID field	*/
} __attribute__ ((__packed__));

#define xmalloc(size) 	kmalloc(size, GFP_KERNEL)
static inline int clock_gettime(int dummy __attribute__((unused)), struct timespec *sp)
{
	getnstimeofday(sp);
	return 0;
}

#define LIST_ENTRY(a) struct list_head 

#else

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/queue.h>

#define xmalloc(size)	malloc(size)
#define NSEC_PER_SEC (1000000000L)

struct semaphore { 
	sem_t sem;
};

#endif

