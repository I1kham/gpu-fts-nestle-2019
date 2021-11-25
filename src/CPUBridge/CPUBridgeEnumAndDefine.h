#ifndef _CPUBridgeEnumAndDefine_h_
#define _CPUBridgeEnumAndDefine_h_
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/rheaBit.h"

#define		NUM_MAX_SELECTIONS					48
#define		VMCDATAFILE_BLOCK_SIZE_IN_BYTE		64
#define		VMCDATAFILE_TOTAL_FILE_SIZE_IN_BYTE	10048

//PIN "universale" code per il menu di programmazione
#define		CPU_BYPASS_MENU_PROG_PIN_CODE       842


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
#define		CPUBRIDGE_NOTIFY_POSIZIONE_MACINA				0x011D
#define		CPUBRIDGE_NOTIFY_MOTORE_MACINA					0x011E
#define		CPUBRIDGE_NOTIFY_TEST_SELECTION					0x011F
#define		CPUBRIDGE_NOTIFY_NOMI_LINGUE_CPU				0x0120
#define		CPUBRIDGE_NOTIFY_EVA_RESET_PARTIALDATA			0x0121
#define		CPUBRIDGE_NOTIFY_GET_VOLT_AND_TEMP				0x0122
#define		CPUBRIDGE_NOTIFY_GET_OFF_REPORT					0x0123
#define		CPUBRIDGE_NOTIFY_GET_LAST_FLUX_INFORMATION		0x0124
#define		CPUBRIDGE_NOTIFY_GET_CPU_STRING_MODEL_AND_VER	0x0125
#define		CPUBRIDGE_NOTIFY_CPU_START_MODEM_TEST			0x0126
#define		CPUBRIDGE_NOTIFY_CPU_EVA_RESET_TOTALS			0x0127
#define		CPUBRIDGE_NOTIFY_GET_TIME_LAVSAN_CAPPUCINATORE	0x0128
#define		CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_GRUPPO	0x0129
#define		CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_MOTORIDUTTORE		0x012A
#define		CPUBRIDGE_NOTIFY_GETSTATUS_TEST_ASSORBIMENTO_GRUPPO			0x012B
#define		CPUBRIDGE_NOTIFY_GETSTATUSTEST_ASSORBIMENTO_MOTORIDUTTORE	0x012C
#define		CPUBRIDGE_NOTIFY_MILKER_VER							0x012D
#define		CPUBRIDGE_NOTIFY_CUR_SEL_RUNNING					0x012E
#define		CPUBRIDGE_NOTIFY_ESAPI_MODULE_VER_AND_TYPE			0x012F
#define		CPUBRIDGE_NOTIFY_START_GRINDER_SPEED_TEST			0x0130
#define		CPUBRIDGE_NOTIFY_LAST_GRINDER_SPEED					0x0131
#define		CPUBRIDGE_NOTIFY_SELECTION_NAME_UTF16_LSB_MSB		0x0132
#define		CPUBRIDGE_NOTIFY_SINGLE_SEL_PRICE_STRING            0x0133
#define		CPUBRIDGE_NOTIFY_GET_PRICEHOLDING_PRICELIST			0x0134
#define		CPUBRIDGE_NOTIFY_GET_MILKER_TYPE					0x0135
#define		CPUBRIDGE_NOTIFY_GET_JUG_REPETITIONS				0x0136
#define		CPUBRIDGE_NOTIFY_GET_CUPSENSOR_LIVE_VALUE			0x0137
#define		CPUBRIDGE_NOTIFY_RUN_CAFFE_CORTESIA					0x0138
#define		CPUBRIDGE_NOTIFY_QUERY_ID101						0x0139
#define		CPUBRIDGE_NOTIFY_VALIDATE_QUICK_MENU_PINCODE		0x013A
#define		CPUBRIDGE_NOTIFY_IS_QUICK_MENU_PINCODE_SET			0x013B
#define		CPUBRIDGE_NOTIFY_CPU_BUZZER_STATUS					0x013C
#define		CPUBRIDGE_NOTIFY_CPU_STOP_JUG						0x013D
#define		CPUBRIDGE_NOTIFY_CPU_GET_JUG_CURRENT_REPETITION		0x013E
#define		CPUBRIDGE_NOTIFY_END_OF_GRINDER_CLEANING_PROC		0x013F
#define		CPUBRIDGE_NOTIFY_MSG_FROM_LANGUAGE_TABLE			0x0140
#define		CPUBRIDGE_NOTIFY_CPU_RESTART						0x013A
#define		CPUBRIDGE_NOTIFY_LOCK_STATUS						0x013B
#define		CPUBRIDGE_NOTIFY_SELECTION_ENABLE					0x013C
#define		CPUBRIDGE_NOTIFY_OVERWRITE_CPU_MESSAGE_ON_SCREEN	0x013D
#define		CPUBRIDGE_NOTIFY_SET_SELECTION_PARAMU16				0x013E
#define		CPUBRIDGE_NOTIFY_GET_SELECTION_PARAMU16				0x013F
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
#define		CPUBRIDGE_SUBSCRIBER_ASK_QUERY_CUR_SEL_RUNNING							0x0837
#define		CPUBRIDGE_SUBSCRIBER_ASK_START_GRINDER_SPEED_TEST						0x0838
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_GRINDER_SPEED							0x0839
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB			0x083A
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_SINGLE_SEL_PRICE							0x083B
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_GET_PRICEHOLDING_PRICELIST					0x083C
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_GET_MILKER_TYPE							0x083D
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_JUG_REPETITONS								0x083E
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_CUPSENSOR_LIVE_VALUE						0x083F
#define		CPUBRIDGE_SUBSCRIBER_ASK_RUN_CAFFE_CORTESIA								0x0840
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_ID_101								0x0841
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_VALIDATE_QUICK_MENU_PINCODE				0x0842
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_IS_QUICK_MENU_PINCODE_SET					0x0843
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION_AND_FORCE_JUG				0x0844
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_ACTIVATE_BUZZER							0x0845
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_BUZZER_STATUS								0x0846
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_JUG									0x0847
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_JUG_CURRENT_REPETITION						0x0848
#define		CPUBRIDGE_SUBSCRIBER_ASK_END_OF_GRINDER_CLEANING_PROC					0x0849
#define		CPUBRIDGE_SUBSCRIBER_ASK_MSG_FROM_LANGUAGE_TABLE						0x084A
#define		CPUBRIDGE_SUBSCRIBER_ASK_CPU_RESTART									0x084B
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_MACHINE_LOCK_STATUS						0x084C
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_MACHINE_LOCK_STATUS						0x084D
#define		CPUBRIDGE_SUBSCRIBER_ASK_SELECTION_ENABLE								0x084E
#define		CPUBRIDGE_SUBSCRIBER_ASK_OVERWRITE_CPU_MESSAGE_ON_SCREEN				0x084F
#define		CPUBRIDGE_SUBSCRIBER_ASK_SET_SELECTION_PARAMU16							0x0850
#define		CPUBRIDGE_SUBSCRIBER_ASK_GET_SELECTION_PARAMU16							0x0851

 /**********************************************************************
  * Nel messaggio "B", il sesto byte ([5]) funziona come una bitmask
  */
