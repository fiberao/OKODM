/**************************************************** 
 * FileName: mirror37usb.h
 * Description:  
 *
 * (c) 2004, FlexibleOptical 
 *           Gleb Vdovin & Mikhail Loktev
 ****************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "ftd2xx.h"

FT_HANDLE com;
BYTE packet[130];         // Packet of DAC data
WORD buf[40];             // Buffer of DAC channels

static BYTE DAC_CHANEL_TABLE[40]= // Table of DAC channels
    {
/*DAC-> 0   1   2   3   4    |
        ---------------------+  OUTPUT    */
        7, 15, 23, 31, 39, //|  A
        6, 14, 22, 30, 38, //|  B
        5, 13, 21, 29, 37, //|  C
        4, 12, 20, 28, 36, //|  D
        3, 11, 19, 27, 35, //|  E
        2, 10, 18, 26, 34, //|  F
        1,  9, 17, 25, 33, //|  G
        0,  8, 16, 24, 32  //|  H
    };

void MakePacket(WORD *buf,BYTE *packet)
/*
   Forming of the packet of DAC data
   "buf" is an array consisting of 16-bit voltage values, which correspond to channels 1..40;
   "packet" is an array consisting of 129 bytes of data that provide setting of the voltage 
   levels at the outputs of DAC-40-USB
*/
{
	BYTE *p=packet+1;

	for(int i=0,s=0;i<8;i++,s+=5)
    {
        // Forming of the address parts of the control data words to be addressed to five DAC chips
    *(p++)=0;
    *(p++)=(i&4)?0x1f:0;
    *(p++)=(i&2)?0x1f:0;
    *(p++)=(i&1)?0x1f:0;

    // Forming of the voltage level codes from the buffer of DAC channels
	// using the table of DAC channels
    for(int j=0,mask=0x800;j<12;j++,mask>>=1)
        *(p++)=
            ((buf[DAC_CHANEL_TABLE[s+0]]&mask)?0x01:0) |
            ((buf[DAC_CHANEL_TABLE[s+1]]&mask)?0x02:0) |
            ((buf[DAC_CHANEL_TABLE[s+2]]&mask)?0x04:0) |
            ((buf[DAC_CHANEL_TABLE[s+3]]&mask)?0x08:0) |
            ((buf[DAC_CHANEL_TABLE[s+4]]&mask)?0x10:0) ;
    }
	packet[0]   = 0xff; // non-zero start byte
}

#define TOTAL_NR_OF_CHANNELS	37
// The total number of channels used 

#define MAX_AMPLITUDE 4095  
// Maximum value that can be addressed

int voltage[TOTAL_NR_OF_CHANNELS];
// Array of voltages addressed to the mirror, values in the range [0..MAX_AMPLITUDE]

int addr[TOTAL_NR_OF_CHANNELS]={10,18,13,9,5,6,14,19,17,12,11,1,3,2,4,7,8,15,16,
                                34,36,37,38,35,23,25,21,22,24,26,27,28,29,30,31,32,33};
// Table of correspondence between the order numbers of the mirror's actuators and
// the order numbers of outputs of DAC-40-USB unit

BOOL init_dac(int index)
// Initialization of DAC-40-USB control unit
{
	DWORD ndevs=0;
 	if ((FT_ListDevices(&ndevs,NULL,FT_LIST_NUMBER_ONLY)==FT_OK))
	{
		if (index >= ndevs) {
			printf("\nTrying to connect with a non-existing device id %d.\n %d device(s) are found on this computer.\n",index, ndevs);
			return FALSE;
		}
			
		char sn[16],dsc[64];
                                // Get the serial number
		FT_ListDevices((PVOID)index,sn,FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
                                // Get the description
		FT_ListDevices((PVOID)index,dsc,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);
		printf("\nDevice serial: %s\n",sn);
		printf("\nDevice description: %s\n", dsc);
		FT_STATUS fs = FT_Open(index, &com);
		if(fs!=FT_OK)
        {                       // Error handling
			printf("\nError opening the device\n");
			return FALSE;
		}
		memset(packet,0,130);          // Fill the buffer with zeros
		unsigned long BR;
		FT_Write(com,packet,130, &BR); // Send zeros to initialize the device
	}
	else
	{
		printf("\nDevices enumeration failed. Check your driver installation.\n");
		return FALSE;
	}
	return TRUE;
}

void close_dac()
// Close DAC-40-USB control unit
{
	FT_Close(com);
}

void set_mirror()
// Set voltages from the "voltage[]" array to the mirror
{	
	int i;
	unsigned long BR=0;

	for(i=0; i<TOTAL_NR_OF_CHANNELS; i++)
		buf[addr[i]] = voltage[i];
	MakePacket(buf,packet);  
	FT_Write(com,packet,130,&BR);
}


