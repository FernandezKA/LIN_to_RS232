#include "fifo.h"

void FifoInit(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}

void Push(FIFO *buf, BufDataType data)
{
	if (GetSize(buf) == BufSize)
	{
		buf->Head = 0x00;
		buf->Data[buf->Head++] = data;
	}
	else
	{
		buf->Data[buf->Head++] = data;
	}
}

BufDataType Pull(FIFO *buf)
{
	BufDataType currByte = buf->Data[buf->Tail++];
	if (buf->Head == buf->Tail)
	{
		FifoInit(buf);
	}
	return currByte;
}

uint16_t GetSize(FIFO *buf)
{
	if (buf->Head > buf->Tail)
	{
		return buf->Head - buf->Tail;
	}
	else
	{
		return 0;
	}
}

void Clear(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}