#define	CPU_MSG_B_BYTE6_FLAG_FORCE_JUG				0x01


namespace cpubridge
{
	enum class eCPUCommand : u8
	{
		checkStatus_B = 'B',
        checkStatus_B_Unicode = 'Z',
        initialParam_C = 'C',
		restart = 'U',
        readDataAudit = 'L',
		writeVMCDataFile = 'D',
        readVMCDataFile= 'E',
        //writeHexFile = 'H'
        //readHexFile= 'h',
        getVMCDataFileTimeStamp = 'T',
        programming = 'P',
		writePartialVMCDataFile = 'X',
		getExtendedConfigInfo = 'c',
		getMilkerVer = 'M',
        startSelWithPaymentAlreadyHandled_V = 'V',
		getNomeSelezioneCPU_d = 'd',
		requestPriceHoldingPriceList = 'H'
	};

	enum class eRunningSelStatus: u8
	{
		wait = 1,
		running = 2,
		finished_KO = 3,
		finished_OK = 4,
		runningCanUseStopBtn = 5
	};

	enum class eStatoPreparazioneBevanda: u8
	{
		unsupported = 0,
		doing_nothing = 0x01,
		wait = 0x02,
		running = 0x03
	};
	
	enum class eVMCState:u8
	{
		DISPONIBILE = 2,
		PREPARAZIONE_BEVANDA = 3,
		PROGRAMMAZIONE = 4,
		INITIAL_CHECK = 5,
		eERROR = 6,
		LAVAGGIO_MANUALE = 7,
		LAVAGGIO_AUTO = 8,
		RICARICA_ACQUA = 9,
		ATTESA_TEMPERATURA = 10,
		ATTESA_CARICA_MASTER = 11,
		INSTALLAZIONE = 12,
		DISINSTALLAZIONE = 13,
		FINE_INSTALLAZIONE = 14,
		FINE_DISINSTALLAZIONE = 15,
		ENERGY_SAVING = 16,
        TEST_DB = 17,
		DATA_AUDIT = 18,
        LAVAGGIO_SANITARIO = 20,
		TEST_ATTUATORE_SELEZIONE = 21,
		TEST_MODEM = 22,
		LAVAGGIO_MILKER_VENTURI = 23,
		LAVAGGIO_MILKER_INDUX = 24,
		DESCALING	= 26,

