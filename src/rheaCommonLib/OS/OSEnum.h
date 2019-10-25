#ifndef _osEnum_h_
#define _osEnum_h_

namespace OSSerialPortConfig
{
    enum eBaudRate
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

    enum eDataBits
    {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8
    };

    enum eParity
    {
        NoParity = 0,
        EvenParity = 2,
        OddParity = 3
    };

    enum eStopBits
    {
        OneStop = 1,
        TwoStop = 2
    };

    enum eFlowControl
    {
        NoFlowControl = 1,
        HardwareControl = 2
    };
}

#endif //_osEnum_h_
