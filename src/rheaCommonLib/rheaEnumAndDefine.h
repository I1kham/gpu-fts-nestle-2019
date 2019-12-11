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


typedef struct sFindHardDriveResult
{
	char	drivePath[128];
	char	driveLabel[256];
} rheaFindHardDriveResult;


#endif // _rheaEnumAndDefine_h_

