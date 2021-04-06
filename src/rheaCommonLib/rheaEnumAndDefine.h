#ifndef _rheaEnumAndDefine_h_
#define _rheaEnumAndDefine_h_
#include "rheaDataTypes.h"

enum class eThreadError : u8
{
    none                = 0,
    invalidStackSize    = 1,
    tooMany             = 2,
    unknown             = 0xff
};

enum class eSocketError: u8
{
    none                        = 0,
    denied                      = 1,    //Permission to create a socket of the specified type and/or protocol is denied.
    unsupported                 = 2,    //The implementation does not support the specified address family
    tooMany                     = 3,    //The per-process limit on the number of open file descriptors has been reached.
    noMem                       = 4,    //insufficient memory is available.  The socket cannot be created until sufficient resources are freed.
    addressInUse                = 5,
    addressProtected            = 6,
    alreadyBound                = 7,
    invalidDescriptor           = 8,
    errorSettingReadTimeout     = 9,
    errorSettingWriteTimeout    = 10,
    errorListening              = 11,
    no_such_host                = 12,
    connRefused                 = 13,
    timedOut                    = 14,
    invalidParameter            = 15,
    unknown                     = 0xff
};


enum class eRS232BaudRate: u32
{
    b1200 = 1200,
    b2400 = 2400,
    b4800 = 4800,
    b9600 = 9600,
    b19200 = 19200,
    b38400 = 38400,
    b57600 = 57600,
    b115200 = 115200,
    b230400 = 230400
};

enum class eRS232DataBits: u8
{
    b5 = 5,
    b6 = 6,
    b7 = 7,
    b8 = 8
};

enum class eRS232Parity : u8
{
    No = 0,
    Even = 2,
    Odd = 3
};

enum class eRS232StopBits: u8
{
    One = 1,
    Two = 2
};

enum class eRS232FlowControl: u8
{
    No = 1,
    HW = 2
};


typedef struct sFindHardDriveResult
{
	u8		utf8_drivePath[32];
	u8		utf8_driveLabel[256];
} rheaFindHardDriveResult;



struct sNetworkAdapterInfo
{
    char    name[32];
	char	ip[16];
	char	subnetMask[16];
};
#endif // _rheaEnumAndDefine_h_

