#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include "ec_sii.h"

/** Configured Address Write */
void ec_cmd_fpwr(e_slave *ecs,uint8_t *dgram_ec)
{
	uint16_t ado = 0;
	uint16_t adp = 0;
	uint16_t datalen = __ec_dgram_dlength(dgram_ec);
	uint8_t *data    = __ec_dgram_data(dgram_ec);

	ado = __ec_dgram_ado(dgram_ec);
	adp = __ec_dgram_adp(dgram_ec);

	ec_printf("%s ado=0x%x adp=0x%x "
			"station addr=0x%x datalen=%d\n",
	       __FUNCTION__, 
	       ado,adp,
	       ec_station_address(),
	       datalen);
	if (datalen == 0) {
		printf("insane no length\n");
		goto FPRD_OUT;
	}
	if (adp != ec_station_address())
		goto FPRD_OUT;
	__ec_inc_wkc__(dgram_ec);
	if (ado == ECT_REG_EEPSTAT){
		// ec_fsm_sii_state_start_reading
		ec_sii_start_read(data, datalen);
	} else{
		ec_raw_set_ado(ecs,ado, data, datalen);
	}

FPRD_OUT:
        __set_fsm_state(ecs, ecs_process_next_dgram);
}
