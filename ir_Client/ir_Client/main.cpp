/*
iRacing SDK

Serial client sample for sending serial data to Arduino.
*/

//------
#define MIN_WIN_VER 0x0501

#ifndef WINVER
#	define WINVER			MIN_WIN_VER
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT		MIN_WIN_VER 
#endif

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <string>
#include "irsdk_defines.h"
#include "irsdk_client.h"
#include "serial.h"

Serial serial;

int counter = 0;

// bool, driver is in the car and physics are running
// shut off motion if this is not true
irsdkCVar g_playerInCar("IsOnTrack");
irsdkCVar g_RPM("RPM");

void ex_program(int sig)
{
	(void)sig;

	printf("recieved ctrl-c, exiting\n\n");

	serial.closeSerial();
	timeEndPeriod(1);

	signal(SIGINT, SIG_DFL);
	exit(0);
}
bool init()
{
	// trap ctrl-c
	signal(SIGINT, ex_program);
	printf("press enter to exit:\n\n");

	// bump priority up so we get time from the sim
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	// ask for 1ms timer so sleeps are more precise
	timeBeginPeriod(1);

	// enumerate serial ports and pick the highest port
	//****FixMe, in a real program you would want to handle the port going away and coming back again in a graceful manner.
	int portList[32];
	int portCount = 32;
	int port = serial.enumeratePorts(portList, &portCount);

	// open serial, hopefully this is the arduino
	if (serial.openSerial(port, CBR_57600))
		return true;
	else
		printf("failed to open port %d\n", port);

	return false;
}

void monitorConnectionStatus()
{
	// keep track of connection status
	bool isConnected = irsdkClient::instance().isConnected();
	static bool wasConnected = !isConnected;
	if (wasConnected != isConnected)
	{
		if (isConnected)
			printf("Connected to iRacing\n");
		else
			printf("Lost connection to iRacing\n");
		wasConnected = isConnected;
	}
}
void run()
{
	// wait up to 16 ms for start of session or new data
	if (irsdkClient::instance().waitForData(16))
	{
		// and grab the data
		if (g_playerInCar.getBool())
		{
			if (counter % 2 == 0) // Sending data over serial once every 32ms (2*16ms) 
			{
				int rpm = g_RPM.getInt();
				printf("%d\n", rpm);
				serial.writeSerialPrintf("!%d;", rpm);
			}
		}
		counter++;
	}
	// your normal process loop would go here
	monitorConnectionStatus();
}
int main(int argc, char *argv[])
{
	printf("ir2ad 1.0\n");
	printf(" send iracing data to arduino\n\n");

	if (init())
	{
		while (!_kbhit())
		{
			run();
		}

		printf("Shutting down.\n\n");
		serial.closeSerial();
		timeEndPeriod(1);
	}
	else
		printf("init failed\n");

	return 0;
}
