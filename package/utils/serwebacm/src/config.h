#include "cfglib.h"

typedef enum
{
	CFG_IBAUD,
	CFG_IDATA,
	CFG_ISTOP,
	CFG_IPORT,
	CFG_INETPORT,
	CFG_ITIMEOUT,
	CFG_INETPROTOCOL,
	CFG_IEND
} cfg_int_t;

typedef enum
{
	CFG_SPARITY,
	CFG_SERIAL_DEVICE,
	CFG_SALLOW,
	CFG_SDENY,
	CFG_SEND
} cfg_str_t;

typedef struct
{
	cfg_item_i ints[CFG_IEND];
	cfg_item_s strs[CFG_SEND];
} cfg_s;

int cfg_init(cfg_s *cfg, int comm_port);
void cfg_cleanup(cfg_s *cfg);
int cfg_fromfile(cfg_s *cfg, char *filename);
void cfg_assign(cfg_s *dst, cfg_s *src);
