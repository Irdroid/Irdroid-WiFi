#ifndef PIPE_H
#define PIPE_H

#include "sio.h"
#include "sock.h"
#include "thread.h"

typedef struct
{
	/* Serial port */
	sio_s sio;

	/* Socket */
	tcpsock_s sock;

	/* Mutex lock */
	thr_mutex_t *mutex;
	
	/* Time out in seconds */
	int timeout;

	/* Protocol (raw, http, websocket) */
	int net_protocol;

} pipe_s;

int pipe_init(pipe_s *pipe, int sock_port);
void pipe_cleanup(pipe_s *pipe);
void pipe_destroy(void *data);

#endif	/* PIPE_H */
