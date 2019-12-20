#ifndef _enumAndDefine_h_
#define _enumAndDefine_h_
#include "winTerminal.h"

#define		MSGQ_USER_INPUT		0x01
#define		MSGQ_DIE			0x02


struct sIPAddressAndSubnetMask
{
	char	ip[16];
	char	subnetMask[16];
};

#endif // _enumAndDefine_h_
