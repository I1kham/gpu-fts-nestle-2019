#ifndef _CPUBridgeEnumAndDefine_h_
#define _CPUBridgeEnumAndDefine_h_
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/rheaBit.h"

#define		NUM_MAX_SELECTIONS					48
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
#define		CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS		0x010E
#define		CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS			0x010F
#define		CPUBRIDGE_NOTIFY_WRITE_PARTIAL_VMCDATAFILE_PROGRESS  0x0110
#define		CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET			0x0111
#define		CPUBRIDGE_NOTIFY_ALL_DECOUNTER_VALUES		0x0112
#define		CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO		0x0113
#define		CPUBRIDGE_NOTIFY_ATTIVAZIONE_MOTORE			0x0114	
#define		CPUBRIDGE_NOTIFY_CALCOLA_IMPULSI_GRUPPO_STARTED	0x0115
#define		CPUBRIDGE_NOTIFY_STATO_CALCOLO_IMPULSI_GRUPPO	0x0116
#define		CPUBRIDGE_NOTIFY_SET_FATTORE_CALIB_MOTORE		0x0117
#define		CPUBRIDGE_NOTIFY_STATO_GRUPPO					0x0118
#define		CPUBRIDGE_NOTIFY_GET_TIME						0x0119
#define		CPUBRIDGE_NOTIFY_GET_DATE						0x011A
#define		CPUBRIDGE_NOTIFY_SET_TIME						0x011B
#define		CPUBRIDGE_NOTIFY_SET_DATE						0x011C
#define		CPUBRIDGE_NOTITFY_POSIZIONE_MACINA				0x011D
#define		CPUBRIDGE_NOTITFY_MOTORE_MACINA					0x011E
#define		CPUBRIDGE_NOTITFY_TEST_SELECTION				0x011F
#define		CPUBRIDGE_NOTITFY_NOMI_LINGUE_CPU				0x0120
#define		CPUBRIDGE_NOTITFY_EVA_RESET_PARTIALDATA			0x0121
#define		CPUBRIDGE_NOTITFY_GET_VOLT_AND_TEMP				0x0122
#define		CPUBRIDGE_NOTITFY_GET_OFF_REPORT				0x0123
#define		CPUBRIDGE_NOTITFY_GET_LAST_FLUX_INFORMATION		0x0124
#define		CPUBRIDGE_NOTITFY_GET_CPU_STRING_MODEL_AND_VER	0x0125
#define		CPUBRIDGE_NOTITFY_CPU_START_MODEM_TEST			0x0126
#define		CPUBRIDGE_NOTITFY_CPU_EVA_RESET_TOTALS			0x0127
#define		CPUBRIDGE_NOTIFY_GET_TIME_LAVSAN_CAPPUCINATORE	0x0128
#define		CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_GRUPPO	0x0129
#define		CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_MOTORIDUTTORE		0x012A
#define		CPUBRIDGE_NOTIFY_GETSTATUS_TEST_ASSORBIMENTO_GRUPPO			0x012B
#define		CPUBRIDGE_NOTIFY_GETSTATUSTEST_ASSORBIMENTO_MOTORIDUTTORE	0x012C
#define		CPUBRIDGE_NOTIFY_MILKER_VER						0x012D



