#ifdef WIN32
#include "winOS.h"
#include "winOSSerialPort.h"
#include "../../rhea.h"


//*****************************************************
bool platform::serialPort_open (OSSerialPort *out_serialPort, const char *deviceName, OSSerialPortConfig::eBaudRate baudRate, bool RST_on, bool DTR_on, OSSerialPortConfig::eDataBits dataBits,
                        OSSerialPortConfig::eParity parity, OSSerialPortConfig::eStopBits stopBits, OSSerialPortConfig::eFlowControl flowCtrl)
{
   
    return false;
}


//*****************************************************
void platform::serialPort_close(OSSerialPort &sp)
{
}

//*****************************************************
void platform::serialPort_setRTS (OSSerialPort &sp, bool bON_OFF)
{
}

//*****************************************************
void platform::serialPort_setDTR (OSSerialPort &sp, bool bON_OFF)
{
}

//*****************************************************
bool platform::serialPort_setAsBlocking (OSSerialPort &sp, u8 minNumOfCharToBeReadInOrderToSblock)
{
    return false;
}

//*****************************************************
bool platform::serialPort_setAsNonBlocking (OSSerialPort &sp, u8 numOfDSecToWaitBeforeReturn)
{
    return false;
}

//*****************************************************
void platform::serialPort_flushIO (OSSerialPort &sp)
{
}

//*****************************************************
u32 platform::serialPort_readBuffer (OSSerialPort &sp, void *out_byteRead, u32 numMaxByteToRead)
{
	return 0;
}

//*****************************************************
u32 platform::serialPort_writeBuffer (OSSerialPort &sp, const void *buffer, u32 nBytesToWrite)
{
	return 0;
}
#endif //WIN32
