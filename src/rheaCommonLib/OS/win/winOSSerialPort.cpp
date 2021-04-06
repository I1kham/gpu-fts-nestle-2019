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
bool platform::serialPort_open (OSSerialPort *out_serialPort, const char *deviceName, eRS232BaudRate baudRate, bool RST_on, bool DTR_on, eRS232DataBits dataBits,
                        eRS232Parity parity, eRS232StopBits stopBits, eRS232FlowControl flowCtrl, bool bBlocking)
{
	//CreateFile(“\\\\.\\COM24”
	wchar_t wctemp[128];
	win32::utf8_towchar ((const u8*)deviceName, u32MAX, wctemp, sizeof(wctemp));
	out_serialPort->hComm = CreateFile (wctemp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (out_serialPort->hComm == INVALID_HANDLE_VALUE)
		return false;

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	GetCommState (out_serialPort->hComm, &dcbSerialParams);

	switch (baudRate)
	{
		case eRS232BaudRate::b1200: dcbSerialParams.BaudRate = CBR_1200; break;
		case eRS232BaudRate::b2400: dcbSerialParams.BaudRate = CBR_2400; break;
		case eRS232BaudRate::b4800: dcbSerialParams.BaudRate = CBR_4800; break;
		case eRS232BaudRate::b9600: dcbSerialParams.BaudRate = CBR_9600; break;
		case eRS232BaudRate::b19200: dcbSerialParams.BaudRate = CBR_19200; break;
		case eRS232BaudRate::b38400: dcbSerialParams.BaudRate = CBR_38400; break;
		case eRS232BaudRate::b57600: dcbSerialParams.BaudRate = CBR_57600; break;
		case eRS232BaudRate::b115200: dcbSerialParams.BaudRate = CBR_115200; break;
		case eRS232BaudRate::b230400: dcbSerialParams.BaudRate = CBR_256000; break;
	}

	switch (dataBits)
	{
	case eRS232DataBits::b5: dcbSerialParams.ByteSize = 5; break;
	case eRS232DataBits::b6: dcbSerialParams.ByteSize = 6; break;
	case eRS232DataBits::b7: dcbSerialParams.ByteSize = 7; break;
	case eRS232DataBits::b8: dcbSerialParams.ByteSize = 8; break;
	}

	switch (stopBits)
	{
		case eRS232StopBits::One: dcbSerialParams.StopBits = ONESTOPBIT; break;
		case eRS232StopBits::Two: dcbSerialParams.StopBits = TWOSTOPBITS; break;
	}

	switch (parity)
	{
		case eRS232Parity::No: dcbSerialParams.Parity = NOPARITY; break;
		case eRS232Parity::Even: dcbSerialParams.Parity = EVENPARITY; break;
		case eRS232Parity::Odd: dcbSerialParams.Parity = ODDPARITY; break;
	}
	
	if (!SetCommState(out_serialPort->hComm, &dcbSerialParams))
		return false;


	COMMTIMEOUTS timeouts = { 0 };
	if (!bBlocking)
	{
		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
		timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds
	}
	else
	{
		timeouts.ReadIntervalTimeout = 50; // in milliseconds
		timeouts.ReadTotalTimeoutConstant = 50; // in milliseconds
		timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
		timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
		timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds
	}
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
