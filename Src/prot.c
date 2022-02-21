#include "prot.h"

enum avCommands GetCommand(uint8_t currByte)
{
	enum avCommands currCommand = none_command;
    switch (currByte)
    {
    case 'b':
        currCommand = setBaud;
        break;

    case 'B':
        currCommand = setBaud;
        break;

    case 'e':
        currCommand = setCRC;
        break;

    case 'E':
        currCommand = setCRC;
        break;

    case 'c':
        currCommand = setFilter;
        break;

    case 'C':
        currCommand = setFilter;
        break;

    case 'i':
        currCommand = getInfo;
        break;

    case 'I':
        currCommand = getInfo;
        break;

    case 's':
        currCommand = sendSlave;
        break;

    case 'S':
        currCommand = sendSlave;
        break;

    case 'm':
        currCommand = sendMaster;
        break;

    case 'M':
        currCommand = sendMaster;
        break;
    }
		return currCommand;
}