		COM_ERROR     = 101,
		REG_APERTURA_MACINA = 102,
		COMPATIBILITY_CHECK = 103,
		CPU_NOT_SUPPORTED = 104,
		DA3_SYNC = 105,
		GRINDER_SPEED_TEST = 106

	};

    enum class eReadDataFileStatus: u8
    {
        inProgress = 0,
        finishedOK = 1,
        finishedKO_cantStart_invalidState = 2,
        finishedKO_cpuDidNotAnswer = 3,
		finishedKO_unableToCreateFile = 4
    };

	enum class eWriteDataFileStatus: u8
	{
		inProgress = 0,
		finishedOK = 1,
		finishedKO_cantStart_invalidState = 2,
		finishedKO_cpuDidNotAnswer = 3,
		finishedKO_unableToCopyFile = 4,
		finishedKO_unableToOpenLocalFile = 5,
        finishedKO_cpuDidNotAnswer2 = 6,
	};

	enum class eWriteCPUFWFileStatus: u8
	{
		inProgress_erasingFlash = 0,
		inProgress,
		finishedOK,
		finishedKO_cantStart_invalidState,
		finishedKO_unableToCopyFile,
		finishedKO_unableToOpenLocalFile,
		finishedKO_k_notReceived,
		finishedKO_M_notReceived,
		finishedKO_h_notReceived,
		finishedKO_generalError
	};

	enum class eCPUMachineType: u8
	{
		instant = 0x00,
        espresso1 = 0x01,
        espresso2 = 0x02,
        unknown = 0xff
	};

	enum class ePaymentMode: u8
	{
		normal = 0,
		freevend = 1,
		testvend = 2,
		invalid = 0xff
	};

	enum class eGPUPaymentType: u8
	{
		unknown = 0,
		alipayChina = 1,
		invalid = 0xff
	};

