#ifndef CFGLIB_H
#define CFGLIB_H

#include <stdlib.h>

typedef struct
{
	char *key;
	char *val;
} cfg_item_s;

typedef struct
{
	char *key;
	int val;
} cfg_item_i;

int cfg_readstr(const char *filename,
					 cfg_item_s *item,
					 const char *deflt);

int cfg_readint(const char *filename,
					 cfg_item_i *item,
					 int deflt);

char *cfg_readbuf(const char *filename,
						const char *key,
						char *value,
						size_t valuesiz);

char *strealloc(char *dest, const char *s);
void strdispose(char *dest);

#endif /* CFGLIB_H */