#define		CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_ARE_YOU_THERE		0x01FE
#define		CPUBRIDGE_NOTIFY_MAX_ALLOWED						0x01FF


 /**********************************************************************
  * msg che i subscriber di CPUBridge possono inviare usando la loro coda di write verso CPUBridge (vedi le fn ask_xxx in CPUBridge.h)
  *	
  */
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION							0x0800
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION								0x0801
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS					0x0802
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE							0x0803
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM							0x0804
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL							0x0805
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES							0x0806
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE							0x0807
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT						0x0808
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE								0x0809
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM							0x080A
#define		CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT                				0x080B
#define		CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE								0x080C
#define		CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE								0x080D
#define		CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP							0x080E
#define		CPUBRIDGE_SUBSCRIBER_ASK_WRITE_CPUFW									0x080F
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD    						0x0810
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_KEEP_SENDING_BUTTON_NUM					0x0811
#define		CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE						0x0812
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER									0x0813
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_ALL_DECOUNTER_VALUES						0x0814
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_EXTENDED_CONFIG_INFO						0x0815
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_ATTIVAZIONE_MOTORE							0x0816
#define		CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO							0x0817
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_CALCOLO_IMPULSI_GRUPPO				0x0818
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE						0x0819
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_GRUPPO								0x081A
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_TIME										0x081B
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_DATE										0x081C
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_TIME										0x081D
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_DATE										0x081E
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA							0x081F
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA								0x0820
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA							0x0821
#define		CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE									0x0822
#define		CPUBRIDGE_SUBSCRIBER_ASK_NOMI_LINGUE_CPU								0x0823
#define		CPUBRIDGE_SUBSCRIBER_ASK_DISINSTALLAZIONE								0x0824
#define		CPUBRIDGE_SUBSCRIBER_ASK_RICARICA_FASCIA_ORARIA_FV						0x0825
#define		CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_PARTIALDATA							0x0826
#define		CPUBRIDGE_SUBSCRIBER_ASK_DIE											0x0827
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_VOLT_AND_TEMP								0x0828
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_OFF_REPORT									0x0829
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_FLUX_INFORMATION						0x082A
#define		CPUBRIDGE_SUBSCRIBER_ASK_SHOW_STR_VERSION_AND_MODEL						0x082B
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_STR_VERSION_AND_MODEL					0x082C
#define		CPUBRIDGE_SUBSCRIBER_ASK_DA3_SYNC										0x082D
#define		CPUBRIDGE_SUBSCRIBER_ASK_START_MODEM_TEST								0x082E
#define		CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_TOTALS								0x082F
#define		CPUBRIDGE_SUBSCRIBER_ASK_TIME_NEXT_LAVSAN_CAPPUCC						0x0830
#define		CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_GRUPPO					0x0831
#define		CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_MOTORIDUTTORE			0x0832
#define		CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_GRUPPO					0x0833
#define		CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE			0x0834
#define		CPUBRIDGE_SUBSCRIBER_ASK_MILKER_VER										0x0835
#define		CPUBRIDGE_SUBSCRIBER_ASK_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED	0x0836


 /**********************************************************************
  * msg che CPUBridge e il modulo MITM di rasPI possono scambiarsi
  */
