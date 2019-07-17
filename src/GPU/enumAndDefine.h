#ifndef ENUMANDDEFINE_H
#define ENUMANDDEFINE_H

/*******************************************************
 * Serial RS232 enum
 *******************************************************/
enum eRS232_BaudRate
{
    eRS232_Baud1200 = 1200,
    eRS232_Baud2400 = 2400,
    eRS232_Baud4800 = 4800,
    eRS232_Baud9600 = 9600,
    eRS232_Baud19200 = 19200,
    eRS232_Baud38400 = 38400,
    eRS232_Baud57600 = 57600,
    eRS232_Baud115200 = 115200,
    eRS232_Baud230400 = 230400
};

enum eRS232_DataBits
{
    eRS232_Data5 = 5,
    eRS232_Data6 = 6,
    eRS232_Data7 = 7,
    eRS232_Data8 = 8
};

enum eRS232_Parity
{
    eRS232_NoParity = 0,
    eRS232_EvenParity = 2,
    eRS232_OddParity = 3
};

enum eRS232_StopBits
{
    eRS232_OneStop = 1,
    eRS232_TwoStop = 2
};

enum eRS232_FlowControl
{
    eRS232_NoFlowControl = 1,
    eRS232_HardwareControl = 2
};





#endif // ENUMANDDEFINE_H

