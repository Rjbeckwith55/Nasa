//============================================================================
// Name        : NasaFinalCode.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "RoboteqDevice.h"
#include "Constants.h"
#include "ErrorCodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#define PORT 5005
using namespace std;
/*Wiring Configurations
 One Roboteq device is plugged into the USB port under device ttyACM0
 The other 3 devices are wired as slave controllers using CAN:
 Which controller controls what motor:
 Master ID-@01: Right drive and Left drive
 Slave ID-@02: Conveyor and Actuators
 Slave ID-@03: Ballscrew and Auger motor1
 Slave ID-@04: Auger motor2 and Auger motor3*/
int main() {
	//4 Roboteq MDC 2130 motor controllers
	RoboteqDevice master;
	struct sockaddr_in address;
	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
	char buffer[7];
	int serverfd;
	//turn console output on and off
	bool DEBUG = true;

	if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		cout << "Socket failed" << endl;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);
	int x = bind(serverfd, (sockaddr *) &address, sizeof(address));
	inet_pton(AF_INET, "192.168.1.80", &address.sin_addr.s_addr);

	//USB connections to the Roboteqs
	int status1 = master.Connect("/dev/ttyACM0");
	//Error checking for device connections
	if (DEBUG) {

		if (status1 != RQ_SUCCESS) {
			cout << "Error connecting to device1: " << status1 << "." << endl;
		}
		int result;
		if (master.GetConfig(_DINA, 1, result) != RQ_SUCCESS) {
			cout << "Failed" << endl;
		} else {
			cout << "Result:" << result << endl;
		}
	}

	while (true) {
		//turn off watch dog timer so the motors stay powered.
		//device1.SetConfig(_RWD,0);
		//device2.SetConfig(_RWD,0);
		master.SetConfig(_RWD,0);
		int power;

		while (recvfrom(serverfd, buffer, 7, 0, (struct sockaddr *) &remaddr,
				&addrlen) > 0) {

			//convert buffer to a string for easier use
			string command(buffer);

			//fixes extra digit issues and string length issues
			if (command.length() >= 6 && command[2] != '-') {
				//fixes an extra digit being sent over
				command = command.substr(0, 5);
			}
			if (DEBUG) {
				//print the command out for debugging purposes
				cout << command << endl;
			}

			//RI - Drive the right side of the robot - Right stick
			//LF - Drive the left side of the robot - Left stick

			//Drive left side of the robot - Left Stick
			if (command.substr(0, 2) == "LF") {
				power = stoi(command.substr(2));
				//left drive motor one will be powered forward or backwards depending on the sign of the power command

				master.SetCommand(_GO, 1, power);
				cout << "- SetCommand(_GO, 1," << power << ")..." << endl;

				//send the data back
				for (int i = 0; i < 5; i++) {
					buffer[i] = command[i];
				}
				inet_ntoa(remaddr.sin_addr);
				ntohs(remaddr.sin_port);
				int y = sendto(serverfd, buffer, 7, 0,
						(struct sockaddr *) &remaddr, addrlen);
				string command(buffer);
				cout << "Sent" << command << endl;
			}
			//Drive right side of the robot - Right Stick
			if (command.substr(0, 2) == "RI") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				master.SetCommand(_GO, 2, power);
			}
			if (command.substr(0, 2) == "CO") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				master.SetCANCommand(02,_GO, 1, power);
			}
			if (command.substr(0, 2) == "SL") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				master.SetCANCommand(02,_GO, 2, power);
			}
			if (command.substr(0, 2) == "AU") {
				//set the command to the number value that was sent
				power = stoi(command.substr(2));
				//3 auger motors for the drill
				master.SetCANCommand(03,_GO, 1, power);
				master.SetCANCommand(04,_GO, 2, power);
				master.SetCANCommand(04,_GO, 1, power);

				if (DEBUG) {
					int amps;
					//print out the amp draw of all of the motors
					master.GetValue(_MOTAMPS, 1, amps);
					cout << "Auger Motor1 Amps" << amps << endl;
					master.GetValue(_MOTAMPS, 2, amps);
					cout << "Auger Motor2 Amps" << amps << endl;
					master.GetValue(_MOTAMPS, 1, amps);
					cout << "Auger Motor3 Amps" << amps << endl;
				}
			}
			//clear out the buffer after each loop
			for (int i = 0; i < 7; i++) {
				buffer[i] = 0;
			}
		}

	}

	return 0;
}