#define		CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_ARE_YOU_THERE						0x0A00
#define		CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_START_SOCKETBRIDGE					0x0A01


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
        eCPUCommand_getVMCDataFileTimeStamp = 'T',
        eCPUCommand_programming = 'P',
		eCPUCommand_writePartialVMCDataFile = 'X',
		eCPUCommand_getExtendedConfigInfo = 'c',
		eCPUCommand_getMilkerVer = 'M',
        eCPUCommand_startSelWithPaymentAlreadyHandled_V = 'V',
        eCPUCommand_rasPI_MITM = 'W'
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
        eVMCState_TEST_DB = 17,
		eVMCState_DATA_AUDIT = 18,
        eVMCState_LAVAGGIO_SANITARIO = 20,
		eVMCState_TEST_ATTUATORE_SELEZIONE = 21,
		eVMCState_TEST_MODEM = 22,
		eVMCState_LAVAGGIO_MILKER = 23,
		
		eVMCState_COM_ERROR     = 101,
		eVMCState_REG_APERTURA_MACINA = 102,
		eVMCState_COMPATIBILITY_CHECK = 103,
		eVMCState_CPU_NOT_SUPPORTED = 104,
		eVMCState_DA3_SYNC = 105

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
		eWriteDataFileStatus_finishedKO_unableToOpenLocalFile = 5,
        eWriteDataFileStatus_finishedKO_cpuDidNotAnswer2 = 6,
	};

	enum eWriteCPUFWFileStatus
	{
		eWriteCPUFWFileStatus_inProgress_erasingFlash = 0,
		eWriteCPUFWFileStatus_inProgress,
		eWriteCPUFWFileStatus_finishedOK,
		eWriteCPUFWFileStatus_finishedKO_cantStart_invalidState,
		eWriteCPUFWFileStatus_finishedKO_unableToCopyFile,
		eWriteCPUFWFileStatus_finishedKO_unableToOpenLocalFile,
		eWriteCPUFWFileStatus_finishedKO_k_notReceived,
		eWriteCPUFWFileStatus_finishedKO_M_notReceived,
		eWriteCPUFWFileStatus_finishedKO_h_notReceived,
		eWriteCPUFWFileStatus_finishedKO_generalError
	};

	enum eCPUMachineType
	{
		eCPUMachineType_instant = 0x00,
        eCPUMachineType_espresso1 = 0x01,
        eCPUMachineType_espresso2 = 0x02,
        eCPUMachineType_unknown = 0xff
	};

	enum ePaymentMode
	{
		ePaymentMode_normal = 0,
		ePaymentMode_freevend = 1,
		ePaymentMode_testvend = 2,
		ePaymentMode_invalid = 0xff
	};

	enum eGPUPaymentType
	{
		eGPUPaymentType_unknown = 0,
		eGPUPaymentType_alipayChina = 1,
		eGPUPaymentType_invalid = 0xff
	};

    enum eCPUProgrammingCommand
    {
        eCPUProgrammingCommand_enterProg = 0x01,
        eCPUProgrammingCommand_cleaning = 0x02,
		eCPUProgrammingCommand_querySanWashingStatus = 0x03,
		eCPUProgrammingCommand_setDecounter = 0x04,
		//ELIMINATO eCPUProgrammingCommand_resetCounter = 0x05,
		eCPUProgrammingCommand_getAllDecounterValues = 0x06,
		eCPUProgrammingCommand_getTime = 0x07,
		eCPUProgrammingCommand_getDate = 0x08,
		eCPUProgrammingCommand_setTime = 0x09,
		eCPUProgrammingCommand_setDate = 0x0A,
		eCPUProgrammingCommand_getStatoGruppo = 0x0B,
		eCPUProgrammingCommand_attivazioneMotore = 0x0C,
		eCPUProgrammingCommand_calcolaImpulsiMacina = 0x0D,
		eCPUProgrammingCommand_getStatoCalcoloImpulsi = 0x0E,
		eCPUProgrammingCommand_setFattoreCalibrazioneMotore = 0x0F,
		eCPUProgrammingCommand_getPosizioneMacina = 0x10,
		eCPUProgrammingCommand_setMotoreMacina = 0x11,
		eCPUProgrammingCommand_testSelezione = 0x12,
		eCPUProgrammingCommand_getNomiLinguaCPU = 0x13,
		eCPUProgrammingCommand_disinstallazione = 0x14,
		eCPUProgrammingCommand_ricaricaFasciaOrariaFV = 0x15,
		eCPUProgrammingCommand_EVAresetPartial = 0x16,
		eCPUProgrammingCommand_getVoltAndTemp = 0x17,
		eCPUProgrammingCommand_getCPUOFFReportDetails = 0x18,
		//CPU EVENTS report details = 0x19 non implementato
		eCPUProgrammingCommand_getLastFluxInformation = 0x1A,
		eCPUProgrammingCommand_getStringVersionAndModel = 0x1B,
		eCPUProgrammingCommand_startModemTest = 0x1C,
		eCPUProgrammingCommand_EVAresetTotals = 0x1D,
		eCPUProgrammingCommand_getTimeNextLavaggioCappuccinatore = 0x1E,
		eCPUProgrammingCommand_startTestAssorbGruppo = 0x1F,
		eCPUProgrammingCommand_getStatusTestAssorbGruppo = 0x20,
		eCPUProgrammingCommand_startTestAssorbMotoriduttore = 0x21,
		eCPUProgrammingCommand_getStatusTestAssorbMotoriduttore = 0x22,
		eCPUProgrammingCommand_unknown = 0xff
    };

	enum eCPUProgrammingCommand_cleaningType
	{
		eCPUProgrammingCommand_cleaningType_invalid = 0x00,
		eCPUProgrammingCommand_cleaningType_mixer1 = 0x01,
		eCPUProgrammingCommand_cleaningType_mixer2 = 0x02,
		eCPUProgrammingCommand_cleaningType_mixer3 = 0x03,
		eCPUProgrammingCommand_cleaningType_mixer4 = 0x04,
		eCPUProgrammingCommand_cleaningType_milker = 0x05,
		eCPUProgrammingCommand_cleaningType_sanitario = 0x08,
		eCPUProgrammingCommand_cleaningType_rinsing = 0xa0
	};

	enum eCPUProgrammingCommand_decounter
	{
		eCPUProgrammingCommand_decounter_unknown = 0,
		eCPUProgrammingCommand_decounter_prodotto1 = 1,
		eCPUProgrammingCommand_decounter_prodotto2 = 2,
		eCPUProgrammingCommand_decounter_prodotto3 = 3,
		eCPUProgrammingCommand_decounter_prodotto4 = 4,
		eCPUProgrammingCommand_decounter_prodotto5 = 5,
		eCPUProgrammingCommand_decounter_prodotto6 = 6,
		eCPUProgrammingCommand_decounter_prodotto7 = 7,
		eCPUProgrammingCommand_decounter_prodotto8 = 8,
		eCPUProgrammingCommand_decounter_prodotto9 = 9,
		eCPUProgrammingCommand_decounter_prodotto10 = 10,
		eCPUProgrammingCommand_decounter_waterFilter = 11,
		eCPUProgrammingCommand_decounter_coffeeBrewer = 12,
		eCPUProgrammingCommand_decounter_coffeeGround = 13,
		eCPUProgrammingCommand_blocking_counter = 14,
		eCPUProgrammingCommand_decounter_error = 0xff
	};

	enum eCPUProgrammingCommand_statoGruppo
	{
		eCPUProgrammingCommand_statoGruppo_nonAttaccato = 0x00,
		eCPUProgrammingCommand_statoGruppo_attaccato = 0x01
	};

	enum eCPUProgrammingCommand_motor
	{
		eCPUProgrammingCommand_motor_unknown = 0,
		eCPUProgrammingCommand_motor_prod1 = 1,
		eCPUProgrammingCommand_motor_prod2 = 2,
		eCPUProgrammingCommand_motor_prod3 = 3,
		eCPUProgrammingCommand_motor_prod4 = 4,
		eCPUProgrammingCommand_motor_prod5 = 5,
		eCPUProgrammingCommand_motor_prod6 = 6,
		eCPUProgrammingCommand_motor_prod7 = 7,
		eCPUProgrammingCommand_motor_prod8 = 8,
		eCPUProgrammingCommand_motor_prod9 = 9,
		eCPUProgrammingCommand_motor_prod10 = 10,
		eCPUProgrammingCommand_motor_macina1 = 11,
		eCPUProgrammingCommand_motor_macina2 = 12
	};

	enum eCPUProgrammingCommand_testSelectionDevice
	{
		eCPUProgrammingCommand_testSelectionDevice_wholeSelection = 0,
		eCPUProgrammingCommand_testSelectionDevice_prod1 = 1,
		eCPUProgrammingCommand_testSelectionDevice_prod2 = 2,
		eCPUProgrammingCommand_testSelectionDevice_prod3 = 3,
		eCPUProgrammingCommand_testSelectionDevice_prod4 = 4,
		eCPUProgrammingCommand_testSelectionDevice_prod5 = 5,
		eCPUProgrammingCommand_testSelectionDevice_prod6 = 6,
		eCPUProgrammingCommand_testSelectionDevice_prod7 = 7,
		eCPUProgrammingCommand_testSelectionDevice_prod8 = 8,
		eCPUProgrammingCommand_testSelectionDevice_prod9 = 9,
		eCPUProgrammingCommand_testSelectionDevice_prod10 = 10,
		
		eCPUProgrammingCommand_testSelectionDevice_macina = 11,

		eCPUProgrammingCommand_testSelectionDevice_water1 = 21,
		eCPUProgrammingCommand_testSelectionDevice_water2 = 22,
		eCPUProgrammingCommand_testSelectionDevice_water3 = 23,
		eCPUProgrammingCommand_testSelectionDevice_water4 = 24,
		eCPUProgrammingCommand_testSelectionDevice_water5 = 25,
		eCPUProgrammingCommand_testSelectionDevice_water6 = 26,
		eCPUProgrammingCommand_testSelectionDevice_water7 = 27,
		eCPUProgrammingCommand_testSelectionDevice_water8 = 28,
		eCPUProgrammingCommand_testSelectionDevice_water9 = 29,
		eCPUProgrammingCommand_testSelectionDevice_water10 = 30,

		eCPUProgrammingCommand_testSelectionDevice_mixer1 = 31,
		eCPUProgrammingCommand_testSelectionDevice_mixer2 = 32,
		eCPUProgrammingCommand_testSelectionDevice_mixer3 = 33,
		eCPUProgrammingCommand_testSelectionDevice_mixer4 = 34,
		eCPUProgrammingCommand_testSelectionDevice_mixer5 = 35,
		eCPUProgrammingCommand_testSelectionDevice_mixer6 = 36,
		eCPUProgrammingCommand_testSelectionDevice_mixer7 = 37,
		eCPUProgrammingCommand_testSelectionDevice_mixer8 = 38,
		eCPUProgrammingCommand_testSelectionDevice_mixer9 = 39,
		eCPUProgrammingCommand_testSelectionDevice_mixer10 = 40,

		eCPUProgrammingCommand_testSelectionDevice_unknown = 0xff
	};

	enum eCPUProgrammingCommand_macinaMove
	{
		eCPUProgrammingCommand_macinaMove_stop = 0,
		eCPUProgrammingCommand_macinaMove_open = 1,
		eCPUProgrammingCommand_macinaMove_close = 2
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
		static const u8 BUFFER_SIZE_IN_U16 = 120;
		u16			utf16LCDString[BUFFER_SIZE_IN_U16];
		u8			importanceLevel;
	};

	struct sCPUStatus
	{
		static const u16 FLAG1_READY_TO_DELIVER_DATA_AUDIT	= 0x0001;
		static const u16 FLAG1_TELEMETRY_RUNNING			= 0x0002;
		static const u16 FLAG1_IS_MILKER_ALIVE				= 0x0004;
		static const u16 FLAG1_IS_FREEVEND					= 0x0008;
		static const u16 FLAG1_IS_TESTVEND					= 0x0010;
		
		static const u16 FLAG1_CUP_ABSENT					= 0x0100;
		static const u16 FLAG1_SHOW_DLG_STOP_SELEZIONE		= 0x0200;

		char							userCurrentCredit[16];
		char							curLangISO[4];
		eVMCState						VMCstate;
		u8								VMCerrorCode;
		u8								VMCerrorType;
		u8								bShowDialogStopSelezione;
		u16								flag1;
		eStatoPreparazioneBevanda		statoPreparazioneBevanda;
		sCPUSelAvailability				selAvailability;
		u16								beepSelezioneLenMSec;
		sCPULCDMessage					LCDMsg;
	};

	struct sCPUVMCDataFileTimeStamp
	{
                sCPUVMCDataFileTimeStamp()                                              { setInvalid(); }
				
        void	setInvalid()                                                            { memset(data, 0xFF, SIZE_OF_BUFFER); data[1] = 0xfe; data[3] = 0xfc; }
        bool    isInvalid()                                                              { return (data[0]==0xff && data[1]==0xfe &&  data[2]==0xff && data[3]==0xfc && data[4]==0xff && data[5]==0xff); }
        u8		readFromBuffer(const void *buffer)                                      { memcpy(data, buffer, SIZE_OF_BUFFER); return SIZE_OF_BUFFER; }
        u8		writeToBuffer(void *buffer) const                                       { memcpy(buffer, data, SIZE_OF_BUFFER); return SIZE_OF_BUFFER; }
        u8		readFromFile (FILE *f)                                                  { fread(data, SIZE_OF_BUFFER, 1, f); return SIZE_OF_BUFFER; }
        u8		writeToFile(FILE *f) const                                              { fwrite(data, SIZE_OF_BUFFER, 1, f); return SIZE_OF_BUFFER; }

        bool	isEqual(const sCPUVMCDataFileTimeStamp &b) const                        { return (memcmp(data, b.data, SIZE_OF_BUFFER) == 0); }
        u8		getLenInBytes() const                                                   { return SIZE_OF_BUFFER; }

        sCPUVMCDataFileTimeStamp&	operator= (const sCPUVMCDataFileTimeStamp& b)		{ memcpy(data, b.data, SIZE_OF_BUFFER); return *this; }
        bool	operator== (const sCPUVMCDataFileTimeStamp &b) const                    { return isEqual(b); }
        bool	operator!= (const sCPUVMCDataFileTimeStamp &b) const                    { return !isEqual(b); }

	private:
		static const u8 SIZE_OF_BUFFER = 6;
		u8	data[SIZE_OF_BUFFER];
	};


	struct sExtendedCPUInfo
	{
		u8				msgVersion;
		eCPUMachineType	machineType;
		u8				machineModel;
		u8				isInduzione;		//induzione o bollitore?
	};

	struct sCPUOffSingleEvent
	{
		u8 codice;
		u8 tipo;
		u8 ora;
		u8 minuto;
		u8 giorno;
		u8 mese;
		u8 anno;
		u8 stato;
	};

} // namespace cpubridge

#endif // _CPUBridgeEnumAndDefine_h_
