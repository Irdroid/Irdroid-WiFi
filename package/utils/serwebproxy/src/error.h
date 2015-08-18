#ifndef MYERROR_H
#define MYERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

void error(const char *fmt, ...);
void errend(const char *fmt, ...);
void perror2(const char *fmt, ...);
void perrend(const char *fmt, ...);
void dbg_msg(const char *fmt, ...);

#ifdef DEBUG
#define DBG_MSG1(x) dbg_msg((x))
#define DBG_MSG2(x,y) dbg_msg((x),(y))
#define DBG_MSG3(x,y,z) dbg_msg((x),(y),(z))
#else
#define DBG_MSG1(x) ;
#define DBG_MSG2(x,y) ;
#define DBG_MSG3(x,y,z) ;
#endif


#endif	/* MYERROR_H */
