#ifndef _raspiCore_h_
#define _raspiCore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaExternalSerialAPI/ESAPI.h"

namespace raspi
{
	/*************************************************************
	 * Core
	 *
	 *	Apre una socket TCP/IP sulla porta 2280 e rimane in attesa di connessioni.
	 *	Ogni volta che una socket si connette, crea un record in [clientList] e da li in poi tutti i dati in arrivo su quella socket vengono
	 *	inviati lungo la rs232 alla GPU.
	 *	Il canale funziona anche nell'altro senso ovvero, i dati in transito lungo la rs232 provenienti dalla GPU, vengono inviati lungo la socket
	 *	L'idea è di mascherare il fatto di avere una rs232 in mezzo e far credere al client che si collega alla 2280 di stare direttamente parlando con la GPU dall'altro lato
	 *	della rs232
	 */
	class Core
	{
	public:
						Core ();
						~Core ()																{ priv_close(); }

		void            useLogger (rhea::ISimpleLogger *loggerIN);

		bool			open (const char *serialPort);
		void			run ();

	private:
		static const u8		VER_MAJOR = 1;
		static const u8		VER_MINOR = 0;
        static const u32	WAITGRP_SOCKET2280          = 0xFFFFFFFF;
        static const u32	WAITGRP_SOCKET2281          = 0xFFFFFFFE;
        static const u32	WAITGRP_RS232               = 0xFFFFFFFD;
        static const u32	WAITGRP_SOK_FROM_REST_API   = 0xFFFFFFFC;
		
		static const u16	SIZE_OF_RS232BUFFEROUT	= 4*1024;
		static const u16	SIZE_OF_RS232BUFFERIN	= 4*1024;
        static const u16	SOK_BUFFER_SIZE			= 4*1024;
        static const u16	SOK2281_BUFFERIN_SIZE   = 2*1024;
        static const u16	SOK2281_BUFFEROUT_SIZE  = 8*1024;

	private:
		struct sConnectedSocket
		{
			u32			uid;
			OSSocket	sok;
		};

        struct sHotspot
        {
            u8      bIsOn;
            u64     timeToTurnOnMSec;
            u8      wifiIP[4];
            u8      ssid[64];

            void    turnOFF()   { system ("sudo systemctl stop hostapd"); }
            void    turnON()    { system ("sudo systemctl start hostapd"); }
        };

		struct sBuffer
		{
		public:
			u8	*buffer;
			u32	numBytesInBuffer;
			u32	SIZE;

		public:
					sBuffer()														{ buffer = NULL; SIZE = 0; numBytesInBuffer = 0; }

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
			void	reset() { this->numBytesInBuffer = 0; }
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

        struct sClient2281
        {
            OSSocket    sok;
            u64         lastTimeRcvMSec;
        };

	private:
		struct sFileUpload
		{
			FILE			*f;
			u32				totalFileSizeBytes;
			u16				packetSizeBytes;
			u32				rcvBytesSoFar;
			u64				lastTimeRcvMSec;
		};

	private:
		void					priv_close ();
		u32						priv_esapi_buildMsg (u8 c1, u8 c2, const u8* optionalData, u32 numOfBytesInOptionalData, u8 *out_buffer, u32 sizeOfOutBuffer);
		bool					priv_esapi_isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse);

		void					priv_2280_accept();
		void					priv_2280_onIncomingData (OSSocket &sok, u32 uid);
		void					priv_2280_onClientDisconnected (OSSocket &sok, u32 uid);
		sConnectedSocket*		priv_2280_findClientByUID (u32 uid);

		void					priv_rs232_handleIncomingData (sBuffer &b);
		void					priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend);
		bool					priv_rs232_handleCommand_A (sBuffer &b);
		bool					priv_rs232_handleCommand_R (sBuffer &b);
        bool                    priv_rs232_waitAnswer(u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLenLSB, u8 whichByteContainsAdditionMsgLenMSB, u8 *answerBuffer, u32 timeoutMSec);

		void					priv_identify_run();
		bool					priv_identify_waitAnswer(u8 command, u8 code, u8 len, u8 *answerBuffer, u32 timeoutMSec);

		void					priv_boot_run();
		void					priv_boot_rs232_handleCommunication (sBuffer &b);
		u32						priv_boot_buildMsgBuffer (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData);
		void					priv_boot_buildMsgBufferAndSend (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData);
        void                    priv_boot_finalizeGUITSInstall (const u8* const pathToGUIFolder);

        void                    priv_openSocket2280();
        void                    priv_openSocket2281();
        void                    priv_2281_accept();
        void                    priv_2281_removeOldConnection(u64 timeNowMSec);
        void                    priv_2281_handle_restAPI (OSSocket &sok);
        void                    priv_2281_handle_singleCommand (OSSocket &sok, const u8 *command, rhea::string::utf8::Iter *params);
        bool                    priv_2281_utils_match (const u8 *command, u32 commandLen, const char *match) const;
        void                    priv_2281_sendAnswer (OSSocket &sok, const u8 *data, u16 sizeOfData);

        void                    priv_runTS();
        void                    priv_runTP();

        void                    priv_REST_getCPULCDMsg (OSSocket &sok);
        void                    priv_REST_getSelAvail (OSSocket &sok);
        void                    priv_REST_get12LEDStatus (OSSocket &sok);
        void                    priv_REST_getSelPrices (OSSocket &sok);
        void                    priv_REST_sendButtonPress (OSSocket &sok, rhea::string::utf8::Iter *params);
        void                    priv_REST_startSel (OSSocket &sok, rhea::string::utf8::Iter *params);
        void                    priv_REST_getSelStatus (OSSocket &sok);

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		rhea::NullLogger        nullLogger;
		OSSocket				sok2280;
        OSSocket                sok2281;
		OSSerialPort			com;
		OSWaitableGrp			waitableGrp;
		u32						sok2280NextID;
		u8						*rs232BufferOUT;
		sBuffer					rs232BufferIN;
		u8						*sok2280Buffer;
        u8                      *sok2281BufferIN;
        u8                      *sok2281BufferOUT;
		rhea::FastArray< sConnectedSocket>			clientList;
		u8						reportedESAPIVerMajor;
		u8						reportedESAPIVerMinor;
		esapi::eGPUType			reportedGPUType;
		bool					bQuit;
        sHotspot                hotspot;
		sFileUpload				fileUpload;
        rhea::FastArray<sClient2281>    client2281List;

	};
} //namespace raspi

#endif //_raspiCore_h_
