#ifndef _fifo_h_
#define _fifo_h_
#include "main.h"

//User defines
#define BufDataType uint8_t
#define BufSize 	  0x40U

struct fifo
{
	BufDataType Data[BufSize];
	uint16_t Head;
	uint16_t Tail;
};

typedef struct fifo FIFO;

void FifoInit(FIFO *buf);

void Push(FIFO *buf, BufDataType data);

BufDataType Pull(FIFO *buf);

uint16_t GetSize(FIFO *buf);

void Clear(FIFO *buf);
#endif
