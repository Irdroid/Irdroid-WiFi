/*
 * sio.c
 *
 * serial port routines
 *
 * (C)1999 serproxy 0.1.2, Stefano Busti
 * (C)2011 serwebproxy 1.0, Lari Temmes
 * 
 * Changes serproxy 0.1.2 -> serwebproxy 1.0
 *	Different function for Windows port (CreateFile instead _open etc.)
 *  Fixed support for COM ports > 9
 *  Some fixes to make it compile for WIN64
 */


#include <stdio.h>

#include "sio.h"

#if defined(SIO_TTY)
#  include <unistd.h>
#  include <termios.h>
#elif defined(SIO_WIN32)
#  include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int sio_init(sio_s *sio)
{
	sio->fd = 0;
	sio->info.port = 1;
	sio->info.baud = 9600;
	sio->info.parity = SIO_PARITY_NONE;
	sio->info.stopbits = 1;
	sio->info.databits = 8;
	sprintf( sio->info.serial_device, "/dev/ttyS" );

	return 0;
}

void sio_cleanup(sio_s *sio)
{
	
	if (sio_isopen(sio)){
		sio_flush(sio, SIO_IN | SIO_OUT );
		sio_close(sio);
	}
}

int sio_open(sio_s *sio)
{
	char filename[30];
	
#if defined(SIO_TTY)
	
	struct termios t;
	int fd, c_stop, c_data, i_parity, c_parity;
	speed_t speed;
	
	if (sio_isopen(sio))
		return -1;
					 
	// sprintf(filename, "/dev/ttyS%hd", sio->info.port - 1);
	sprintf(filename, "%s%hd", sio->info.serial_device, sio->info.port - 1 );	

	fd = open(filename, O_RDWR );

	if (fd == -1){
		fprintf(stderr, "Failed to open serial port %s.\n", filename );
		return -1;
	}
	
	if (tcgetattr(fd, &t))
	{
		fprintf(stderr, "Failed to tcgetattr for serial port %s.\n", filename );

		close(fd);
		return -1;
	}
	
	switch(sio->info.baud)
	{
	case 0: speed = B0; break;
	case 50: speed = B50; break;
	case 75: speed = B75; break;
	case 110: speed = B110; break;
	case 134: speed = B134; break;
	case 150: speed = B150; break;
	case 300: speed = B300; break;
	case 600: speed = B600; break;
	case 1200: speed = B1200; break;
	case 1800: speed = B1800; break;
	case 2400: speed = B2400; break;
	case 4800: speed = B4800; break;
	case 9600: speed = B9600; break;
	case 19200: speed = B19200; break;
	case 38400: speed = B38400; break;
	case 57600: speed = B57600; break;
	case 115200: speed = B115200; break;
	case 230400: speed = B230400; break;
	default: speed = B0; break;
	}

	if (speed == B0)
	{
		close(fd);
		return -1;
	}
	
	if (cfsetospeed(&t, speed))
	{
		close(fd);
		return -1;
	}

	if (cfsetispeed(&t, speed))
	{
		close(fd);
		return -1;
	}
	
	switch(sio->info.stopbits)
	{
	case 1: c_stop = 0; break;
	case 2: c_stop = CSTOPB; break;
	default: close(fd); return -1;
	}

	switch(sio->info.databits)
	{
	case 5: c_data = CS5; break;
	case 6: c_data = CS6; break;
	case 7: c_data = CS7; break;
	case 8: c_data = CS8; break;
	default: close(fd); return -1;
	}

	switch(sio->info.parity)
	{
	case SIO_PARITY_NONE:
		i_parity = IGNPAR;
		c_parity = 0;
		break;
		
	case SIO_PARITY_EVEN:
		i_parity = INPCK;
		c_parity = PARENB;
		break;

	case SIO_PARITY_ODD:
		i_parity = INPCK;
		c_parity = PARENB | PARODD;
		break;

	default:
		close(fd);
		return -1;
	}

  t.c_oflag = 0;

  /*
    ICANON  : enable canonical input
    disable all echo functionality, and don't send signals to calling program
  */
   t.c_lflag = ICANON;

  /*
    initialize all control characters
    default values can be found in /usr/include/termios.h, and are given
    in the comments, but we don't need them here
  */
   t.c_cc[VINTR]    = 0;     /* Ctrl-c */
   t.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
   t.c_cc[VERASE]   = 0;     /* del */
   t.c_cc[VKILL]    = 0;     /* @ */
   t.c_cc[VEOF]     = 4;     /* Ctrl-d */
   t.c_cc[VTIME]    = 0;     /* inter-character timer unused */
   t.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
   t.c_cc[VSWTC]    = 0;     /* '\0' */
   t.c_cc[VSTART]   = 0;     /* Ctrl-q */
   t.c_cc[VSTOP]    = 0;     /* Ctrl-s */
   t.c_cc[VSUSP]    = 0;     /* Ctrl-z */
   t.c_cc[VEOL]     = 0;     /* '\0' */
   t.c_cc[VREPRINT] = 0;     /* Ctrl-r */
   t.c_cc[VDISCARD] = 0;     /* Ctrl-u */
   t.c_cc[VWERASE]  = 0;     /* Ctrl-w */
   t.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
   t.c_cc[VEOL2]    = 0;     /* '\0' */

	if (tcsetattr(fd, TCSANOW, &t))
	{
		close(fd);
		return -1;
	}
	
	sio->fd = fd;
	
	return 0;
#elif defined(SIO_WIN32)
	HANDLE fd;
	HANDLE hComm;
	DCB dcb;
	COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 4, 4 }; 

	if (sio_isopen(sio))
		return -1;
			 
	sprintf(filename, "\\\\.\\COM%d", sio->info.port); 

	hComm = fd = CreateFileA( filename , GENERIC_READ | GENERIC_WRITE, 0, NULL,
			     OPEN_EXISTING, 0, NULL);
	
	if (fd == INVALID_HANDLE_VALUE){
		fprintf(stderr, "Failed to open port %s ", filename );
		return -1;
	}

	if (!SetCommTimeouts(hComm, &cto))
	{
		fprintf(stderr, "SetCommState failed." );
		//_close(fd);
		//return -1;
	}
	
	dcb.DCBlength = sizeof(dcb);
	memset(&dcb, sizeof(dcb), 0);

	if (!GetCommState(hComm, &dcb))
	{
		//_close(fd);
		//return -1;	
		fprintf(stderr, "GetCommState failed." );
	}

	dcb.BaudRate = sio->info.baud;
	dcb.fBinary = TRUE;
	dcb.fParity = (sio->info.parity == SIO_PARITY_NONE) ? FALSE : TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;
	dcb.ByteSize = sio->info.databits;

	switch (sio->info.parity)
	{
	case SIO_PARITY_NONE: dcb.Parity = 0; break;
	case SIO_PARITY_ODD: dcb.Parity = 1; break;
	case SIO_PARITY_EVEN: dcb.Parity = 2; break;		
	default: CloseHandle(fd); return -1;
	}

	switch (sio->info.stopbits)
	{
	case 1: dcb.StopBits = 0; break;
	case 2: dcb.StopBits = 2; break;
	default: CloseHandle(fd); return -1;
	}

	if (!SetCommState(hComm, &dcb))
	{
		fprintf(stderr, "SetCommState failed." );
		//_close(fd);
		//return -1;
	}	

	sio->fd = fd;
	sio->hComm = hComm;

	return 0;