    enum class eCPUProgrammingCommand: u8
    {
        enterProg = 0x01,
        cleaning = 0x02,
		querySanWashingStatus = 0x03,
		setDecounter = 0x04,
		//ELIMINATO resetCounter = 0x05,
		getAllDecounterValues = 0x06,
		getTime = 0x07,
		getDate = 0x08,
		setTime = 0x09,
		setDate = 0x0A,
		getStatoGruppo = 0x0B,
		attivazioneMotore = 0x0C,
		calcolaImpulsiMacina = 0x0D,
		getStatoCalcoloImpulsi = 0x0E,
		setFattoreCalibrazioneMotore = 0x0F,
		getPosizioneMacina = 0x10,
		setMotoreMacina = 0x11,
		testSelezione = 0x12,
		getNomiLinguaCPU = 0x13,
		disinstallazione = 0x14,
		ricaricaFasciaOrariaFV = 0x15,
		EVAresetPartial = 0x16,
		getVoltAndTemp = 0x17,
		getCPUOFFReportDetails = 0x18,
		//CPU EVENTS report details = 0x19 non implementato
		getLastFluxInformation = 0x1A,
		getStringVersionAndModel = 0x1B,
		startModemTest = 0x1C,
		EVAresetTotals = 0x1D,
		getTimeNextLavaggioCappuccinatore = 0x1E,
		startTestAssorbGruppo = 0x1F,
		getStatusTestAssorbGruppo = 0x20,
		startTestAssorbMotoriduttore = 0x21,
		getStatusTestAssorbMotoriduttore = 0x22,
		getLastGrinderSpeed = 0x23,
		getCupSensorLiveValue = 0x24,			//ritorna il valore attualmente letto da CPU
		caffeCortesia = 0x25,
		//get_ID101_104_106 = 0x26,				//non implementato al momento
		activate_cpu_buzzer = 0x27,
		get_cpu_buzzer_status = 0x28,
		get_jug_current_repetition = 0x29,
		stop_jug = 0x2A,
		notify_end_of_grinder_cleaning_proc = 0x2B,
		//
		//
		ask_msg_from_table_language = 0x31,
		//
		setSelectionParam			= 0x33,
		unknown = 0xff
    };

	enum class eCPUProg_cleaningType: u8
	{
		invalid = 0x00,
		mixer1 = 0x01,
		mixer2 = 0x02,
		mixer3 = 0x03,
		mixer4 = 0x04,
		milker = 0x05,			//questo dice alla CPU di far partire il lav san del milker considerando che il comando arrriva da dentro il menu prog
		sanitario = 0x08,
		milkerQuick = 0x20,		//come 0x05, ma considerando che il comando arrriva da fuori menu prog
		rinsing = 0xA0,
		descaling = 0xA1
	};

	enum class eCPUProg_decounter: u8
	{
		unknown = 0,
		prodotto1 = 1,
		prodotto2 = 2,
		prodotto3 = 3,
		prodotto4 = 4,
		prodotto5 = 5,
		prodotto6 = 6,
		prodotto7 = 7,
		prodotto8 = 8,
		prodotto9 = 9,
		prodotto10 = 10,
		waterFilter = 11,
		coffeeBrewer = 12,
		coffeeGround = 13,
		blocking_counter = 14,
		maintenance_counter = 15,
		error = 0xff
	};

	enum class eCPUProg_statoGruppo : u8
	{
		nonAttaccato = 0x00,
		attaccato = 0x01
	};

	enum class eCPUProg_motor: u8
	{
		unknown = 0,
		prod1 = 1,
		prod2 = 2,
		prod3 = 3,
		prod4 = 4,
		prod5 = 5,
		prod6 = 6,
		prod7 = 7,
		prod8 = 8,
		prod9 = 9,
		prod10 = 10,
		macina1 = 11,
		macina2 = 12
	};

	enum class eCPUProg_testSelectionDevice: u8
	{
		wholeSelection = 0,
		prod1 = 1,
		prod2 = 2,
		prod3 = 3,
		prod4 = 4,
		prod5 = 5,
		prod6 = 6,
		prod7 = 7,
		prod8 = 8,
		prod9 = 9,
		prod10 = 10,
		
