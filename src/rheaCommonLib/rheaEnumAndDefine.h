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
    eSocketError_invalidParameter = 15,
    eSocketError_unknown = 0xff
};


enum eRS232BaudRate
{
    eRS232BaudRate_1200 = 1200,
    eRS232BaudRate_2400 = 2400,
    eRS232BaudRate_4800 = 4800,
    eRS232BaudRate_9600 = 9600,
    eRS232BaudRate_19200 = 19200,
    eRS232BaudRate_38400 = 38400,
    eRS232BaudRate_57600 = 57600,
    eRS232BaudRate_115200 = 115200,
    eRS232BaudRate_230400 = 230400
};

enum eRS232DataBits
{
    eRS232DataBits_5 = 5,
    eRS232DataBits_6 = 6,
    eRS232DataBits_7 = 7,
    eRS232DataBits_8 = 8
};

enum eRS232Parity
{
    eRS232Parity_No = 0,
    eRS232Parity_Even = 2,
    eRS232Parity_Odd = 3
};

enum eRS232StopBits
{
    eRS232StopBits_One = 1,
    eRS232StopBits_Two = 2
};

enum eRS232FlowControl
{
    eRS232FlowControl_No = 1,
    eRS232FlowControl_HW = 2
};


typedef struct sFindHardDriveResult
{
	char	drivePath[128];
	char	driveLabel[256];
} rheaFindHardDriveResult;


#endif // _rheaEnumAndDefine_h_

