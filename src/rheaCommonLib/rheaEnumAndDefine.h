#ifndef _rheaEnumAndDefine_h_
#define _rheaEnumAndDefine_h_


enum eThreadError
{
    eThreadError_none = 0,
    eThreadError_invalidStackSize = 1,
    eThreadError_tooMany = 2,
    eThreadError_unknown = 0xff
};

enum eSocketError
{
    eSocketError_none = 0,
    eSocketError_denied = 1,        //Permission to create a socket of the specified type and/or protocol is denied.
    eSocketError_unsupported = 2,   //The implementation does not support the specified address family
    eSocketError_tooMany = 3,       //The per-process limit on the number of open file descriptors has been reached.
    eSocketError_noMem = 4,         //nsufficient memory is available.  The socket cannot be created until sufficient resources are freed.
    eSocketError_addressInUse = 5,
    eSocketError_addressProtected = 6,
    eSocketError_alreadyBound = 7,
    eSocketError_invalidDescriptor = 8,
    eSocketError_errorSettingReadTimeout = 9,
    eSocketError_errorSettingWriteTimeout = 10,
    eSocketError_errorListening = 11,
    eSocketError_no_such_host   = 12,
    eSocketError_connRefused = 13,
    eSocketError_timedOut = 14,
    eSocketError_unknown = 0xff
};


enum eRS232BaudRate
{
	Baud1200 = 1200,
	Baud2400 = 2400,
	Baud4800 = 4800,
	Baud9600 = 9600,
	Baud19200 = 19200,
	Baud38400 = 38400,
	Baud57600 = 57600,
	Baud115200 = 115200,
	Baud230400 = 230400
};

enum eRS232DataBits
{
	Data5 = 5,
	Data6 = 6,
	Data7 = 7,
	Data8 = 8
};

enum eRS232Parity
{
	NoParity = 0,
	EvenParity = 2,
	OddParity = 3
};

enum eRS232StopBits
{
	OneStop = 1,
	TwoStop = 2
};

enum eRS232FlowControl
{
	NoFlowControl = 1,
	HardwareControl = 2
};


typedef struct sFindHardDriveResult
{
	char	drivePath[128];
	char	driveLabel[256];
} rheaFindHardDriveResult;


#endif // _rheaEnumAndDefine_h_

