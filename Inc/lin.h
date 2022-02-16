#ifndef _lin_h_
#define _lin_h_

#include <stdint.h>

struct LIN{
	uint8_t PID;
	uint8_t data[8U];
	uint8_t CRC; 
};

typedef struct LIN lin;

extern lin lin_received;
extern lin lin_transmit;



#endif