#endif
}

void sio_close(sio_s *sio)
{
#if defined(SIO_TTY)
	close(sio->fd);
	sio->fd = 0;
#elif defined(SIO_WIN32)
	CloseHandle( sio->fd );
	sio->fd = 0;
	sio->hComm = INVALID_HANDLE_VALUE;
#endif
}

#ifdef SIO_TTY
__inline
#endif
int sio_read(sio_s *sio, void *buf, size_t count)
{
#if defined(SIO_TTY)
	return read(sio->fd, buf, count);
#elif defined(SIO_WIN32)
	int res;
	DWORD error;

	if( !ReadFile(sio->hComm, buf, count, &res, NULL )){
		error = GetLastError();
		fprintf(stderr, "Read failed. System error %d", error );
		return -1;
	}
	return res;
#endif
}

#ifdef SIO_TTY
__inline
#endif
int sio_write(sio_s *sio, void *buf, size_t count)
{
#if defined(SIO_TTY)
	return write(sio->fd, buf, count);
#elif defined(SIO_WIN32)
	int res;
	DWORD error;

	if (!WriteFile(sio->hComm, buf, count, &res, NULL)){
		error = GetLastError();
		fprintf(stderr, "Write failed. System error %d", error );
		return -1;
	}
	return res;	
#endif
}

int sio_isopen(sio_s *sio)
{
	return (sio->fd != 0);
}

int sio_setinfo(sio_s *sio, serialinfo_s *info)
{
	sio->info = *info;
	return 0;
}
	
void sio_flush(sio_s *sio, int dir)
{
	if (sio_isopen(sio))
	{
#if defined(SIO_TTY)
		switch(dir)
		{
		case SIO_IN: tcflush(sio->fd, TCIFLUSH); break;
		case SIO_OUT: tcflush(sio->fd, TCOFLUSH); break;
		case SIO_IN | SIO_OUT: tcflush(sio->fd, TCIOFLUSH); break;
		}
#elif defined(SIO_WIN32)
		DWORD flags = 0;

		if (sio_isopen(sio))
		{
			if (dir & SIO_IN)
				flags |= PURGE_RXCLEAR | PURGE_RXABORT;
			if (dir & SIO_OUT)
				flags |= PURGE_TXCLEAR | PURGE_TXABORT;
		}
		
		PurgeComm(sio->hComm, flags);
#endif
	}
}

void sio_drain(sio_s *sio)
{
	if (sio_isopen(sio))
	{
#if defined(SIO_TTY)
		tcdrain(sio->fd);
#elif defined(SIO_WIN32)
		FlushFileBuffers(sio->hComm);
#endif
	}
}

void sio_debug(sio_s *sio, FILE *f)
{
	fprintf(f, "sio {\n");
	fprintf(f, "\tfd = %d\n", sio_isopen(sio) ? sio->fd : 0);
	fprintf(f, "\tport = %d\n", sio->info.port);	
	fprintf(f, "\tbaud = %ld\n", sio->info.baud);	
	fprintf(f, "\tparity = %d\n", sio->info.parity);	
	fprintf(f, "\tstopbits = %d\n", sio->info.stopbits);
	fprintf(f, "\tdatabits = %d\n", sio->info.databits);	
	fprintf(f, "\topen = %d\n", sio_isopen(sio));	
	fprintf(f, "}\n\n");
}
