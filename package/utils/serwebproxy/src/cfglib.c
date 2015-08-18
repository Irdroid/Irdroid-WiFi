/*
 * cfglib.c
 *
 * Config file parsing routines
 *
 * (C)1999 Stefano Busti
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfglib.h"

char *cfg_getvalue(const char *key, const char *src);


int cfg_readstr(const char *filename,
					 cfg_item_s *item,
					 const char *deflt)
{
	char buf[BUFSIZ];

	if (cfg_readbuf(filename, item->key, buf, sizeof(buf)) == NULL) {
		item->val = strealloc(item->val, deflt);
		return -1;
	}

	item->val = strealloc(item->val, buf);
	return 0;
}


int cfg_readint(const char *filename,
					 cfg_item_i *item,
					 int deflt)
{
	char buf[BUFSIZ], *perr;

	if (cfg_readbuf(filename, item->key, buf, sizeof(buf)) == NULL) {
		item->val = deflt;
		return -1;
	}
	
	item->val = strtol(buf, &perr, 0);

	if ((*perr != '\0') || (*buf == '\0')) {
		item->val = deflt;
		return -1;
	}

	return 0;
}

char *cfg_readbuf(const char *filename,
					 const char *key,
					 char *value,
					 size_t valuesiz)
{
	char *ptr;
	int len;

	FILE *f = fopen(filename, "rt");

	if (!f)
		return NULL;
	
	while (fgets(value, valuesiz, f))
	{
		len = strlen(value);

		/* strip crs */
		if ((len > 0) && (value[len - 1] == '\n')) {
			value[len - 1] = '\0';
			len--;
		}

		/* ignore blank lines and comments */
		if ((len > 0) && (value[0] != '#')) {
			if ((ptr = cfg_getvalue(key, value)) != NULL) {
				memmove(value, ptr, strlen(ptr) + 1);
				fclose(f);
				return value;
			}
		}
	}
	fclose(f);
	return NULL;
}

char *cfg_getvalue(const char *key, const char *src)
{
	char *peql;
	
	if ((peql = strchr(src, '=')) != NULL) { 
		int keylen = strlen(key);

		if ((keylen == peql - src) && (strncmp(key, src, keylen) == 0)) 
			return peql + 1;
	}

	return NULL;
}

char *strealloc(char *dest, const char *s)
{
	/* 
	 * Don't reallocate if source and destination
	 * point to the same string, otherwise s becomes invalid.
	 */
	if (dest == s)
		return dest;
		
	dest = realloc(dest, strlen(s) + 1);
	if (dest == NULL)
		return NULL;

	strcpy(dest, s);
	return dest;
}

void strdispose(char *dest)
{
	free(dest);
}
