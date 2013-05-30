#include "../include/xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_regs.h"
#include "ec_coe.h"
#include "ec_coe_sdo_mapping.h"


#define NR_SDOS 2
#define SDO1 0x1234
#define SDO2 0x5678

void ec_populate_sdos(coe_sdo_service_data *srvdata)
{
	srvdata->index[0] = SDO1;
	srvdata->index[1] = SDO2;
	srvdata->index[2] = 0x0;
}

int ec_nr_sdos(void)
{
	return NR_SDOS;
}

void ec_sdo_action(uint16_t index, uint16_t subindex)
{
	switch (index) {
	case SDO1:
		break;
	case SDO2:
		break;
	default:
		break;
	}
}
