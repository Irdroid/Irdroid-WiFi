/*
 * sock.c
 *
 * socket routines
 *
 * (C)1999 Stefano Busti
 *
 */


#include <stdlib.h>

#include "sock.h"



#  include <sys/types.h>


#if defined(SOCK_BSD)
#  include <unistd.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/ioctl.h>
#  include <fcntl.h>
#else

#include <winsock.h>
#	define ioctl ioctlsocket
#endif

int sock_start(void)
{
#if defined(SOCK_BSD)
	return 0;
#else
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	return WSAStartup(wVersionRequested, &wsaData);
#endif
}


void sock_finish(void)
{
#if defined(SOCK_BSD)
#else
	WSACleanup();
#endif
}

int tcp_init(tcpsock_s *sock, int port)
{
	int optval = 1;
	
	//struct protoent *ent;
	struct sockaddr_in name;
	
	//ent = getprotobyname("TCP");

	//if (!ent)
	//	return -1;
	
	sock->fd = socket(AF_INET, SOCK_STREAM, 0/*ent->p_proto*/);

	if (sock->fd < 0)
		return -1;

	/* Allow reuse of existing TIME_WAIT sockets */
	if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR,
				   (void *)&optval, sizeof(optval)))
	{
		tcp_cleanup(sock);
		return -1;
	}

	if (ioctl(sock->fd, FIONBIO, &optval))
	{
		tcp_cleanup(sock);
		return -1;		
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock->fd, (struct sockaddr *)&name, sizeof(name)) < 0)
	{
		tcp_cleanup(sock);
		return -1;
	}

	sock->port = port;

	return 0;
}

void tcp_cleanup(tcpsock_s *sock)
{
#if defined(SOCK_BSD)
	close(sock->fd);
#else
	closesocket(sock->fd);
#endif
}

int tcp_listen(tcpsock_s *sock)
{
	return listen(sock->fd, 0);
}

tcpsock_s *tcp_accept(tcpsock_s *sock)
{
	struct sockaddr addr;
	int addrlen;
	int fd;
	tcpsock_s *newsock;
	
	addrlen = sizeof(addr);
	
	fd = accept(sock->fd, &addr, &addrlen);

	if (fd < 0)
		return NULL;

	newsock = malloc(sizeof(tcpsock_s));

	if (newsock)
		newsock->fd = fd;

	return newsock;
}

void tcp_refuse(tcpsock_s *sock)
{
	tcpsock_s *temp = tcp_accept(sock);

	if (temp)
	{
		/* Disconnect immediately */
		tcp_disconnect(temp);

		/* Clean up */
		tcp_cleanup(temp);
		free(temp);
	}
}

int tcp_connect(tcpsock_s *sock, char *host, int port)
{
	struct sockaddr_in name;
	struct hostent *hostinfo;
	unsigned int addr;

	name.sin_family = AF_INET;
	name.sin_port = htons(port);

	addr = inet_addr(host);
	hostinfo = gethostbyaddr((char *)&addr, 4, PF_INET);
	/*hostinfo = gethostbyname(hostname);*/

	if (hostinfo == NULL)
		return -1;
	
	name.sin_addr = *(struct in_addr *) hostinfo->h_addr;
		
	return connect(sock->fd, (struct sockaddr *)&name, sizeof(name));
}

void tcp_disconnect(tcpsock_s *sock)
{
	shutdown(sock->fd, 2);
}

#ifdef SOCK_BSD
__inline
#endif
int tcp_read(tcpsock_s *sock, void *buf, size_t count)
{
#if defined(SOCK_BSD)
	return read(sock->fd, buf, count);
#else
	return recv(sock->fd, buf, count, 0);
#endif
}

#ifdef SOCK_BSD
__inline
#endif
int tcp_write(tcpsock_s *sock, void *buf, size_t count)
{
#if defined(SOCK_BSD)
	return write(sock->fd, buf, count);
#else
	return send(sock->fd, buf, count, 0);
#endif
}

int tcp_getport(tcpsock_s *sock)
{
	return sock->port;
}

void tcp_debug(tcpsock_s *sock, FILE *f)
{
	fprintf(f, "tcp_socket {\n");
	fprintf(f, "\tfd = %d\n", sock->fd);
	fprintf(f, "\tport = %d\n", sock->port);
	fprintf(f, "}\n\n");
}
