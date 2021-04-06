#include "history.h"
#include "header.h"
#include "../rheaCommonLib/rhea.h"


//******************************************************************************
FILE* History::openHistoryFile(const char *mode)
{
    QString history_folder = "~/.my-app";
    QString history_filename = "history.dat";

    QDir destDir(history_folder);
    if (!destDir.exists())
        destDir.mkpath(".");

    QString fullPathAndName = history_folder;
    fullPathAndName +="/";
    fullPathAndName += history_filename;

    const char *fname = fullPathAndName.toStdString().c_str();
    return fopen(fname, mode);
}


/******************************************************************************
 * Apre in r+
 * Se il file non esiste, lo crea
 */
FILE* History::openOrCreate()
{
    FILE *f = History::openHistoryFile("r+");
    if (NULL == f)
    {
        sTheFile fileInMem;
        memset(&fileInMem, 0, sizeof(sTheFile));
        for (UINT8 i=0; i<N_RECORD_IN_RECORDLIST; i++)
            fileInMem.recordList[i].type = (UINT8)HISTORY_TYPE_UNK;

        writeFromMemory (&fileInMem);

        f = History::openHistoryFile("r+");
    }

    return f;
}

//******************************************************************************
void History::readAllInMemory (sTheFile *out)
{
    memset (out, 0, sizeof(sTheFile));

    FILE *f = openOrCreate();

    UINT16 headerLen;
    fread (&headerLen, 2, 1, f);
    fread (&out->nSelezioni, 4, 1, f);


    fseek (f, 2 + headerLen, SEEK_SET);
    while (!feof(f) && out->nRecord < N_RECORD_IN_RECORDLIST)
    {
        sRecord *rec = &(out->recordList[out->nRecord++]);

        fread (rec, sizeof(sRecord), 1, f);

        if (rec->filename[0] == 0x00 || rec->type > HISTORY_TYPE_PCAP)
            out->nRecord--;
    }

    rhea::fs::fileClose(f);
}

//******************************************************************************
void History::writeFromMemory (const sTheFile *in)
{
    FILE *f = openHistoryFile("w");
    if (NULL == f)
    {
        DBGBREAK;
        return;
    }

    UINT16 headerLen = 4;
    fwrite (&headerLen, 2, 1, f);
    fwrite (&in->nSelezioni, 4, 1, f);

    for (UINT8 i=0; i<in->nRecord; i++)
    {
        if (in->recordList[i].type != HISTORY_TYPE_UNK && in->recordList[i].filename[0] != 0x00)
            fwrite (&in->recordList[i], sizeof(sRecord), 1, f);
    }
    fflush(f);
    rhea::fs::fileClose(f);
}

//******************************************************************************
void History::incCounterSelezioni()
{
    FILE *f = openOrCreate();
    if (NULL == f)
    {
        DBGBREAK;
        return;
    }

    UINT32 nSelezioni;
    fseek (f, 2, SEEK_SET);
    fread (&nSelezioni, 4, 1, f);
    nSelezioni++;

    fseek (f, 2, SEEK_SET);
    fwrite (&nSelezioni, 4, 1, f);

    rhea::fs::fileClose(f);

    //utils::DEBUG_MSG("History::incCountSel => now=%d", nSelezioni);
}

//******************************************************************************
void History::DEBUG_toScreen()
{
    /*sTheFile fileInMem;
    readAllInMemory (&fileInMem);

    utils::DEBUG_MSG("History:");
    utils::DEBUG_MSG("  nSel: %d", fileInMem.nSelezioni);
    for (UINT8 i=0; i<fileInMem.nRecord; i++)
    {

        utils::DEBUG_MSG(" type:%d, data:%d-%d-%d %d:%d:%d, file:%s",
                        fileInMem.recordList[i].type,
                        fileInMem.recordList[i].year,
                          fileInMem.recordList[i].month,
                          fileInMem.recordList[i].day,
                          fileInMem.recordList[i].hour,
                          fileInMem.recordList[i].minute,
                          fileInMem.recordList[i].second,
                          fileInMem.recordList[i].filename
                  );
    }
*/
}

//******************************************************************************
void History::addEntry (eHistoryType what, const char *filename)
{
    //recupera data e ora di sistema
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    //legge l'attuale history
    sTheFile fileInMem;
    readAllInMemory (&fileInMem);

    //il record lo devo aggiungere solo se non esiste giÃ .
    //Se esiste giÃ , sovrascrivo quello esistente
    sRecord *rec = NULL;
    for (UINT8 i=0; i<fileInMem.nRecord; i++)
    {
        if (fileInMem.recordList[i].type == what)
        {
            rec = &fileInMem.recordList[i];
            break;
        }
    }

    if (NULL == rec)
        rec = &fileInMem.recordList[fileInMem.nRecord++];

    //aggiorno il record con data/ora e nome del file caricato
    memset (rec, 0, sizeof(sRecord));
    rec->type = what;
    rec->year = (UINT8)(tm.tm_year + 1900 - 2000);
    rec->month = tm.tm_mon + 1;
    rec->day = tm.tm_mday;
    rec->hour = tm.tm_hour;
    rec->minute = tm.tm_min;
    rec->second = tm.tm_sec;

    UINT8 filenameLen = (UINT8)strlen(filename);
    if (filenameLen >= MAX_SIZE_FILENAME)
        filenameLen=MAX_SIZE_FILENAME-1;
    memcpy (rec->filename, filename, filenameLen);
    rec->filename[filenameLen] = 0;

    //salvo
    writeFromMemory(&fileInMem);
}


void addToBuffer8_littleEndian (QByteArray &buffer, UINT16 &n, UINT8 b)  { buffer[n++] = b; }
void addToBuffer16_littleEndian (QByteArray &buffer, UINT16 &n, UINT16 b)
{
    buffer[n++] = (UINT8)(b & 0x00FF);
    buffer[n++] = (UINT8)((b & 0xFF00) >> 8);
}
void addToBuffer32_littleEndian (QByteArray &buffer, UINT16 &n, UINT32 b)
{
    buffer[n++] = (UINT8) (b & 0x000000FF);
    buffer[n++] = (UINT8)((b & 0x0000FF00) >> 8);
    buffer[n++] = (UINT8)((b & 0x00FF0000) >> 16);
    buffer[n++] = (UINT8)((b & 0xFF000000) >> 24);
}

//******************************************************************************
void History::appendAllDataToQFile (QFile *qf)
{
    //legge l'attuale history
    sTheFile fileInMem;
    readAllInMemory (&fileInMem);

    QByteArray buffer;
    buffer.resize(1024);

    UINT16 n = 0;
    addToBuffer16_littleEndian (buffer, n, 4);   //headerLen
    addToBuffer32_littleEndian (buffer, n, fileInMem.nSelezioni);

    for (UINT8 i=0; i<fileInMem.nRecord; i++)
    {
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].type);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].year);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].month);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].day);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].hour);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].minute);
        addToBuffer8_littleEndian (buffer, n, fileInMem.recordList[i].second);

        UINT8 len = (UINT8)strlen(fileInMem.recordList[i].filename);
        addToBuffer8_littleEndian (buffer, n, len);

        for (UINT8 t=0; t<len;t++)
            buffer[n++] = fileInMem.recordList[i].filename[t];
    }

    qf->write (buffer, n);
}