		macina = 11,

		water1 = 21,
		water2 = 22,
		water3 = 23,
		water4 = 24,
		water5 = 25,
		water6 = 26,
		water7 = 27,
		water8 = 28,
		water9 = 29,
		water10 = 30,

		mixer1 = 31,
		mixer2 = 32,
		mixer3 = 33,
		mixer4 = 34,
		mixer5 = 35,
		mixer6 = 36,
		mixer7 = 37,
		mixer8 = 38,
		mixer9 = 39,
		mixer10 = 40,

		unknown = 0xff
	};

	enum class eCPUProg_macinaMove: u8
	{
		stop = 0,
		open = 1,
		close = 2
	};

    enum class eCPUGruppoCaffe : u8
	{
        Variflex = 'V',
        Micro = 'M',
        None = 'N'
	};

	enum class eCPUMilkerType: u8
	{
		none = 0,
		venturi = 1,
		indux = 2
	};

	enum class eLockStatus : u8
	{
		unlocked = 0,
		locked = 1,
		onlyAPI = 2,
	};

	enum class eSelectionParam : u8 
	{
		EVFreshMilk = 0x01,
		EVFreshMilkDelay_dsec = 0x02,
		EVAirFreshMilk = 0x03,
		EVAirFreshMilkDelay_dsec = 0x04
	};

	struct sSubscriber
	{
		HThreadMsgR	hFromMeToSubscriberR;
		HThreadMsgW	hFromMeToSubscriberW;
		HThreadMsgR	hFromSubscriberToMeR;
		HThreadMsgW	hFromSubscriberToMeW;
	};


	struct sCPUParamIniziali
	{
		char	CPU_version[16];
		u8		protocol_version;
		u16		pricesAsInAnswerToCommandC[48];	//nel 99% dei casi, questi sono i prezzi delle selezioni. Nel caso di PRICE-HOLDING, questi sono indici ai prezzi mantenuti dalla gettoniera
		u16		pricesAsInPriceHolding[120];	//Nel casi di price holding, qui ci sono i (max 100 ad partire da indice 1 fino a indice 100 compresi) prezzi contenuti nella gettoniera.
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
		static const u16 FLAG1_IS_RFID_DEBT					= 0x0020;
		static const u16 FLAG1_IS_RFID_JUG					= 0x0040;
		
		static const u16 FLAG1_CUP_DETECTED					= 0x0100;
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

		bool							isReadyToDeliverDataAduti() const				{ return ((flag1 & FLAG1_READY_TO_DELIVER_DATA_AUDIT) != 0); }
		bool							isTelemetryRunning() const						{ return ((flag1 & FLAG1_TELEMETRY_RUNNING) != 0); }
		bool							isMilkerAlive() const							{ return ((flag1 & FLAG1_IS_MILKER_ALIVE) != 0); }
		bool							isFreevend() const								{ return ((flag1 & FLAG1_IS_FREEVEND) != 0); }
		bool							isTestvend() const								{ return ((flag1 & FLAG1_IS_TESTVEND) != 0); }
		bool							isCupDetected() const							{ return ((flag1 & FLAG1_CUP_DETECTED) != 0); }
	};

	struct sCPUVMCDataFileTimeStamp
	{
                sCPUVMCDataFileTimeStamp()                                              { setInvalid(); }
				
        void	setInvalid()                                                            { memset(data, 0xFF, SIZE_OF_BUFFER); data[1] = 0xfe; data[3] = 0xfc; }
        bool    isInvalid()                                                             { return (data[0]==0xff && data[1]==0xfe &&  data[2]==0xff && data[3]==0xfc && data[4]==0xff && data[5]==0xff); }
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
		eCPUGruppoCaffe	tipoGruppoCaffe;
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
