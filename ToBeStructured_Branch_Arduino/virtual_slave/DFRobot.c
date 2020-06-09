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
#include "dfrslaves.h"
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

// http://www.dfrobot.com/wiki/index.php?title=DFRduino_Romeo-All_in_one_Controller_V1.1%28SKU:DFR0004%29
#define ARDUINO_E1	5
#define ARDUINO_E2	6
#define ARDUINO_M1	4
#define ARDUINO_M2	7

#define ARDUINO_HIGH	1
#define ARDUINO_LOW	0

#define ARDUINO_ANALOG_WRITE 0x1
#define ARDUINO_ANALOG_READ  0x2
#define ARDUINO_DIGITAL_WRITE 0x3
#define ARDUINO_DIGITAL_READ  0x4

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

#define ROBOT_ADVANCE	0x12
#define ROBOT_BACK	0x13
#define ROBOT_LEFT	0x14
#define ROBOT_RIGHT	0x15
#define ROBOT_STOP	0x16
//
// offsets for PDO entries
//
static unsigned int cmd_offset = -1;
static unsigned int e1_offset  = -1;
static unsigned int e2_offset  = -1;
static unsigned int m1_offset  = -1;
static unsigned int m2_offset  = -1;

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, 0x14, &cmd_offset},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, ARDUINO_E1, &e1_offset},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, ARDUINO_E2, &e2_offset},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, ARDUINO_M1, &m1_offset},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, ARDUINO_M2, &m2_offset},
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


static void stop(void)
{
	int cmd = ARDUINO_DIGITAL_WRITE;
	int val = ARDUINO_LOW;
	uint32_t t;

	t = cmd | (val << 8) ;
	memcpy(&pd[e1_offset],&t, 2);
	memcpy(&pd[e2_offset],&t, 2);
}

void fillData(int e1Val,int e2Val,int m1Val,int m2Val)
{
	uint32_t t;
	
	t = (ARDUINO_ANALOG_WRITE  | (e1Val << 8) ) & 0xFFFF;
	memcpy(&pd[e1_offset] ,&t,2); 

	t = (ARDUINO_DIGITAL_WRITE | (m1Val << 8) ) & 0xFFFF;
	memcpy(&pd[m1_offset], &t,2);

	t = (ARDUINO_ANALOG_WRITE  | (e2Val << 8) ) & 0xFFFF;
	memcpy(&pd[e2_offset] , &t,2 );

	t = (ARDUINO_DIGITAL_WRITE | (m2Val << 8) ) & 0xFFFF;
	memcpy(&pd[m2_offset] , &t,2);
}

static void advance(void)
{
	fillData(255, 255, ARDUINO_HIGH, ARDUINO_HIGH);
}

static void moveBackward(void)
{
	fillData(255, 255, ARDUINO_LOW, ARDUINO_HIGH);
}

static void turnLeft(void)
{
	fillData(100, 100, ARDUINO_LOW, ARDUINO_HIGH);
}

static void turnRight (void)
{
	fillData(100, 100, ARDUINO_HIGH, ARDUINO_LOW);
}

void cyclic_task(void)
{
    int count = 0;
    int action = ROBOT_ADVANCE;
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

	check_domain1_state();

	if (count++ == 10) {
		action = ROBOT_ADVANCE;
	}
	
	if (count == 20) {
		action = ROBOT_BACK;
	}
	if (count == 30) {
		action = ROBOT_RIGHT; 
	}
	if (count == 40) {
		action = ROBOT_LEFT;
	}
	if (count == 50) {
		action = ROBOT_STOP;
		count = 0;
	}

	cmd = 0;
	switch(action){
		case ROBOT_ADVANCE:
			advance();
			cmd = (1 << ARDUINO_E1) | (1 << ARDUINO_E2) | (1 << ARDUINO_M1) | (1 << ARDUINO_M2) ;
			break;

		case ROBOT_BACK:
			moveBackward();
			cmd = (1 << ARDUINO_E1) | (1 << ARDUINO_E2) | (1 << ARDUINO_M1) | (1 << ARDUINO_M2) ;
			break;

		case ROBOT_LEFT:
			turnLeft();
			cmd = (1 << ARDUINO_E1) | (1 << ARDUINO_E2) | (1 << ARDUINO_M1) | (1 << ARDUINO_M2) ;
			break;

		case ROBOT_RIGHT:
			turnRight();
			cmd = (1 << ARDUINO_E1) | (1 << ARDUINO_E2) | (1 << ARDUINO_M1) | (1 << ARDUINO_M2) ;
			break;

		case ROBOT_STOP:
			cmd = (1 << ARDUINO_E1) | (1 << ARDUINO_E2) ;
			stop();
			break;

		default: // do nothing
			break;
	}
	memcpy(&pd[cmd_offset], &cmd, 4);
	// send process data
	ecrt_domain_queue(domain1);
	ecrt_master_send(master);
	}
}

/****************************************************************************/

int main(int argc, char **argv)
{
    if (config_slaves())
	return -1;
    printf("offsets: %d %d %d %d %d\n", cmd_offset, e1_offset , e2_offset , m1_offset , m2_offset);
    cyclic_task();
    return 0;
}

