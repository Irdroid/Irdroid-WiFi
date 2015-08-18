/*
 * main.c
 *
 * main module for serwebproxy
*/ 
/*  serwebproxy 1.12, Lari Temmes
	Copyright (C) 2011-2012 Lari Temmes

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 */
 /*
 * Original codebase from serproxy 0.1.2 by Stefano Busti, see README for more 
 * information.
 * 
 * Changes serproxy 0.1.2 -> serwebproxy 1.0
 *	Added support for WebSocket protocol (on by default, disabled from config file)
 *  Some fixes to make it compile for WIN64
 * Changes serwebproxy 1.0 -> serwebproxy 1.1
 *  Added support for HTTP protocol (GET requests)
  * Changes serwebproxy 1.0 -> serwebproxy 1.12
 *  Added support for websocket close frame (sent by browser on ws.close())
 */

#define DEBUG
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#if defined(__UNIX__)
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/time.h>
#elif defined(__WIN32__)
#  include <windows.h>
#  include <io.h>
#else
#  include <windows.h>
#  include <io.h>
#endif

#include "sio.h"
#include "sock.h"
#include "http.h"
#include "websocket.h"
#include "pipe.h"
#include "thread.h"
#include "vlist.h"
#include "cfglib.h"
#include "config.h"
#include "error.h"

int readcfg(void);
void cleanup(void);
int waitclients(void);
thr_startfunc_t serve_pipe(void *data);
void debug(void);

#if defined(__UNIX__)
char cfgfile[] = "/etc/serweb-usb.cfg";
#elif defined(__WIN32__)
char cfgfile[] = "serweb-usb.cfg";
#else
char cfgfile[] = "serweb-usb.cfg";
#endif

cfg_s cfg;
vlist_s pipes;

int main(int argc, char **argv)
{
	if (sock_start())
		return -1;
	vlist_init(&pipes, pipe_destroy);
	cfg_init(&cfg, 0);
	atexit(cleanup);
	readcfg();
#ifdef DEBUG
	debug();
#endif
	waitclients();
	
	return 0;
}

void cleanup(void)
{
	cfg_cleanup(&cfg);
	vlist_cleanup(&pipes);
	sock_finish();
}

int readcfg(void)
{
	char ports[BUFSIZ], *p;
	int port;
	pipe_s *pipe;
	cfg_s local;
	serialinfo_s sinfo;
	char *parity;
	
	/* Read the global config settings */
	cfg_fromfile(&cfg, cfgfile);
	
	/* Read the comm port list */
	if (cfg_readbuf(cfgfile, "comm_ports", ports, sizeof(ports)) == NULL)
		errend("Couldn't find 'comm_ports' entry in config file '%s'", cfgfile);

	vlist_clear(&pipes);

	/* Parse the comm ports list */
	p = strtok(ports, ",");
	while (p)
	{
		if (sscanf(p, "%d", &port) > 0)
		{
			pipe = malloc(sizeof(pipe_s));
			//pipe_init(pipe);
			if (pipe == NULL)
				perrend("malloc(pipe_s)");

			cfg_init(&local, port);
			
			/* Copy global settings to those for current pipe */
			cfg_assign(&local, &cfg);

			/* Set the comm port */
			local.ints[CFG_IPORT].val = port;

			/* Load this pipe's config */
			cfg_fromfile(&local, cfgfile);

			/* Try initializing the pipe */
			if (pipe_init(pipe, local.ints[CFG_INETPORT].val))
				perrend("pipe_init");

			/* Copy over the rest of the pipe's config */
			pipe->timeout = local.ints[CFG_ITIMEOUT].val;
			sinfo.port = port;
			sinfo.baud = local.ints[CFG_IBAUD].val;
			sinfo.stopbits = local.ints[CFG_ISTOP].val;
			sinfo.databits = local.ints[CFG_IDATA].val;
			//sinfo.serial_device = local.strs[CFG_SERIAL_DEVICE].val;
			memcpy( sinfo.serial_device, local.strs[CFG_SERIAL_DEVICE].val, strlen(local.strs[CFG_SERIAL_DEVICE].val) );

			/* Use websocket protocol for this pipe? */
			pipe->net_protocol = local.ints[CFG_INETPROTOCOL].val;

			parity = local.strs[CFG_SPARITY].val;

			if (strcmp(parity, "none") == 0)
			{
				sinfo.parity = SIO_PARITY_NONE;
			}
			else if (strcmp(parity, "even") == 0)
			{
				sinfo.parity = SIO_PARITY_EVEN;
			}
			else if (strcmp(parity, "odd") == 0)
			{
				sinfo.parity = SIO_PARITY_ODD;
			}
			else
			{
				errend("Unknown parity string '%s'", parity);
			}

			if (sio_setinfo(&pipe->sio, &sinfo))
				errend("Unable to configure comm port %d", port);
			
			/* Finally add the pipe to the pipes list */
			vlist_add(&pipes, pipes.tail, pipe);

			cfg_cleanup(&local);
		}
		
		p = strtok(NULL, ",");
	}

	/* Clean up local cfg struct */
	cfg_cleanup(&local);
	
	return 0;
}

