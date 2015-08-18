#ifndef STRING_H
#define STRING_H

int str_assign(char **pstr, char *text);
int str_cat(char **pstr, char *text);
int str_striptrail(char *str);
void str_cleanup(char **pstr);

#endif	/* STRING_H */
