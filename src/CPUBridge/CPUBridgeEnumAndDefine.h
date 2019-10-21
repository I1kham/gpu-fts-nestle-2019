#ifndef _CPUBridgeEnumAndDefine_h_
#define _CPUBridgeEnumAndDefine_h_
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/rheaBit.h"

#define		NUM_MAX_SELECTIONS					48
#define		_CPU_MSG_LCD_MAX_LEN_IN_BYTES		64
#define		LCD_BUFFER_SIZE_IN_BYTES			(_CPU_MSG_LCD_MAX_LEN_IN_BYTES+4)
#define		LCD_BUFFER_SIZE_IN_U16				(LCD_BUFFER_SIZE_IN_BYTES/2)
#define		TRANSLATED_LCD_BUFFER_SIZE_IN_U16	(LCD_BUFFER_SIZE_IN_U16 + 16)
#define		VMCDATAFILE_BLOCK_SIZE_IN_BYTE		64
#define		VMCDATAFILE_TOTAL_FILE_SIZE_IN_BYTE	10048

/**********************************************************************
 * Messaggi in/out sul canale di "servizio" di CPUBridge
 */
#define		CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST	0x0001
#define		CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER		0x0002
#define		CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST	0x0003

/**********************************************************************
 *	Notifiche che CPUBridge manda a tutti i suoi subscriber (vedi le fn notify_xxx  in CPUBridge.h) oppure in risposta
 *	ad una specifica richiesta di un subsriber (vedi le fn ask_xxx in CPUBridge.h)
 *	Per convenzione, valori validi sono tra 0x0100 e 0x01FF
 */
#define		CPUBRIDGE_NOTIFY_DYING                      0x0100
#define		CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED          0x0101
#define		CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE        0x0102
#define		CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED         0x0103
#define		CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED      0x0104
#define		CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED     0x0105
#define		CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS     0x0106
#define		CPUBRIDGE_NOTIFY_CPU_FULLSTATE              0x0107
#define		CPUBRIDGE_NOTIFY_CPU_INI_PARAM              0x0108
#define		CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED           0x0109
#define		CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS   0x010A
#define		CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS  0x010B
#define		CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS 0x010C
#define		CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP		0x010D

#define		CPUBRIDGE_NOTIFY_MAX_ALLOWED                0x01FF


 /**********************************************************************
  * msg che i subscriber di CPUBridge possono inviare usando la loro coda di write verso CPUBridge (vedi le fn ask_xxx in CPUBridge.h)
  *	
  */
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION			0x0800
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION				0x0801
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS	0x0802
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE			0x0803
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM			0x0804
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL			0x0805
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES			0x0806
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE			0x0807
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT		0x0808
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE				0x0809
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM			0x080A
#define		CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT                0x080B
#define		CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE				0x080C
#define		CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE				0x080D
#define		CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP			0x080E

namespace cpubridge
{
	enum eCPUCommand
	{
		eCPUCommand_checkStatus_B = 'B',
        eCPUCommand_checkStatus_B_Unicode = 'Z',
        eCPUCommand_initialParam_C = 'C',
		eCPUCommand_restart = 'U',
        eCPUCommand_readDataAudit = 'L',
		eCPUCommand_writeVMCDataFile = 'D',
        eCPUCommand_readVMCDataFile= 'E',
        //eCPUCommand_writeHexFile = 'H'
        //eCPUCommand_readHexFile= 'h',
		eCPUCommand_getVMCDataFileTimeStamp = 'T'
	};

	enum eRunningSelStatus
	{
		eRunningSelStatus_wait = 1,
		eRunningSelStatus_running = 2,
		eRunningSelStatus_finished_KO = 3,
		eRunningSelStatus_finished_OK = 4,
		eRunningSelStatus_runningCanUseStopBtn = 5
	};

	enum eStatoPreparazioneBevanda
	{
		eStatoPreparazioneBevanda_unsupported = 0,
		eStatoPreparazioneBevanda_doing_nothing = 0x01,
		eStatoPreparazioneBevanda_wait = 0x02,
		eStatoPreparazioneBevanda_running = 0x03
	};

	enum eVMCState
	{
		eVMCState_DISPONIBILE = 2,
		eVMCState_PREPARAZIONE_BEVANDA = 3,
		eVMCState_PROGRAMMAZIONE = 4,
		eVMCState_INITIAL_CHECK = 5,
		eVMCState_ERROR = 6,
		eVMCState_LAVAGGIO_MANUALE = 7,
		eVMCState_LAVAGGIO_AUTO = 8,
		eVMCState_RICARICA_ACQUA = 9,
		eVMCState_ATTESA_TEMPERATURA = 10,
		eVMCState_ATTESA_CARICA_MASTER = 11,
		eVMCState_INSTALLAZIONE = 12,
		eVMCState_DISINSTALLAZIONE = 13,
		eVMCState_FINE_INSTALLAZIONE = 14,
		eVMCState_FINE_DISINSTALLAZIONE = 15,
		eVMCState_ENERGY_SAVING = 16,
		eVMCState_LAVAGGIO_SANITARIO = 17,
		eVMCState_DATA_AUDIT = 18,
		eVMCState_COM_ERROR     = 101
	};

