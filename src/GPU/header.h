#ifndef RHEA_HEADER_H
#define RHEA_HEADER_H


//Versione GPU
#define GPU_VERSION_MAJOR   2
#define GPU_VERSION_MINOR   0
#define GPU_VERSION_BUILD   0



//define per mostrare il puntatore del mouse
#define DEBUG_SHOW_MOUSE

//se definita, mostra una label con la % di utilizzo della CPU
#undef DEBUG_MONITOR_CPU_USAGE

/*  define per visualizzare la finestra di debug che mostra la comunicazione tra CPU e GPU
    In generale, premendo 10 volte il btn grafico "info" la finestra viene visualizzata.
    Se si vuole però vedere la finestra sin da subito, è necessario abilitare questa define*/
#undef SHOW_DEBUG_WINDOW_WITH_COM_MESSAGES_AT_STARTUP

//la cpu deve passare da DISPONIBILE a "BEVANDA IN PREPARAZIONE" entro il tempo definito qui sotto
#define TIMEOUT_SELEZIONE_1_MSEC    12000

//una volta che la CPU è entrata in "PREPARAZIONE", deve tornare disponibile entro il tempo definito qui sotto
#define TIMEOUT_SELEZIONE_2_MSEC    120000




#define ScreenW 1024
#define ScreenH 600

#define labelStatus_Prog_W          800
#define labelStatus_MarginBottom    8   
#define labelStatusProg_H           48


#define TIMER_INTERVAL_MSEC         50

#define BTN_RESET_GROUND_COUNTER_TEXT   "RESET"
#define BTN_STOP_CURRENT_SELECTION_TEXT "STOP"
#define BTN_LAVAGGIO_MILKER_TEXT        "CLEAN\nMILKER"



#ifdef PLATFORM_UBUNTU_DESKTOP
    #define serialCPU_NAME  "/dev/ttyUSB0"
#else
    #define serialCPU_NAME  "/dev/ttymxc3"  
#endif



long            getTimeNowMSec();
void            enableFormDEBUG();
void            DEBUG_COMM_MSG (const unsigned char *buffer, int start, int lenInBytes, bool bIsGPUSending);
void            DEBUG_MSG (const char* format, ...);
void            DEBUG_MSG_REPLACE_SPACES (const char* format, ...);
void            DEBUG_rawBuffer (const unsigned char *buffer, int start, int lenInBytes);
void            hideMouse();
bool            isUsbPresent ();
double          updateCPUStats();


/*  questo rappresenta il "bottone" premuto dall'utente. Viene inviato alla CPU periodicamente
 *  Deriva dal fatto che inizialmente la macchina aveva una tastiera fisica con 16 bottoni. Il touch in sostanza
 *  simula la stessa cosa
 */
unsigned char   getButtonKeyNum ();
void            setButtonKeyNum (unsigned char i);




#define FormStatus_NORMAL               0
#define FormStatus_SELECTION_RUNNING    4
#define FormStatus_BOOT                 98
#define FormStatus_PROG                 99


#define ConfigFileSize 10048 
#define ConfigFile_blockDim 64
#define ConfigFileOperation_status_idle  0
#define ConfigFileOperation_status_Write_inProgress  1
#define ConfigFileOperation_status_Write_endOK  8
#define ConfigFileOperation_status_Write_endKO  9
#define ConfigFileOperation_status_Read_inProgress  11
#define ConfigFileOperation_status_Read_endOK  18
#define ConfigFileOperation_status_Read_endKO  19
#define ConfigFileOperation_status_CPU_inProgress  21
#define ConfigFileOperation_status_CPU_endOK  28
#define ConfigFileOperation_status_CPU_endKO  29
#define ConfigFileOperation_status_GUI_inProgress  31
#define ConfigFileOperation_status_GUI_endOK  38
#define ConfigFileOperation_status_GUI_endKO  39
#define ConfigFileOperation_status_Audit_inProgress  41
#define ConfigFileOperation_status_Audit_endOK  48
#define ConfigFileOperation_status_Audit_endKO  49

#define AUDIT_MAX_FILESIZE      (1024*64)
#define AUDIT_BLOCK_DIM         64


#define VMCSTATE_DISPONIBILE                    2
#define VMCSTATE_PREPARAZIONE_BEVANDA           3
#define VMCSTATE_PROGRAMMAZIONE                 4
#define VMCSTATE_INITIAL_CHECK                  5
#define VMCSTATE_ERROR                          6
#define VMCSTATE_LAVAGGIO_MANUALE               7
#define VMCSTATE_LAVAGGIO_AUTO                  8
#define VMCSTATE_RICARICA_ACQUA                 9
#define VMCSTATE_ATTESA_TEMPERATURA             10
#define VMCSTATE_SPENTA                         16
#define VMCSTATE_COM_ERROR                      101
#define VMCSTATE_SHOW_DIALOG_RESET_GRND_COUNTER 200
#define VMCSTATE_SHOW_DIALOG_LAVAGGIO_MILKER    201


#define charHeaderPacket                '#'
#define CommandCPUCheckStatus           'B'
#define CommandCPUCheckStatus_Unicode	'Z' 
#define CommandCPUInitialParam_C        'C'
#define CommandCPURestart               'U'
#define CommandCPUWriteConfigFile       'D'
#define CommandCPUReadConfigFile        'E'
#define CommandCPUWriteHexFile          'H'
#define CommandCPUReadHexFile           'h'
#define CommandCPUReadAudit             'L'


#define ComStatus_Idle                  0
#define ComStatus_Tx1                   1
#define ComStatus_Tx2                   2
#define ComStatus_Rx                    3
#define ComStatus_HandleReply           4
#define ComStatus_ReplyOK               5
#define ComStatus_Error                 101
#define ComStatus_Error_WaitRestart     102
#define ComStatus_Disabled              110



#define Com_MAX_TIMEOUT_CHAR_MSEC       1000
#define Com_MAX_TOT_TIMEOUT_COMMAND     20



#define ComCommandRequest_CheckStatus_req       0
#define ComCommandRequest_RestartCPU_req        10
#define ComCommandRequest_InitialParam_req      30
#define ComCommandRequest_WriteConfigFile_req	40
#define ComCommandRequest_ReadConfigFile_req	50
#define ComCommandRequest_ReadAudit_req         60



#define MaxLenBufferComCPU_Rx	240	
#define MaxLenBufferComCPU_Tx	400	
#define MAXLEN_MSG_LCD            32



#define NUM_MAX_SELECTIONS 48










#endif 

