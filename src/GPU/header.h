#ifndef RHEA_HEADER_H
#define RHEA_HEADER_H


//Versione GPU (Fusioe Beta v2)
#define GPU_VERSION_MAJOR   2
#define GPU_VERSION_MINOR   0
#define GPU_VERSION_BUILD   2



#ifdef PLATFORM_UBUNTU_DESKTOP
    #define CPU_COMPORT  "/dev/ttyUSB0"
#else
    #define CPU_COMPORT  "/dev/ttymxc3"
#endif


//define per mostrare il puntatore del mouse
#undef DEBUG_SHOW_MOUSE


#ifdef _DEBUG
    #ifndef DEBUG_SHOW_MOUSE
        #define DEBUG_SHOW_MOUSE
    #endif
#endif


#define ScreenW 1024
#define ScreenH 600

#define labelStatus_Prog_W          800
#define labelStatus_MarginBottom    8   
#define labelStatusProg_H           48


#define TIMER_INTERVAL_MSEC         50




#include "../rheaCommonLib/rhea.h"
#include "Utils.h"


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




#endif 

