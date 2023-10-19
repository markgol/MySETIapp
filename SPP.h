#pragma once
#include <stdint.h>
//
// Space Packet Procol definitions
//	Source: ECSS-E-ST-70-41C15April2016.PDF, European Cooperation for Space Standardization, ECSS-E-ST-70-41C1
//	Source:	133x0b2e1.pdf, Consultative Committee for Space Data Systems, CCSDS 133.0-B-2
//
// This uses bitfield definitions to do the bit field extractions
//
#pragma pack(push, 1)

typedef struct SPP_UNPACKED_PRIMARY_HEADER {
	int PVN;
	int Type;
	int SecHeaderFlag;
	int APID;
	int SeqFlag;
	int SeqCount;
	int DataLength;
} SPP_UNPACKED_PRIMARY_HEADER;

typedef struct SPP_PRIMARY_HEADER {
	union {
		unsigned char Bytes[6];
		struct {
			uint16_t ID;
			uint16_t SEQ;
			uint16_t DataLength;
		};
	};
} SPP_PRIMARY_HEADER;


typedef struct SPP_TM_SECONDARY_HEADER {
	//
	// Packet for secondary header for telemetry packets
	// This required for all telemetry packets that are not spacecraft time packets
	//
	int	PUSver;		// Bits 0-3 bits, TM packet PUS version number
					// 2 - ECSS-E-70-41C
					// 1 - ECSS-E-70-41A
					// 0 - ESA PSS-07-101
	int SpaceTime;	// 4-7 bits, spacecraft time reference status
	int ServiceID;	// Bits 8-15, Service Type ID
	int subtypeID;	// Bits 16-23, message subtype ID
	int msgCounter;	// Bits 24-39, message type counter
	int DestID;		// bits 40-55, destination ID
	int Pad[3];		// bits 56-79, the last 3 bytes of the secondary header
} SPP_TM_SECONDARY_HEADER;

#pragma pack(pop)