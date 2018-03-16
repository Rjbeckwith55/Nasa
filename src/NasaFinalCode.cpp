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

int main() {
	//4 Roboteq MDC 2130 motor controllers
	RoboteqDevice device1;
	RoboteqDevice device2;
	RoboteqDevice device3;
	RoboteqDevice device4;
	struct sockaddr_in address;
	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
	char buffer[7];
	int serverfd;

	if((serverfd = socket(AF_INET,SOCK_DGRAM,0))==0){
		cout<<"Socket failed"<<endl;	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);
	int x = bind(serverfd,(sockaddr *)&address ,sizeof(address));
	inet_pton(AF_INET,"192.168.1.80",&address.sin_addr.s_addr);

	//USB connections to the Roboteqs
	int status1 = device1.Connect("/dev/ttyACM3");
	int status2 = device2.Connect("/dev/ttyACM0");
	int status3 = device3.Connect("/dev/ttyACM2");
	int status4 = device4.Connect("/dev/ttyACM1");
	//Error checking for device connections
	if(status1 != RQ_SUCCESS)
	{
		cout<<"Error connecting to device1: "<<status1<<"."<<endl;
	}
	if(status2 != RQ_SUCCESS)
	{
		cout<<"Error connecting to device2: "<<status2<<"."<<endl;
	}
	if(status3 != RQ_SUCCESS)
	{
		cout<<"Error connecting to device3: "<<status3<<"."<<endl;
	}
	if(status4 != RQ_SUCCESS)
	{
		cout<<"Error connecting to device4: "<<status4<<"."<<endl;
	}
	int result;
	if(device3.GetConfig(_DINA,1,result) != RQ_SUCCESS)
	{
		cout<<"Failed"<<endl;
	}
	else{
	cout<<"Result:"<<result<<endl;
	}



	while(true){
		//turn off watch dog timer so the motors stay powered.
		//device1.SetConfig(_RWD,0);
		//device2.SetConfig(_RWD,0);
		device3.SetConfig(_RWD,0);
		device4.SetConfig(_RWD,0);
		int power;

		while( recvfrom(serverfd,buffer,7,0,(struct sockaddr *)&remaddr,&addrlen)>0){


			//convert buffer to a string for easier use
			string command(buffer);

			//fixes extra digit issues and string length issues
			if(command.length() >=6 && command[2]!='-'){
				//fixes an extra digit being sent over
				command = command.substr(0,5);
			}
			cout<<command<<endl;


			//RI - Drive the right side of the robot - Right stick
			//LF - Drive the left side of the robot - Left stick


			//Drive left side of the robot - Left Stick
			if(command.substr(0,2) == "LF"){
				power =  stoi(command.substr(2));
				//left drive motor one will be powered forward or backwards depending on the sign of the power command

				device1.SetCommand(_GO,1,power);
				cout<<"- SetCommand(_GO, 1,"<<power<<")..."<<endl;

				//sendto
				for (int i =0; i<5;i++){
					buffer[i] = command[i];
				}
				inet_ntoa(remaddr.sin_addr);
				ntohs(remaddr.sin_port);
				int y = sendto(serverfd, buffer, 7, 0, (struct sockaddr *)&remaddr, addrlen);
				string command(buffer);
				cout<<"Sent" << command<<endl;
			}
			//Drive right side of the robot - Right Stick
			if(command.substr(0,2)=="RI"){
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				device1.SetCommand(_GO,2,power);
				cout<<"- SetCommand(_GO, 2,"<<power<<")..."<<endl;
			}
			if(command.substr(0,2)=="CO"){
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				device2.SetCommand(_GO,1,power);
				cout<<"- SetCommand(_GO, 1,"<<power<<")..."<<endl;
			}
			if(command.substr(0,2)=="SL"){
				power = stoi(command.substr(2));
				//right drive motor will be powered forward or backwards depending on the sign of the power command
				device2.SetCommand(_GO,2,power);
				cout<<command.substr(0,2)<<"- SetCommand(_GO, 2,"<<power<<")..."<<endl;
			}
			if(command.substr(0,2)=="AU"){
				power = stoi(command.substr(2));
				//3 auger motors for the drill
				device3.SetCommand(_GO,1,power);
				device3.SetCommand(_GO,2,power);
				device4.SetCommand(_GO,1,power);
				cout<<command.substr(0,2)<<endl;
			}
			//clear out the buffer after each loop
			for(int i = 0; i <7;i++){
				buffer[i]=0;
			}
		}

	}

	return 0;
}
