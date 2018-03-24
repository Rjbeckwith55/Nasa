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
 */

int main() {
	//4 Roboteq MDC 2130 motor controllers
	RoboteqDevice Roboteq1, Roboteq2, Roboteq3, Roboteq4,placeholder;
	//placeholder is just to make the array make more sense when coding it.
	RoboteqDevice roboteqs[] = {placeholder, Roboteq1, Roboteq2, Roboteq3, Roboteq4 };
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
	int status1 = Roboteq1.Connect("/dev/ttyACM0");
	int status2 = Roboteq2.Connect("/dev/ttyACM1");
	int status3 = Roboteq3.Connect("/dev/ttyACM2");
	int status4 = Roboteq4.Connect("/dev/ttyACM3");
	//Error checking for device connections
	if (DEBUG) {

		if (status1 != RQ_SUCCESS) {
			cout << "Error connecting to Roboteq1: " << status1 << "." << endl;
		}
		int result;
		if (Roboteq1.GetConfig(_DINA, 1, result) != RQ_SUCCESS) {
			cout << "Failed to receive configuration data" << endl;
		} else {
			cout << "Result:" << result << endl;
		}
	}

	while (true) {
		//turn off watch dog timer so the motors stay powered.
		for (int i = 1; i < 5; i++) {
			roboteqs[i].SetConfig(_RWD, 0);
		}

		int power;

		while (recvfrom(serverfd, buffer, 7, 0, (struct sockaddr *) &remaddr,
				&addrlen) > 0) {

			//convert buffer to a string for easier use
			string command(buffer);
			//Send the data back to the server
			string send;
			int values[15];
			int n = 0;
			cout<<Roboteq1.GetValue(_MOTAMPS, 1, values[n])<<endl;
			cout<<Roboteq1.GetValue(_MOTAMPS, 2, values[++n])<<endl;
			Roboteq2.GetValue(_MOTAMPS, 1, values[++n]);
			Roboteq2.GetValue(_MOTAMPS, 2, values[++n]);
			Roboteq3.GetValue(_MOTAMPS, 1, values[++n]);
			Roboteq3.GetValue(_MOTAMPS, 2, values[++n]);
			Roboteq4.GetValue(_MOTAMPS, 1, values[++n]);
			Roboteq4.GetValue(_MOTAMPS, 2, values[++n]);

			for (int i = 0; i <= n; i++) {
				send += values[i];
			}
			if (DEBUG) {
				int debugValues[15];
				int d = 0;
				Roboteq1.GetValue(_BATAMPS, debugValues[d++]);
				Roboteq1.GetValue(_TEMP, debugValues[d++]);
				Roboteq1.GetValue(_VOLTS, debugValues[d++]);
				for (int i = 0; i <= d; i++) {
					send += ","+values[i];
				}
				cout << "Sent: " << send << endl;
			}
			for (int i = 0; i < send.length(); i++) {
				buffer[i] = send[i];
			}
			inet_ntoa(remaddr.sin_addr);
			ntohs(remaddr.sin_port);
			int y = sendto(serverfd, buffer, 100, 0,
					(struct sockaddr *) &remaddr, addrlen);

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
				Roboteq1.SetCommand(_GO, 1, power * .2);
			}
			//Drive right side of the robot - Right Stick
			if (command.substr(0, 2) == "RI") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				Roboteq1.SetCommand(_GO, 2, power * .2);
			}
			if (command.substr(0, 2) == "CO") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				Roboteq2.SetCommand(_GO, 1, power);
			}
			if (command.substr(0, 2) == "SL") {
				power = stoi(command.substr(2));
				//Ballscrew up and down
				Roboteq4.SetCommand(_GO, 2, power);
			}
			if (command.substr(0, 2) == "AU") {
				//AUGER command 3 motors
				power = stoi(command.substr(2));
				//3 auger motors for the drill
				Roboteq3.SetCommand(_GO, 1, power);
				Roboteq3.SetCommand(_GO, 2, power);
				Roboteq4.SetCommand(_GO, 1, power);
			}
			if (command.substr(0, 2) == "AC") {
				power = stoi(command.substr(2));
				//Actuators up and down
				Roboteq2.SetCommand(_GO, 2, power);
			}
			if (command.substr(0, 2) == "QU") {
				//stop all motors
				for (int i = 1; i < 5; i++) {
					roboteqs[i].SetCommand(_GO, 1, 0);
					roboteqs[i].SetCommand(_GO, 2, 0);
				}
				//buffer[]= {'O','F','F'};
				int y = sendto(serverfd, buffer, 3, 0,
									(struct sockaddr *) &remaddr, addrlen);
				if (DEBUG) {
					cout << "All motors off" << endl;
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
