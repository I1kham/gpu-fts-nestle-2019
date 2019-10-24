#ifdef WIN32
#include "winOS.h"
#include "winOSSerialPort.h"
#include "../../rhea.h"

//*****************************************************
void platform::serialPort_setInvalid(OSSerialPort &sp)
{
	sp.hComm = INVALID_HANDLE_VALUE;
}

//*****************************************************
bool platform::serialPort_isInvalid(const OSSerialPort &sp)
{
	return (sp.hComm == INVALID_HANDLE_VALUE);
}

//*****************************************************
bool platform::serialPort_open (OSSerialPort *out_serialPort, const char *deviceName, OSSerialPortConfig::eBaudRate baudRate, bool RST_on, bool DTR_on, OSSerialPortConfig::eDataBits dataBits,
                        OSSerialPortConfig::eParity parity, OSSerialPortConfig::eStopBits stopBits, OSSerialPortConfig::eFlowControl flowCtrl, bool bBlocking)
{
	//per ora l'implementazione della porta NON BLOCCANTE non la faccio
	if (!bBlocking)
	{
		DBGBREAK;
		return false;
	}

	//CreateFile(“\\\\.\\COM24”
	out_serialPort->hComm = CreateFile (deviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (out_serialPort->hComm == INVALID_HANDLE_VALUE)
		return false;

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	GetCommState (out_serialPort->hComm, &dcbSerialParams);

	switch (baudRate)
	{
		case OSSerialPortConfig::Baud1200: dcbSerialParams.BaudRate = CBR_1200; break;
		case OSSerialPortConfig::Baud2400: dcbSerialParams.BaudRate = CBR_2400; break;
		case OSSerialPortConfig::Baud4800: dcbSerialParams.BaudRate = CBR_4800; break;
		case OSSerialPortConfig::Baud9600: dcbSerialParams.BaudRate = CBR_9600; break;
		case OSSerialPortConfig::Baud19200: dcbSerialParams.BaudRate = CBR_19200; break;
		case OSSerialPortConfig::Baud38400: dcbSerialParams.BaudRate = CBR_38400; break;
		case OSSerialPortConfig::Baud57600: dcbSerialParams.BaudRate = CBR_57600; break;
		case OSSerialPortConfig::Baud115200: dcbSerialParams.BaudRate = CBR_115200; break;
		case OSSerialPortConfig::Baud230400: dcbSerialParams.BaudRate = CBR_256000; break;
	}

	switch (dataBits)
	{
	case OSSerialPortConfig::Data5: dcbSerialParams.ByteSize = 5; break;
	case OSSerialPortConfig::Data6: dcbSerialParams.ByteSize = 6; break;
	case OSSerialPortConfig::Data7: dcbSerialParams.ByteSize = 7; break;
	case OSSerialPortConfig::Data8: dcbSerialParams.ByteSize = 8; break;
	}

	switch (stopBits)
	{
		case OSSerialPortConfig::OneStop: dcbSerialParams.StopBits = ONESTOPBIT; break;
		case OSSerialPortConfig::TwoStop: dcbSerialParams.StopBits = TWOSTOPBITS; break;
	}

	switch (parity)
	{
		case OSSerialPortConfig::NoParity: dcbSerialParams.Parity = NOPARITY; break;
		case OSSerialPortConfig::EvenParity: dcbSerialParams.Parity = EVENPARITY; break;
		case OSSerialPortConfig::OddParity: dcbSerialParams.Parity = ODDPARITY; break;
	}
	
	if (!SetCommState(out_serialPort->hComm, &dcbSerialParams))
		return false;


	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 50; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

	if (!SetCommTimeouts(out_serialPort->hComm, &timeouts))
		return false;

	return true;
}


//*****************************************************
void platform::serialPort_close(OSSerialPort &sp)
{
	CloseHandle(sp.hComm);
	sp.hComm = INVALID_HANDLE_VALUE;
}

//*****************************************************
void platform::serialPort_setRTS (OSSerialPort &sp, bool bON_OFF)
{
	if (bON_OFF)
		EscapeCommFunction(sp.hComm, SETRTS);
	else
		EscapeCommFunction(sp.hComm, CLRRTS);
}

//*****************************************************
void platform::serialPort_setDTR (OSSerialPort &sp, bool bON_OFF)
{
	if (bON_OFF)
		EscapeCommFunction(sp.hComm, SETDTR);
	else
		EscapeCommFunction(sp.hComm, CLRDTR);
}

//*****************************************************
void platform::serialPort_flushIO (OSSerialPort &sp)
{
	
}

//*****************************************************
u32 platform::serialPort_readBuffer (OSSerialPort &sp, void *out_byteRead, u32 numMaxByteToRead)
{
	DWORD nRead = 0;
	if (!ReadFile(sp.hComm, out_byteRead, numMaxByteToRead, &nRead, NULL))
		return 0;
	return (u32)nRead;
}

//*****************************************************
u32 platform::serialPort_writeBuffer (OSSerialPort &sp, const void *buffer, u32 nBytesToWrite)
{
	DWORD nWritten = 0;
	if (!WriteFile (sp.hComm, buffer, nBytesToWrite, &nWritten, NULL))
		return 0;
	return (u32)nWritten;
}
#endif //WIN32
