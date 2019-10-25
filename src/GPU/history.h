#ifndef _HISTORY_H_INCLUSO_
#define _HISTORY_H_INCLUSO_
#include <stdint.h>
#include <QDir>

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;



/*
    la struttura del file history da esportare è cosi' costruita:
    2 bytes		LEN	= lunghezza dell'header

                HEADER
    4 bytes		numero di selezioni
                FINE HEADER

                INIZIO STORICO
    8 bytes		(vedi sHistoryRecord)
    n bytes		nome del file
    ...
    ...
    8 bytes		(vedi sHistoryRecord)
    n bytes		nome del file
                FINE DELLO STORICO



    Tecnicamente, la struttura è costruita in modo da poter memorizzare un numero "infinito" di record.
    Per motivi di spazio su HD (non su questa macchina in particolare, ma sulle TT che montano il PIC24 per esempio), si è deciso
    di memorizzare sempre e comunque solo l'ultimo file che l'utente ha uppato.
    Quindi nel file, al max, ci sarà  un recordo per l'ultima CPU caricata, uno per la GPU, uno per il DA3 e via dicendo.
*/


enum eHistoryType
{
    HISTORY_TYPE_CPU =  0,
    HISTORY_TYPE_GPU =	1,
    HISTORY_TYPE_DA2 =  2,
    HISTORY_TYPE_DA3 =	3,
    HISTORY_TYPE_GUI =	4,
    HISTORY_TYPE_PCAP=	5,

    HISTORY_TYPE_UNK = 0xff
};


/*********************************************************************
 * History
 *
 */
class History
{
private:
    static const UINT8 MAX_SIZE_FILENAME = 121;
    struct sRecord
    {
        UINT8 type;
        UINT8 year;
        UINT8 month;
        UINT8 day;
        UINT8 hour;
        UINT8 minute;
        UINT8 second;
        char  filename[MAX_SIZE_FILENAME];
    };

    static const UINT8 N_RECORD_IN_RECORDLIST = 8;
    struct sTheFile
    {
        UINT32  nSelezioni;
        UINT8   nRecord;
        sRecord recordList[N_RECORD_IN_RECORDLIST];
    };

public:
    static void     incCounterSelezioni();
    static void     addEntry (eHistoryType what, const char *filename);
    static void     appendAllDataToQFile (QFile *qf);
    static void     DEBUG_toScreen();

private:
    static FILE*    openHistoryFile(const char *mode);
    static FILE*    openOrCreate();
    static void     readAllInMemory (sTheFile *out);
    static void     writeFromMemory (const sTheFile *in);
};


#endif
