/*
 * config.c
 *
 * Configuration module
 *
 * (C)1999 serproxy 0.1.2, Stefano Busti
 * (C)2011-2012 serwebproxy 1.1, Lari Temmes
 * 
 * Changes serproxy 0.1.2 -> serwebproxy 1.0
 *	Added net_websocket config option
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "string.h"

cfg_item_i cfg_int_defs[CFG_IEND] = {
	{"comm_baud", 9600},
	{"comm_databits", 8},
	{"comm_stopbits", 1},
	{"comm_port", 30},
	{"net_port", 5331},
	{"timeout", 300},
	{"net_protocol", 1}
};

cfg_item_s cfg_str_defs[CFG_SEND] = {
	{"comm_parity", "none"},
	{"serial_device", "/dev/ttyS"},
	{"net_allow", "all"},
	{"net_deny", "none"}
};

int cfg_init(cfg_s *cfg, int comm_port)
{
	int i;
	char buf[BUFSIZ];

	sprintf(buf, "%d", comm_port);
		
	for (i = 0; i < CFG_IEND; i++)
	{
		cfg->ints[i].key = NULL;
		str_assign(&cfg->ints[i].key, cfg_int_defs[i].key);

		if (comm_port > 0)
			str_cat(&cfg->ints[i].key, buf);
		
		cfg->ints[i].val = cfg_int_defs[i].val;
	}

	for (i = 0; i < CFG_SEND; i++)
	{
		cfg->strs[i].key = NULL;
		str_assign(&cfg->strs[i].key, cfg_str_defs[i].key);

		if (comm_port > 0)
			str_cat(&cfg->strs[i].key, buf);
		
		cfg->strs[i].val = NULL;;
		str_assign(&cfg->strs[i].val, cfg_str_defs[i].val);		
	}
	
	return 0;
}

void cfg_cleanup(cfg_s *cfg)
{
	int i;

	for (i = 0; i < CFG_IEND; i++)
	{
		str_cleanup(&cfg->ints[i].key);
	}

	for (i = 0; i < CFG_SEND; i++)
	{
		str_cleanup(&cfg->strs[i].key);
		str_cleanup(&cfg->strs[i].val);		
	}
}

int cfg_fromfile(cfg_s *cfg, char *filename)
{
	int i, port;

	port = cfg->ints[CFG_IPORT].val;

	/* Read integers */
	for (i = 0; i < CFG_IEND; i++)
		cfg_readint(filename, &cfg->ints[i], cfg->ints[i].val); 

	/* Read strings */
	for (i = 0; i < CFG_SEND; i++)
		cfg_readstr(filename, &cfg->strs[i], cfg->strs[i].val); 
	
	return 0;
}

void cfg_assign(cfg_s *dst, cfg_s *src)
{
	int i;

	for (i = 0; i < CFG_IEND; i++)
		dst->ints[i].val = src->ints[i].val;

	for (i = 0; i < CFG_SEND; i++)
		str_assign(&dst->strs[i].val, src->strs[i].val);		
}
