#ifndef _ESAPIEnumAndDefine_h_
#define _ESAPIEnumAndDefine_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"


#define		ESAPI_SERVICECH_SUBSCRIPTION_REQUEST	0x0001
#define		ESAPI_SERVICECH_SUBSCRIPTION_ANSWER		0x0003


#define		ESAPI_ASK_UNSUBSCRIBE					0x1000
#define		ESAPI_ASK_GET_MODULE_TYPE_AND_VER		0x1001
#define		ESAPI_ASK_RASPI_GET_IPandSSID			0x1002
#define		ESAPI_ASK_RASPI_START					0x1003



#define		ESAPI_NOTIFY_MODULE_TYPE_AND_VER		0x2000
#define		ESAPI_NOTIFY_RASPI_IPandSSID			0x2001
#define		ESAPI_NOTIFY_RASPI_STARTED				0x2002


namespace esapi
{
	enum eExternalModuleType
	{
		eExternalModuleType_unknown			= 0x00,
		eExternalModuleType_rasPI_wifi_REST = 0x01
	};

	enum eGPUType
	{
		eGPUType_unknown	= 0x00,
		eGPUType_TS			= 0x01,
		eGPUType_TP			= 0x02
	};

	struct sESAPIModule
	{
		eExternalModuleType type;
		u8					verMajor;
		u8					verMinor;
	};

	struct sBuffer
	{
	public:
		u8	*buffer;
		u32	numBytesInBuffer;
		u32	SIZE;

	public:
		sBuffer() { buffer = NULL; SIZE = 0; numBytesInBuffer = 0; }

		void	alloc (rhea::Allocator *allocator, u16 max_size)
				{
					this->SIZE = max_size;
					this->numBytesInBuffer = 0;
					this->buffer = (u8*)RHEAALLOC(allocator, max_size);
				}
		void	free (rhea::Allocator *allocator)
				{
					this->numBytesInBuffer = 0;
					this->SIZE = 0;
					if (NULL != this->buffer)
						RHEAFREE(allocator, this->buffer);
					this->buffer = NULL;
				}

		bool	appendU8 (u8 d)
				{
					if (this->numBytesInBuffer + 1 > SIZE)
					{
						DBGBREAK;
						return false;
					}
                    buffer[numBytesInBuffer++] = d;
					return true;
				}
		bool	appendU16 (u16 d)
				{
					if (this->numBytesInBuffer + 2 > SIZE)
					{
						DBGBREAK;
						return false;
					}
					buffer[numBytesInBuffer++] = (u8)((d & 0xff00) >> 8);
					buffer[numBytesInBuffer++] = (u8)(d & 0x00ff);
					return true;
				}
		bool	append (const void *src, u32 numBytesToAppend)
				{
					if (this->numBytesInBuffer + numBytesToAppend > SIZE)
					{
						DBGBREAK;
						return false;
					}
					memcpy (&buffer[numBytesInBuffer], src, numBytesToAppend);
					numBytesInBuffer += numBytesToAppend;
					return true;
				}
		void	reset()															{ this->numBytesInBuffer = 0; }
		void	removeFirstNBytes (u32 n)
				{
					if (n > this->numBytesInBuffer)
					{
						DBGBREAK;
						n = this->numBytesInBuffer;
					}
					const u32 nBytesLeft = this->numBytesInBuffer - n;
					if (nBytesLeft)
						memcpy (this->buffer, &this->buffer[n], nBytesLeft);
					this->numBytesInBuffer = nBytesLeft;				
				}
	};

	struct sSubscription
	{
		OSEvent		hEvent;
		cpubridge::sSubscriber	q;
	};

	
} // namespace esapi

#endif // _ESAPIEnumAndDefine_h_

