#include <Arduino.h>
#include "xgeneral.h"
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"
#include "ec_cmd.h"

struct fsm_slave fsm_slave;
ecat_slave ecs;

void ecat_rcv(ecat_slave  *ecs);

void setup()
{
	int ret;
        Serial.begin(9600);
        Serial.println("Arduino Ethercat slave"); 
 
	ecs.fsm = 0; /* act as flag */
	ret = ecs_net_init(0 , 0, &ecs);
	if (ret < 0) {
		Serial.print("Error init network");
		Serial.println(ret);
		return;
	}
	ret  = ec_init_regs(&ecs) ;
	if (ret < 0){
		Serial.println("Error init registers");
		Serial.println(ret);
		return;
	}

	init_sii(&ecs);
	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(9, OUTPUT);
	if (init_process_data(&ecs) < 0) {
		Serial.println("illegal pdo configuration\n");
		return;
	}
	ecs.fsm = &fsm_slave;
	ecs.dgram_processed = 0;
	ecs.dgrams_cnt = 0;
}

void loop()
{
	if (ecs.fsm)
		ecat_rcv(&ecs);
}

/*
 * process data  operations for Arduino UNO only
 *  Each pin may be used for a dedicated  purpose.
 * 
*/
extern "C" int set_process_data(uint8_t *data, uint16_t offset, uint16_t datalen)
{
	int pdoe_idx = 0;
	uint32_t cmd ; 
	int should_act = 0;

	memcpy( (void *)&cmd, (void *)&data[CMD_PDO_IDX], 4);

	for (;pdoe_idx < NR_PDOS; pdoe_idx++) {
		should_act = cmd & (1 << pdoe_idx);
		if (!should_act) {
			continue;
		}
		Serial.print("<");
		Serial.print(data[pdoe_idx]);
		Serial.print(":");
		Serial.print(pdoe_idx);
		Serial.println(">");
		digitalWrite(pdoe_idx, data[pdoe_idx]);
	}
	return 0;
}
