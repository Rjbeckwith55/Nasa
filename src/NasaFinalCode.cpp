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
#include <sys/socket.h>
using namespace std;

int main() {
	RoboteqDevice device1;
	RoboteqDevice device2;
	int status1 = device1.Connect("/dev/ttyACM0");
	int status2 = device2.Connect("/dev/ttyACM1");
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	//Error checking
	if(status1 != RQ_SUCCESS)
		{
			cout<<"Error connecting to device1: "<<status1<<"."<<endl;
			return 1;
		}
	if(status2 != RQ_SUCCESS)
			{
				cout<<"Error connecting to device2: "<<status2<<"."<<endl;
				return 1;
			}

	return 0;
}
