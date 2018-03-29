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
 Each Roboteq handles 2 channels and is plugged into a USB on each one.
 */

int main() {
	//4 Roboteq MDC 2230 motor controllers
	RoboteqDevice Roboteq1, Roboteq2, Roboteq3, Roboteq4, placeholder;
	//placeholder is just to make the array make more sense when coding it.
	RoboteqDevice roboteqs[] = { placeholder, Roboteq1, Roboteq2, Roboteq3,
			Roboteq4 };
	struct sockaddr_in address;
	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
	char buffer[7];
	int serverfd;
	bool sendData = true;
	//turn console output on and off
	bool DEBUG = false;

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
			cout << "Returned:" << result << endl;
		}
	}
	Roboteq1.SetConfig(_RWD, 0);
	Roboteq2.SetConfig(_RWD, 0);
	Roboteq3.SetConfig(_RWD, 0);
	Roboteq4.SetConfig(_RWD, 0);
	while (true) {
		//turn off watch dog timer so the motors stay powered.
//		for (int i = 1; i < 5; i++) {
//			roboteqs[i].SetConfig(_RWD, 0);
//		}

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
			sendData = true;
			//Drive left side of the robot - Left Stick
			if (command.substr(0, 2) == "LF") {
				power = stoi(command.substr(2));
				//left drive motor one will be powered forward or backwards depending on the sign of the power command
				Roboteq1.SetCommand(_GO, 1, power);
				sendData = false;
			}
			//Drive right side of the robot - Right Stick
			if (command.substr(0, 2) == "RI") {
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				Roboteq1.SetCommand(_GO, 2, power);
				sendData = false;
			}
			if (command.substr(0, 2) == "CO") {
				power = stoi(command.substr(2));
				//Conveyor belt
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
			if (command.substr(0, 2) == "TI") {
				power = stoi(command.substr(2));
				//Actuators up and down
				Roboteq2.SetCommand(_GO, 2, power);
			}
			if (command.substr(0, 2) == "QU") {
				//stop all motors
				Roboteq1.SetCommand(_GO,1,0);
				Roboteq1.SetCommand(_GO,2,0);
				Roboteq2.SetCommand(_GO,1,0);
				Roboteq2.SetCommand(_GO,2,0);
				Roboteq3.SetCommand(_GO,1,0);
				Roboteq3.SetCommand(_GO,2,0);
				Roboteq4.SetCommand(_GO,1,0);
				Roboteq4.SetCommand(_GO,2,0);

				//buffer[]= {'O','F','F'};
				int y = sendto(serverfd, buffer, 3, 0,
						(struct sockaddr *) &remaddr, addrlen);
				if (DEBUG) {
					cout << "All motors off" << endl;
				}
			}
			if (sendData) {
				//Send the data back to the server
				string send;
				int values[15];
				int n = 0;
				int leftamp, rightamp, conveyoramp, tiltamp, augamp1, augamp2,
						augamp3, screwamp, augtemp1, augtemp2, augtemp3,
						screwtemp;
				//get the motor amps of each motor
				Roboteq1.GetValue(_MOTAMPS, 1, leftamp);
				Roboteq1.GetValue(_MOTAMPS, 2, rightamp);
				Roboteq2.GetValue(_MOTAMPS, 1, conveyoramp);
				Roboteq2.GetValue(_MOTAMPS, 2, tiltamp);
				Roboteq3.GetValue(_MOTAMPS, 1, augamp1);
				Roboteq3.GetValue(_MOTAMPS, 2, augamp2);
				Roboteq4.GetValue(_MOTAMPS, 1, augamp3);
				Roboteq4.GetValue(_MOTAMPS, 2, screwamp);
				//get the temperatures of the temperature sensors on the different motors
				cout<<Roboteq3.GetValue(_ANAIN, augtemp1);
				cout<<Roboteq3.GetValue(_ANAIN, augtemp2);
				cout<<Roboteq4.GetValue(_ANAIN, augtemp3);
				cout<<Roboteq4.GetValue(_ANAIN, screwtemp);
				augtemp3 = (augtemp3*125)>>8;
				screwtemp = (screwtemp*125)>>8;
				augtemp1 = (augtemp1*125)>>8;
				augtemp2 = (augtemp2*125)>>8;
				send = to_string(leftamp) + "," + to_string(rightamp) + ","
						+ to_string(conveyoramp) + "," + to_string(tiltamp)
						+ "," + to_string(augamp1) + "," + to_string(augamp2)
						+ "," + to_string(augamp3) + "," + to_string(screwamp)
						+ "," + to_string(augtemp1) + "," + to_string(augtemp2)
						+ "," + to_string(augtemp3) + ","
						+ to_string(screwtemp);
//						for (int i = 0; i <= n; i++) {
//							send += ","+values[i];
//						}
				if (DEBUG) {
					int debugValues[15];
					int d = 0;
					Roboteq1.GetValue(_BATAMPS, debugValues[d++]);
					Roboteq1.GetValue(_TEMP, debugValues[d++]);
					Roboteq1.GetValue(_VOLTS, debugValues[d++]);
					for (int i = 0; i <= d; i++) {
						send += "," + values[i];
					}
					cout << "Sent: " << send << endl;
				}
				for (int i = 0; i < send.length(); i++) {
					buffer[i] = send[i];
				}
				inet_ntoa(remaddr.sin_addr);
				ntohs(remaddr.sin_port);
				int y = sendto(serverfd, buffer, 30, 0,
						(struct sockaddr *) &remaddr, addrlen);
			}
			//clear out the buffer after each loop
			for (int i = 0; i < 7; i++) {
				buffer[i] = 0;
			}
		}

	}

	return 0;
}
