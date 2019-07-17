#ifdef LINUX
#ifndef _linuxOSSerialPort_h_
#define linuxOSSerialPort_h_
#include "linuxOSInclude.h"
#include "OS/OSEnum.h"

namespace platform
{
    bool            serialPort_open (OSSerialPort *out_serialPort, const char *deviceName,
                                        OSSerialPortConfig::eBaudRate baudRate,
                                        bool RST_on,
                                        bool DTR_on,
                                        OSSerialPortConfig::eDataBits dataBits,
                                        OSSerialPortConfig::eParity parity,
                                        OSSerialPortConfig::eStopBits stop,
                                        OSSerialPortConfig::eFlowControl flowCtrl);

    void            serialPort_close (OSSerialPort &sp);

    void            serialPort_setRTS (OSSerialPort &sp, bool bON_OFF);
    void            serialPort_setDTR (OSSerialPort &sp, bool bON_OFF);

    bool            serialPort_setAsBlocking (OSSerialPort &sp, u8 minNumOfCharToBeReadInOrderToSblock);
    bool            serialPort_setAsNonBlocking (OSSerialPort &sp, u8 numOfDSecToWaitBeforeReturn);

    void            serialPort_flushIO (OSSerialPort &sp);

    u32             serialPort_readBuffer (OSSerialPort &sp, void *out_byteRead, u32 numMaxByteToRead);
    u32             serialPort_writeBuffer (OSSerialPort &sp, const void *buffer, u32 nBytesToWrite);
}

#endif // linuxOSSerialPort_h_
#endif // LINUX
