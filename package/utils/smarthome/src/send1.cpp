/*
 Usage: ./send <systemCode> <unitCode> <command>
 Command is 0 for OFF and 1 for ON
 */

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    
    /*
     output PIN is hardcoded for testing purposes
     see https://projects.drogon.net/raspberry-pi/wiringpi/pins/
     for pin mapping of the raspberry pi GPIO connector
     */
//    int PIN = 121;
    int systemCode =atoi( argv[1]);
    int unitCode = atoi(argv[2]);
    int command  = atoi(argv[3]);
    int timer = atoi(argv[4]);
    int pin = atoi(argv[5]);
    int repeat = atoi(argv[6]);    
//    if (wiringPiSetup () == -1) return 1;
	printf("sending systemCode[%i] unitCode[%i] command[%i] timer[%i] pin[%i] repeat[%i]\n", systemCode, unitCode, command, timer, pin, repeat);
	RCSwitch mySwitch = RCSwitch();
	mySwitch.setPulseLength(timer);
        mySwitch.enableTransmit(pin);
	mySwitch.setRepeatTransmit(repeat);
    switch(command) {
        case 1:
            mySwitch.switchOn(systemCode, unitCode);
            break;
        case 0:
            mySwitch.switchOff(systemCode,unitCode);
            break;
        default:
            printf("command[%i] is unsupported\n", command);
            return -1;
    }
	return 0;
}