int waitclients(void)
{
	int fd_max;
	fd_set fds;
	vlist_i *it;
	pipe_s *pit, *newpipe;
	tcpsock_s *newsock;
	thr_t thread;
	
	fd_max = 0;
	FD_ZERO(&fds);

	/* Set all sockets to listen */
	for (it = pipes.head; it; it = it->next)
	{
		pit = (pipe_s *)it->data;

		if (tcp_listen(&pit->sock))
			perror("waitclients() - tcp_listen()");
	}

	printf("serwebproxy 1.12 Copyright (C) 2012 Lari Temmes / lari.temmes@gmail.com (GPL2 license).\n");
	
	while (1)
	{
		/* Iterate through the pipe list */
		for (it = pipes.head; it; it = it->next)
		{
			pit = (pipe_s *)it->data;

			/* Monitor socket fd of each */
			FD_SET(pit->sock.fd, &fds);

			/* Track max fd */
			if (pit->sock.fd > fd_max)
				fd_max = pit->sock.fd;
		}

		/* Wait for a read ( == accept() in this case) */
		if (select(fd_max + 1, &fds, NULL, NULL, NULL) == -1)
			perrend("waitclients() - select()");


		/* Find which sockets are involved */
		for (it = pipes.head; it; it = it->next)
		{
			pit = (pipe_s *)it->data;

			if (FD_ISSET(pit->sock.fd, &fds))
			{
				/* Create a new pipe struct for the new thread */
				newpipe = malloc(sizeof(pipe_s));
				if (!newpipe)
					perrend("waitclients() - malloc(pipe_s)");

				newpipe->sio = pit->sio;

				/* Try to open serial port */
				if (sio_open(&newpipe->sio))
				{
					tcp_refuse(&pit->sock);
					error("Failed to open comm port - connection refused");
					free(newpipe);
					continue;
				}
				
				/* Accept the connection */
				newsock = tcp_accept(&pit->sock);

				/* All ok? */
				if (newsock)
				{
					newpipe->sock = *newsock;
					free(newsock);

					newpipe->timeout = pit->timeout;
					newpipe->net_protocol = pit->net_protocol;
					newpipe->mutex = pit->mutex;
						
					/* Create the server thread */
					if (thr_create(&thread, 1, serve_pipe, newpipe))
					{
						error("thread creation failed");
						free(newpipe);
					}
					else
					{
						//fprintf(stderr, "Server thread launched\n");
					}
				}
				else
				{
					perror("waitclients() - accept()");
					free(newpipe);
				}
			}
		}
	}
	return 0;
}

