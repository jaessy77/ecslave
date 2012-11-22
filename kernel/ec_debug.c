#include "../include/xgeneral.h"

void ec_dump_string(uint8_t *c,int len)
{
	int i ;

	for  (i = 0 ; i < len ; i++){
		printk("%02X ", c[i]);
	}
}

void ec_printf(const char *str, ...){}
