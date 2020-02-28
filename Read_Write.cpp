// Windows version
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "dynamixel.h"

#pragma comment(lib, "dynamixel.lib")

// Control table address
#define P_GOAL_POSITION_L		30
#define P_GOAL_POSITION_H		31
#define P_PRESENT_POSITION_L	36
#define P_PRESENT_POSITION_H	37
#define P_MOVING				46

// Defualt setting
#define DEFAULT_PORTNUM		3 // COM3
#define DEFAULT_BAUDNUM		1 // 1Mbps

#define RIGHT_SHOULDER_YAW		2
#define RIGHT_SHOULDER_PITCH    25
#define RIGHT_SHOULDER_ROLL		9
#define LEFT_SHOULDER_YAW		11
#define LEFT_SHOULDER_PITCH		20
#define LEFT_SHOULDER_ROLL		22

#define MID_POSITION_GOAL	   512

void PrintCommStatus(int CommStatus);
void PrintErrorCode();

int WaitForKeypress(int nEchoPrompt);

// IS THERE A PROBLEM WITH THIS CODE ?
// Can press space bar approx 3 times, then gets access violation writing location:
// Unhandled exception at 0x00B0F589 in Read_Write.exe: 0xC0000005: Access violation writing location 0x00B96172.

int main()
{
	int GoalPos[2] = { 400, 750 }; // Mmiddle ~= 512
	//int GoalPos[2] = { 0, 1023 };
	//int GoalPos[2] = {0, 4095}; // for EX series

	int ActuatorIdArray[] = { 
		RIGHT_SHOULDER_YAW, 
		RIGHT_SHOULDER_PITCH, 
		RIGHT_SHOULDER_ROLL, 
		LEFT_SHOULDER_YAW, 
		LEFT_SHOULDER_PITCH, 
		LEFT_SHOULDER_ROLL};
	int index = 0;
	int Moving, PresentPos;
	int CommStatus;

	int indexActuator = 0;

	printf("CDeen\n%s\nCompiled: %s %s\n",  __FILE__, __DATE__, __TIME__ );

	// Open device
	if( dxl_initialize(DEFAULT_PORTNUM, DEFAULT_BAUDNUM) == 0 )
	{
		printf( "Failed to open USB2Dynamixel!\n" );

		printf( "Press any key to terminate...\n" );
		getch();
		return 0;
	}
	else
		printf( "Succeed to open USB2Dynamixel!\n" );
	
	while(1)
	{
		printf( "Press any key to continue!\n(press ESC to quit)\n" );
		if (getch() == 0x1b)
			break;

		// Write goal position
		int theActuatorId = ActuatorIdArray[ indexActuator ];
		dxl_write_word( theActuatorId, P_GOAL_POSITION_L, GoalPos[index] );
		do
		{
			// Read present position
			PresentPos = dxl_read_word( theActuatorId, P_PRESENT_POSITION_L );
			CommStatus = dxl_get_result();
			if( CommStatus == COMM_RXSUCCESS )
			{
				printf( "%d %d %d %d\n", theActuatorId, indexActuator, GoalPos[index], PresentPos );
				PrintErrorCode();
			}
			else
			{
				PrintCommStatus(CommStatus);
				break;
			}

			// Check moving done
			Moving = dxl_read_byte( theActuatorId, P_MOVING );
			CommStatus = dxl_get_result();
			if( CommStatus == COMM_RXSUCCESS )
			{
				if( Moving == 0 )
				{
					// Change goal position
					if( index == 0 )
						index = 1;
					else
						index = 0;					
				}

				PrintErrorCode();
			}
			else
			{
				PrintCommStatus(CommStatus);
				break;
			}

			indexActuator++;
			if (indexActuator > 5)
			   indexActuator = 0;

		} while(Moving == 1);
	}

	// Close device
	dxl_terminate();
	printf( "Press any key to terminate...\n" );
	getch();
	return 0;
}

// Print communication result
void PrintCommStatus(int CommStatus)
{
	char *StatusMsg[8] = {
		"COMM_TXSUCCESS: TX Success",
		"COMM_RXSUCCESS: RX Success",
		"COMM_TXFAIL: Failed transmit instruction packet",
		"COMM_RXFAIL: Failed get status packet from device",
		"COMM_TXERROR: Incorrect instruction packet!",
		"COMM_RXWAITING: Now receiving status packet",
		"COMM_RXTIMEOUT: There is no status packet",
		"COMM_RXCORRUPT: Incorrect status packet"
	};

	char* theMsg = "Unknown error code"; // Default

	switch(CommStatus)
	{
		case COMM_TXSUCCESS:
		case COMM_RXSUCCESS:
		case COMM_TXFAIL:
		case COMM_RXFAIL:
		case COMM_TXERROR:
		case COMM_RXWAITING:
		case COMM_RXTIMEOUT:
		case COMM_RXCORRUPT:
			strcat(theMsg, StatusMsg[CommStatus]);
			break;
	}
	printf("%s\n", theMsg);
}

// Does modern CPP offer associate arrays ?
// Print error bit of status packet
void PrintErrorCode()
{
	char* errString = "";

	if (dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
	{
		strcat(errString, "Input voltage error!\n");
		//printf("Input voltage error!\n");
		printf("%s", errString);
	}

	if(dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
		printf("Angle limit error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
		printf("Overheat error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
		printf("Out of range error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
		printf("Checksum error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
		printf("Overload error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
		printf("Instruction code error!\n");
}

int WaitForKeypress(int nEchoPrompt)
{
	if (nEchoPrompt)
	{
		printf("Press any key ...\n");
	}

	return getch(); // Return code from getch().
}