/* Main routine for the server threads */
thr_startfunc_t serve_pipe(void *data)
{
	char sio_buf[BUFSIZ], sock_buf[BUFSIZ];
	char out_buf[BUFSIZ+2];
	char misc_buf[BUFSIZ];
	int fd_max, sock_fd;
	int sio_count, sock_count;
	int res, port;	
	int websocket_state = 0;		// keeps track of websocket state
	int http_state = 0;				// keeps track of http state
	fd_set rfds, wfds;
	pipe_s *pipe = (pipe_s *)data;
#if defined(__UNIX__)
	struct timeval tv = {pipe->timeout, 0};
	struct timeval *ptv = &tv;
	int	sio_fd;
#elif defined(__WIN32__)
	struct timeval tv = {0,10000};
	struct timeval *ptv = &tv;
	DWORD msecs = 0, timeout = pipe->timeout * 1000;
	HANDLE sio_fd;
#else
	struct timeval tv = {0,10000};
	struct timeval *ptv = &tv;
	DWORD msecs = 0, timeout = pipe->timeout * 1000;
	HANDLE sio_fd;
#endif


	if( pipe->net_protocol == 0 ){
			// raw tcp socket, no headers of handshakes
			websocket_state = 10; 
			http_state = 10;	
	}else if( pipe->net_protocol == 1 ){
			// websocket
			websocket_state = 0;	
			http_state = 10;		
	}else{
			// http
			websocket_state = 10;	
			http_state = 0;		
	}

	// avoid junk

	memset((void *)sock_buf, 0, sizeof(sock_buf) );
	memset((void *)sio_buf, 0, sizeof(sio_buf) );
	memset((void *)out_buf, 0, sizeof(out_buf) );
	memset((void *)misc_buf, 0, sizeof(misc_buf) );

	port = pipe->sio.info.port;

	fprintf(stderr, "server(%d), using protocol %d\n", port, pipe->net_protocol);

	/* Only proceed if we can lock the mutex */
	if (thr_mutex_trylock(pipe->mutex))
	{
		error("server(%d) - resource is locked", port);
	}
	else
	{

		sio_count = 0;
		sock_count = 0;
		sio_fd = pipe->sio.fd;
		sock_fd = pipe->sock.fd;
#if defined(__UNIX__)
		fd_max = sio_fd > sock_fd ? sio_fd : sock_fd;	
#elif defined(__WIN32__)
		fd_max = sock_fd;
		msecs = GetTickCount();
#else
		fd_max = sock_fd;
		msecs = GetTickCount();
#endif
		fprintf(stderr, "server(%d) - thread started\n", port);
		
		while (1)
		{
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);

#if defined(__UNIX__)
			/* Always ask for read notification to check for EOF */			
			FD_SET(sio_fd, &rfds);
			/* Only ask for write notification if we have something to write */
			if (sock_count > 0)
				FD_SET(sio_fd, &wfds);

			/* Reset timeout values */
			tv.tv_sec = pipe->timeout;
			tv.tv_usec = 0;

#endif
			/* Always ask for read notification to check for EOF */
			FD_SET(sock_fd, &rfds);
			/* Only ask for write notification if we have something to write */
			if (sio_count > 0)
				FD_SET(sock_fd, &wfds);

			//DBG_MSG2("server(%d) waiting for events", port);
			
			/* Wait for read/write events */
			res = select(fd_max + 1, &rfds, &wfds, NULL, ptv);
			if (res == -1)
			{
				perror2("server(%d) - select()", port);
				break;
			}
#if defined(__UNIX__)

			/* Use the select result for timeout detection */
			if (res == 0)
			{
				fprintf(stderr, "server(%d) - timed out\n", port);
				break;
			}

			/* Input from serial port? */
			if (FD_ISSET(sio_fd, &rfds))
#elif defined(__WIN32__)
				
			if (1)
#else
			if (1)
#endif
			{
				/* Only read input if buffer is empty */
				if (sio_count == 0)
				{
					sio_count = sio_read(&pipe->sio, sio_buf, sizeof(sio_buf) );

					if (sio_count <= 0)
					{
						if (sio_count == 0)
						{
#if defined(__UNIX__)
							fprintf(stderr, "server(%d) - EOF from sio\n", port);
							break;
#endif
						}
						else
						{
							perror2("server(%d) - read(sio) count %d", port, sio_count);
							break;
						}
					}
					else 
					{
						DBG_MSG3("server(%d) - read %d bytes from sio", port, sio_count);
						
					}
				}
			}

			if( websocket_state == 1 ){

					// we have header from client, send our key and headers to client.
					
					sio_count = websocket_generate_headers( sio_buf );
					websocket_state = 2;

			}

			/* Write to socket possible? */
			if (FD_ISSET(sock_fd, &wfds))
			{



			

	if (sio_count > 0)
				{
					if( websocket_state == 2 ){
						
						// websocket handshake done, send data packet to socket with needed extra bytes

						memset( out_buf,0,sizeof(out_buf) );
						memcpy( out_buf+2, sio_buf, sio_count );
						out_buf[0] = 0x81;
						out_buf[1] = sio_count;
						// fprintf( stderr, "%s\n", out_buf+2 );
						if ((res = tcp_write(&pipe->sock, out_buf, sio_count+2)) < 0)
						{
							perror2("server(%d) - write(sock)", port);
							break;
						}
						sio_count -= res-2;

					}else if( http_state == 2 ){

						// http connection ready and headers sent, wait for end of line and send HTTP response to client

						memset( out_buf,0,sizeof(out_buf) );

						if ((res = tcp_write(&pipe->sock, sio_buf, sio_count )) < 0)
						{
							perror2("server(%d) - http data sent(sock)", port);
							break;
						}// else fprintf(stderr, "server(%d) - data %s\n", port,sio_buf);
						
						if( strchr( sio_buf, '\n' )  ){ sio_count = 0; break; }
						// else fprintf(stderr, "server(%d) - waiting end of line\n", port );				
					
						sio_count -= res;
						
					}else{
						// send raw data
						if ((res = tcp_write(&pipe->sock, sio_buf, sio_count)) < 0)
						{
							perror2("server(%d) - write(sock)", port);
							break;
						}
						sio_count -= res;
					}
					DBG_MSG3("server(%d) - Wrote %d bytes to sock", port, res);
				
				}

					

			}


			/* Input from socket? */
			if (FD_ISSET(sock_fd, &rfds))
			{
				/* Only read input if buffer is empty */
				if (sock_count == 0)
				{
					sock_count = tcp_read(&pipe->sock, sock_buf, sizeof(sock_buf));
					if (sock_count <= 0)
					{
						if (sock_count == 0)
						{
							fprintf(stderr, "server(%d) - EOF from sock\n", port);
							break;
						}
						else
						{
							//perror2("server(%d) - read(sock)", port);
							break;
						}
					}
					
					DBG_MSG3("server(%d) - read %d bytes from sock", port, sock_count );
					FD_SET(sock_fd, &wfds);
					
				

					if( websocket_state == 0 ){

						websocket_generate_key( sock_buf );
						
						// now that we have key, discard header. device in serial port is not interested of it
						memset( sock_buf, 0, sizeof(sock_buf) );
						sock_count = 0;

						websocket_state = 1;

					}else if( http_state == 0 ){
						
						DBG_MSG2("server(%), http request from client\n", port);

						http_generate_headers( out_buf, 200, "OK", -1 ); 
						
						if ((res = tcp_write(&pipe->sock, out_buf, strlen(out_buf) )) < 0)
						{
							perror2("server(%d) - header write(sock)", port);
							break;
						}// else fprintf(stderr, "server(%d) - header %s\n", port,out_buf);

						// handle request parameters
						memset( misc_buf, 0, sizeof(misc_buf) );

						res = http_handle_get( sock_buf, misc_buf );
						if( res == 0 ){	// favicon request or similar, we ignore that
							fprintf(stderr, "server(%d), ignoring get %s\n", port, misc_buf );
							memset( sock_buf, 0, sizeof(sock_buf) );
							sock_count = 0; sio_count = 0;
							break;
						}else{
							fprintf(stderr, "server(%d), get %s\n", port, misc_buf );
							memset( sock_buf, 0, sizeof(sock_buf) );
							memcpy( sock_buf, misc_buf, strlen(misc_buf) );
							sock_count = strlen( misc_buf );
						}
						http_state = 1;
					
					}
					if( websocket_state == 1 ){

					// we have header from client, send our key and headers to client.
					
					sio_count = websocket_generate_headers( sio_buf );
					websocket_state = 2;

			}
				}else fprintf(stderr, "socket blocked\n");
			}

#if defined(__UNIX__)
			/* Write to serial port possible? */
			if (FD_ISSET(sio_fd, &wfds))
#else
			
			/* No socket IO performed? */
			if ((!FD_ISSET(sock_fd, &rfds)) && (!FD_ISSET(sock_fd, &wfds)))
			{
				/* Break on a time out */
				if (GetTickCount() - msecs > timeout)
				{
					fprintf(stderr, "server(%d) - timed out\n", port);
					break;					
				}
			}
			else
			{
				msecs = GetTickCount();
			}

			if (1)
#endif
			{
				if (sock_count > 0)
				{
					if( sock_buf[0] & 0x8 && sock_count > 1 && websocket_state == 2 ){
						fprintf(stderr,"server(%d) close frame from client\n", port);
						break;
					}

					if( sock_buf[0] & 0x81 && sock_count > 6 && websocket_state == 2 ){
						// data from socket to serial, since we are using websocket, we
						// need to check if data is masked
						// it is, this is for spec 10
						// fprintf(stderr,"masked data from socket : %s\n", sock_buf );

						websocket_mask_data( sock_buf, out_buf, sock_count );

						memset( sock_buf,0, sizeof( sock_buf ) );
						memcpy( sock_buf, &out_buf[6], sock_count-6 );
						// fprintf(stderr,"unmasked data : %s\n", sock_buf );
						sock_count -= 6;

					}else if( http_state == 1 ){
						// we can ignore this part with HTTP

						http_state = 2;
					}

					if ((res = sio_write(&pipe->sio, sock_buf, sock_count )) < 0)
					{
						perror2("server(%d) - write(sio)", port);
						break;
					}
					//fprintf(stderr,"wrote %d bytes - sock_count = %d\n", res, sock_count);
					DBG_MSG3("server(%d) - wrote %d bytes to sio", port, res);
					sock_count -= res;
				}
			}
		// fprintf(stderr,sock_buf);
	//	memset((void *)sock_buf, 0, sizeof(sock_buf) );

		}
		/* Unlock our mutex */
		thr_mutex_unlock(pipe->mutex);		
	}

   	fprintf(stderr, "server(%d) exiting\n", port);

  	fprintf(stderr, "server(%d) releasing serial port\n", port);



	/* Clean up - don't call pipe_cleanup() as that would nuke our mutex */
	sio_cleanup(&pipe->sio);
	tcp_cleanup(&pipe->sock);


	free(pipe);
	
	thr_exit((thr_exitcode_t)0);

	return (thr_exitcode_t)0;
}

void debug(void)
{
	vlist_i *it;
	pipe_s *pit;
	int i = 1;
	
	fprintf(stderr, "pipes:\n\n");
	vlist_debug(&pipes, stderr);

	for (it = pipes.head; it; it = it->next)
	{
		pit = (pipe_s *)it->data;

		fprintf(stderr, "sio[%d]:\n\n", i);
		sio_debug(&pit->sio, stderr);

		fprintf(stderr, "sock[%d]:\n\n", i);
		tcp_debug(&pit->sock, stderr);

		i++;
	}
}


