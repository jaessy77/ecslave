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

extern "C" void ecat_rcv(ecat_slave  *ecs);

#define ARDUINO_ANALOG_WRITE 0x1
#define ARDUINO_ANALOG_READ  0x2
#define ARDUINO_DIGITAL_WRITE 0x3
#define ARDUINO_DIGITAL_READ  0x4

int E1 = 5;     //M1 Speed Control
int E2 = 6;     //M2 Speed Control
int M1 = 4;    //M1 Direction Control
int M2 = 7;    //M1 Direction Control
 
struct fsm_slave fsm_slave;
ecat_slave ecs;

void configurePins(void)
{
	pinMode(E1,OUTPUT);
	pinMode(M1,OUTPUT);
	pinMode(E2,OUTPUT);
	pinMode(M2,OUTPUT);
}

void arduinoAction(int pdoe,uint32_t value)
{
	uint8_t cmd = value  & 0x00FF;
	uint8_t val = (value & 0xFF00) >> 8;

	switch(cmd) {
		case ARDUINO_ANALOG_WRITE:
			analogWrite(pdoe, val);
			break;
		case ARDUINO_DIGITAL_WRITE:
			digitalWrite(pdoe, val);
			break;
	}

} 

void setup()
{
	int i = 0;
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

	if (init_process_data(&ecs) < 0) {
		return;
	}
	ecs.fsm = &fsm_slave;
	ecs.dgram_processed = 0;
	ecs.dgrams_cnt = 0;
	configurePins();
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
	uint32_t* pdos = (uint32_t *)data;

	memcpy( (void *)&cmd, (void *)&pdos[CMD_PDO_IDX], 4);

	for (;pdoe_idx < NR_PDOS; pdoe_idx++) {
		should_act = cmd & (1 << pdoe_idx);
		if (!should_act) {
			continue;
		}
		arduinoAction(pdoe_idx, pdos[pdoe_idx]);
	}
	return 0;
}
