#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/****************************************************************************/
#include "ecrt.h"
#include "slaves.h"
/****************************************************************************/
#if __BYTE_ORDER == __LITTLE_ENDIAN
#pragma "using little endian"
#endif

// Application parameters
#define FREQUENCY 10
// Optional features

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define CLOCK_TO_USE CLOCK_REALTIME
#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
#define ARDUINO_HIGH	1
#define ARDUINO_LOW	0
//#define ARDUINO_PIN 	7 , 6 ,3, 2 ,8,9
#define ARDUINO_PIN 	9

/****************************************************************************/

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

const struct timespec cycletime = {0, PERIOD_NS};
/****************************************************************************/

// process data
static uint8_t *pd = NULL;

#define AnaInSlavePos1 0, 0

#define LIBIX_VP 0x000001ee, 0x0000000e /* LIBIX_VP = VENDOR PRODUCT */

// offsets for PDO entries

static unsigned int cmd_off = -1;
static unsigned int pin_off[21]={-1};

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, 0x14, &cmd_off},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, ARDUINO_PIN, &pin_off[0]},
    {}
};

static unsigned int counter = 0;
static unsigned int blink = 0;

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
	struct timespec result;

	if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
		result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
		result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
	} else {
		result.tv_sec = time1.tv_sec + time2.tv_sec;
		result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
	}

	return result;
}
/*****************************************************************************/

int config_slaves(void)
{
    ec_slave_config_t *sc1;
    ec_slave_config_t *sc2;
    
    master = ecrt_request_master(0);
    if (!master)
        return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    if (!(sc1 = ecrt_master_slave_config(
                    master, AnaInSlavePos1, LIBIX_VP))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }
    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(sc1, EC_END, slave_0_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
        return -1;
    }

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        fprintf(stderr, "PDO entry registration failed!\n");
        return -1;
    }

    ecrt_slave_config_dc(sc1, 0x0700, PERIOD_NS, 4400000, 0, 0);
    printf("Activating master...\n");
    if (ecrt_master_activate(master))
        return -1;

    if (!(pd = ecrt_domain_data(domain1))) {
        return -1;
    }

    printf("Offsets %d %d\n",
	cmd_off, pin_off[0]);

    return 0;
}

void check_domain1_state(void)
{
    ec_domain_state_t ds;

    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain1_state.working_counter)
        printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

/*****************************************************************************/

void check_master_state(void)
{
    ec_master_state_t ms;

    ecrt_master_state(master, &ms);

	if (ms.slaves_responding != master_state.slaves_responding)
        printf("%u slave(s).\n", ms.slaves_responding);
    if (ms.al_states != master_state.al_states)
        printf("AL states: 0x%02X.\n", ms.al_states);
    if (ms.link_up != master_state.link_up)
        printf("Link is %s.\n", ms.link_up ? "up" : "down");

    master_state = ms;
}

int toggle = 0;

void cyclic_task(void)
{
    int cmd = 0;
    struct timespec wakeupTime;

    // get current time
    clock_gettime(CLOCK_TO_USE, &wakeupTime);

    while(1) {
	wakeupTime = timespec_add(wakeupTime, cycletime);
       	clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wakeupTime, NULL);
	// receive process data
	ecrt_master_receive(master);
	ecrt_domain_process(domain1);
	// check process data state (optional)
	check_domain1_state();
	cmd = 1 << ARDUINO_PIN;
	memcpy(&pd[cmd_off], &cmd, 4);
	if ( toggle ){
		toggle = 0;
		pd[pin_off[0]] = ARDUINO_HIGH;
	} else{
		toggle = 1;
		pd[pin_off[0]] = ARDUINO_LOW;
	}
	// send process data
	ecrt_domain_queue(domain1);
	ecrt_master_send(master);
	}
}

/****************************************************************************/

int main(int argc, char **argv)
{
    printf("Using pin %d\n",ARDUINO_PIN);
    if (config_slaves())
	return -1;
    cyclic_task();
    return 0;
}

