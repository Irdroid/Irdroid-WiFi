
SerWebProxy 1.12
----------------

Author : Lari Temmes (lari.temmes@gmail.com)
Original author of serproxy 0.1.2 : Stefano Busti (sbusti@nildram.co.uk)

SerWebProxy is a GPL multi-threaded proxy program for redirecting
network socket connections to/from serial links, in cases where
the remote end of the serial link doesn't have a TCP/IP
stack (eg an embedded or microcontroller system).

The proxy allows other hosts on the network to communicate
with the system on the remote end of the serial link.
When run, it listens for incoming connections on a specified TCP
ports. Whenever a connection is made data is proxied to and from
that connection to a serial port.

To control serial device from browser, there are two protocol available :

* raw TCP socket                       (protocol 0 in config)
* WebSocket protocol (spec 10)         (protocol 1 in config)
* HTTP GET                             (protocol 2 in config)

WebSocket works as expected, connection is kept open and it's very similar to
how serial ports work. However, HTTP GET request is one-way and single request
type of communication. SerWebProxy handles it so that when GET request is made,
parameters (everything after hostname) are sent to serial port and response is
read until end of line appears. Then data is sent to network and connection is
closed. 

Example GET request to make from browser could be 'http://localhost:5331/hello'
and serial device would then receive hello.

Installation
------------

See the files INSTALL-unix and INSTALL-win32.txt for windows/unix
installation instructions.

Windows version is tested with Microsoft Visual Studio 2010. Solution
file is included.

Unix version is tested with Debian 6.0. Requires pthread library
to compile/run.

Both platforms also need openssl library to compile/run.

Configuration directives
------------------------

SerWebProxy can be configured via the configuration file "serwebroxy.cfg".
Comments are allowed by starting a line with the '#' character.
The configuration file supports the following directives:

---------------------------------------------------------------------
comm_ports=<list>

Used to configure which comm ports SerWebProxy will use.
<list> is a comma-separated list of port numbers, where 1 corresponds
to ttyS0/COM1, 2 corresponds to ttyS1/COM2, etc.

Example:

# Use ports 1,3
comm_ports=1,3

---------------------------------------------------------------------
comm_baud=<n>

Sets the default baud rate for serial connections.

Example:

# Use 38400 baud
comm_baud=38400

---------------------------------------------------------------------
comm_databits=<n>

Sets the default number of data bits.

Example:

# Use 8 data bits
comm_databits=8

---------------------------------------------------------------------
comm_stopbits=<n>

Sets the default number of stop bits. Possible values are 1 or 2.

Example:

# Use 1 stop bit
comm_stopbits=1

---------------------------------------------------------------------
comm_parity=<p>

Sets the default parity. Possible values are "none", "even" and "odd"

Example:

# No parity
comm_parity=none

---------------------------------------------------------------------
timeout=<n>

If a client is idle for more than <n> seconds, he will be
disconnected.

Example:

# Time out after 300 seconds
timeout=300

---------------------------------------------------------------------
net_port<x>=<n>

Used to map tcp port numbers to serial port numbers. These define
which serial ports serproxy tries to open when it detects a tcp
network connection on a socket for proxying. <x> is the serial
port number, <n> is the tcp port number.

Example:

# Proxy ttyS0/COM1 to/from tcp port 5331
net_port1=5331

# Proxy ttyS2/COM3 to/from tcp port 5333
net_port3=5333

---------------------------------------------------------------------
net_protocol<x>=<n>

Used to select if TCP port uses WebSocket protocol or not.

n = 0, send only unmodified raw data
n = 1, use WebSocket protocol (default)
n = 2, listen for HTTP GET requests

Example:

# Use WebSocket protocol with device 3
net_protocol3=1


---------------------------------------------------------------------
Port-specific settings

The baud rate, data bits, stop bits, timeout and protocol values can also be
configured individually for each serial port. This is achieved
by appending a port number to the end of a particular directive.

Example:

# Override the global settings for ttyS0
comm_baud1=14400

# Override the global settings for ttyS2/COM3
comm_baud3=9600
comm_databits3=7
comm_stopbits3=2
comm_parity3=odd
comm_websocket3=0
timeout3=100