    enum eReadDataFileStatus
    {
        eReadDataFileStatus_inProgress = 0,
        eReadDataFileStatus_finishedOK = 1,
        eReadDataFileStatus_finishedKO_cantStart_invalidState = 2,
        eReadDataFileStatus_finishedKO_cpuDidNotAnswer = 3,
		eReadDataFileStatus_finishedKO_unableToCreateFile = 4
    };

	enum eWriteDataFileStatus
	{
		eWriteDataFileStatus_inProgress = 0,
		eWriteDataFileStatus_finishedOK = 1,
		eWriteDataFileStatus_finishedKO_cantStart_invalidState = 2,
		eWriteDataFileStatus_finishedKO_cpuDidNotAnswer = 3,
		eWriteDataFileStatus_finishedKO_unableToCopyFile = 4,
		eWriteDataFileStatus_finishedKO_unableToOpenLocalFile = 5
	};

	struct sSubscriber
	{
		HThreadMsgR	hFromCpuToOtherR;
		HThreadMsgW	hFromCpuToOtherW;
		HThreadMsgR	hFromOtherToCpuR;
		HThreadMsgW	hFromOtherToCpuW;
	};


	struct sCPUParamIniziali
	{
		char	CPU_version[16];
		u8		protocol_version;
		u16		prices[48];
	};

	struct sCPUSelAvailability
	{
		void        reset()														{ rhea::bit::zero(_flag, sizeof(_flag)); }
		bool        isAvail(u8 selNumberStartingFromOne) const					{ return rhea::bit::isSet(_flag, sizeof(_flag), selNumberStartingFromOne - 1); }
		bool        areAllNotAvail() const										{ return (_flag[0] == 0 && _flag[1] == 0); }
		void        setAsAvail(u8 selNumberStartingFromOne)						{ assert(selNumberStartingFromOne > 0); rhea::bit::set(_flag, sizeof(_flag), selNumberStartingFromOne - 1); }
		void        setAsNotAvail(u8 selNumberStartingFromOne)					{ assert(selNumberStartingFromOne > 0); rhea::bit::unset(_flag, sizeof(_flag), selNumberStartingFromOne - 1); }
		const u32*  getBitSequence() const										{ return _flag; }

	public:
		u32         _flag[2];    //1 bit per ogni selezione, non modificare direttamente, usa i metodi forniti qui sopra
	};


	struct sCPULCDMessage
	{
		u16			buffer[TRANSLATED_LCD_BUFFER_SIZE_IN_U16];
		u16			ct;	//num di bytes "buffer" in msgLCG
		u8			importanceLevel;
	};

	struct sCPUStatus
	{
		char							userCurrentCredit[16];
		char							curLangISO[4];
		eVMCState						VMCstate;
		u8								VMCerrorCode;
		u8								VMCerrorType;
		u8								bShowDialogStopSelezione;
		u8								CupAbsentStatus_flag;
		eStatoPreparazioneBevanda		statoPreparazioneBevanda;
		sCPUSelAvailability				selAvailability;
		u16								beepSelezioneLenMSec;
		sCPULCDMessage					LCDMsg;
	};

	struct sCPUVMCDataFileTimeStamp
	{
				sCPUVMCDataFileTimeStamp()										{ setInvalid(); }
				
		void	setInvalid()													{ memset(data, 0xFF, SIZE_OF_BUFFER); data[1] = 0xfe; data[3] = 0xfc; }
		u8		readFromBuffer(const void *buffer)								{ memcpy(data, buffer, SIZE_OF_BUFFER); return SIZE_OF_BUFFER; }
		u8		writeToBuffer(void *buffer) const								{ memcpy(buffer, data, SIZE_OF_BUFFER); return SIZE_OF_BUFFER; }
		u8		readFromFile (FILE *f)											{ fread(data, SIZE_OF_BUFFER, 1, f); return SIZE_OF_BUFFER; }
		u8		writeToFile(FILE *f) const										{ fwrite(data, SIZE_OF_BUFFER, 1, f); return SIZE_OF_BUFFER; }

		bool	areEqual(const sCPUVMCDataFileTimeStamp &b) const				{ return (memcmp(data, b.data, SIZE_OF_BUFFER) == 0); }
		u8		getLenInBytes() const											{ return SIZE_OF_BUFFER; }

	private:
		static const u8 SIZE_OF_BUFFER = 6;
		u8	data[SIZE_OF_BUFFER];
	};
} // namespace cpubridge

#endif // _CPUBridgeEnumAndDefine_h_
