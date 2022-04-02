#include "fifo.h"
//This function get init states for correctly work fifo buffer
void FifoInit(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}
//This function add data to the fifo - buf
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
//This function return first element from fifo - buf
BufDataType Pull(FIFO *buf)
{
	BufDataType currByte = buf->Data[buf->Tail++];
	if (buf->Head == buf->Tail)
	{
		FifoInit(buf);
	}
	return currByte;
}
//This function return size of ring - bufer
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
//This function set head and tail pointer at zero states
void Clear(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}
