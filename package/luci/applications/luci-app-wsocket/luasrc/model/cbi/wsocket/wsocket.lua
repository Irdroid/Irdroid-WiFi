-- Copyright 2015 Georgi Bakalski
-- Licensed to the public under the Apache License 2.0.

m = Map("wsocket", translate("WebSocket"),
        translate("The wsocket allows websocket connections to be established with a unit's serial ports.<br/>"))

function m.on_after_commit(self)
        luci.sys.call("/etc/init.d/wsocket enable 1\>/dev/null 2\>/dev/null")
        luci.sys.call("/etc/init.d/wsocket restart 1\>/dev/null 2\>/dev/null")
end

s = m:section(TypedSection, "proxy", translate("Proxies"),
        translate("The program comes up normally as a service, opens the TCP ports specified in the configuration file, and waits for connections."))

s.anonymous = true
s.addremove = true

tcpport = s:option(Value, "tcpport", translate("TCP Port"),
        translate("Name or number of the TCP/IP port to accept connections from for this device."))

tcpport.rmempty = false
tcpport.default = "10.10.1.10,5331"

state = s:option(Value, "state", translate("State"),
        translate("Either wsocket (1), http (2) or RAW (3)"))
state.rmempty = false
state:value("1", translate("Websocket"))
state:value("2", translate("HTTP"))
state:value("3", translate("RAW"))
state.default = "1"


timeout = s:option(Value, "timeout", translate("Timeout"),
        translate("The time (in seconds) before the port will be disconnected if there is no activity on it.<br/>A zero value disables this funciton."))
timeout.rmempty = false
timeout.default = "500"

device = s:option(Value, "device", translate("Device"),
        translate("The name of the device to connect to.<br/>This must be in the form of /dev/<device>."))
device.rmempty = false
device.default = "/dev/ttyS0"

options = s:option(Value, "options", translate("Options"),
        translate("Sets operational parameters for the serial port.<br/>Values may be separated by spaces or commas.<br/>Options 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 set the various baud rates. EVEN, ODD, NONE set the parity.<br/>1STOPBIT, 2STOPBITS set the number of stop bits.<br/>7DATABITS, 8DATABITS set the number of data bits. [-]XONXOFF turns on (- off) XON/XOFF support.<br/>[-]RTSCTS turns on (- off) hardware flow control.<br/>[-]LOCAL ignores (- checks) the modem control lines (DCD, DTR, etc.) [-]HANGUP_WHEN_DONE lowers (- does not lower) the modem control lines (DCD, DTR, etc.) when the connection closes.<br/>NOBREAK Disables automatic clearing of the break setting of the port.<br/>remctl allows remote control of the serial port parameters via RFC 2217.<br/><br/> the README for more info. <banner name> displays the given banner when a user connects to the port.<br/><br/>" .. 
"tr=<filename> When the port is opened, open the given tracefile and store all data read from the physical device (and thus written to the user's TCP port) in the file. The actual filename is specified in the TRACEFILE directive. If the file already exists, it is appended.<br/> The file is closed when the port is closed.<br/><br/>" .. 
"tw=<filename> Like tr, but traces data written to the device.<br/><br/>" ..
"tb=<filename> trace both read and written data to the same file.<br/>Note that this is independent of tr and tw, so you may be tracing read, write, and both to different files.<br/><br/>" .. 
"banner name<br/>" .. 
"A name for the banner; this may be used in the options of a port.<br/><br/>" .. 
"banner text<br/>" .. 
"The text to display as the banner. It takes escape sequences for substituting strings, see 'FILENAME AND BANNER FORMATTING' for details.<br/><br/>" .. 
"tracefile name<br/>" .. 
"A name for the tracefile, this is used in the tw, tr, and tb options of a port.<br/><br/>" .. 
"tracefile<br/>" .. 
"The file to send the trace into. Note that this takes escape sequences for substituting strings, see 'FILENAME AND BANNER FORMATTING' for details.<br/>Note that when using the time escape sequences, the time is read once at port startup, so if you use both tw and tr they will have the same date and time."))

options.rmempty = true
options.default = ""

return m
