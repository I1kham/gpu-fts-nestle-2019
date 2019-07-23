#include "lang.h"
#include "header.h"


#define LANG_ERR_FILE_NOT_FOUND			1
#define LANG_ERR_FSEEK_ERROR			2
#define LANG_ERR_WRONG_INITIAL_CHAR		3
#define LANG_ERR_STR_WITH_NO_TERINATION	4
#define LANG_ERR_GENERAL				5

#define LANG_SIZE_OF_A_MESSAGE_IN_BYTES	72
typedef struct sLangMsg
{
    QChar	msg[2 + (LANG_SIZE_OF_A_MESSAGE_IN_BYTES/2)];
} LangMsg;


extern QString Folder_languages;



void lang_setAndDisplayErrorCode (sLanguage *lang, QChar *out, unsigned char code, char tableCode, int msgNum)
{
    lang->errorCode = code;

    char s[32];
    sprintf (s,"Lang err (%d) %c %d", code, tableCode, msgNum);

    unsigned char i = 0;
    while (s[i] != 0x00)
    {
        out[i] = s[i];
        i++;
    }
    out[i] = 0;
}


void lang_init (sLanguage *lang)
{
    lang->iso[0] = 'X';
    lang->iso[1] = 'X';
    lang->iso[2] = 0x00;
    lang->errorCode = 0x00;
    lang->ff[0] = lang->ff[1] = NULL;
    lang->tableIDD[0] = lang->tableIDD[1] = 0xff;
}


/*****************************************************
 * 	mantengo aperti 2 puntatori  file.
 * 	f[0] punta sempre alla tabella A oppure B le quali contengono l grosso dei messaggi (utente e programmazione).
 *	f[1] invece lo apro e chiudo ala bisogna in quanto punta alle tabelle accessorie che sono utilizzate con una frequenza molto minore rispetto a A e B
 */
FILE* lang_open_table (sLanguage *lang, unsigned char tableID)
{
    unsigned char iTable = 0;
    if (tableID != 'A' && tableID != 'B')
        iTable = 1;

    if (lang->tableIDD[iTable] == tableID)
        return lang->ff[iTable];

    lang->tableIDD[iTable] = tableID;


    char filename[128];
    sprintf(filename,"%s/%s-MSG%c.lng", Folder_languages.toStdString().c_str(), lang->iso, tableID);

    if (lang->ff[iTable] != NULL)
        fclose (lang->ff[iTable]);

    DEBUG_MSG (filename);
    lang->ff[iTable] = fopen (filename, "rb");

    return lang->ff[iTable];
}


void lang_open (sLanguage *lang, const char *langISOCode)
{
    if (lang->ff[0] != NULL)
        fclose (lang->ff[0]);
    if (lang->ff[1] != NULL)
        fclose (lang->ff[1]);

    lang->iso[0] = langISOCode[0];
    lang->iso[1] = langISOCode[1];
    lang->errorCode = 0x00;

    lang->ff[0] = lang->ff[1] = NULL;
    lang->tableIDD[0] = lang->tableIDD[1] = 0xff;
}


const char*	lang_getCurLanguage (const sLanguage *lang)
{
    return lang->iso;
}

unsigned char	lang_getErrorCode (const sLanguage *lang)
{
    return lang->errorCode;
}

void lang_clearErrorCode (sLanguage *lang)
{
    lang->errorCode = 0;
}

/***********************************************************
 *	Cerca il messaggio i-esimo nel file di linguaggio della lingua attualmente in uso.
 *	Se non ci sono problemi,ritorna 1 e out->msg contiene il messaggio
 *
 *	In caso di problemi, ritorna 0 e lang->msg contiene un msg di errore
 *
 */
unsigned char lang_getMessage (sLanguage *lang, unsigned char tableID, int msgID, LangMsg *out)
{
    int t;

    FILE *f = lang_open_table (lang, tableID);
    if (NULL == f)
    {
        lang_setAndDisplayErrorCode (lang, out->msg, LANG_ERR_FILE_NOT_FOUND, tableID, msgID);
        return 0;
    }


    long offset = (long)LANG_SIZE_OF_A_MESSAGE_IN_BYTES * (long)(msgID-1);
    if (fseek(f, offset, SEEK_SET) < 0)
    {
        lang_setAndDisplayErrorCode (lang, out->msg, LANG_ERR_FSEEK_ERROR, tableID, msgID);
        return 0;
    }

    unsigned char s[LANG_SIZE_OF_A_MESSAGE_IN_BYTES];
    fread(s, 1, LANG_SIZE_OF_A_MESSAGE_IN_BYTES, f);

    int i;
    t=0;
    for (i=0; i<(LANG_SIZE_OF_A_MESSAGE_IN_BYTES/2); i++)
    {
        //b1 b2 b1 b2
        //00 6c FF 76 FF 6F FF 8C FF 9F FF 66
        ushort b1 = (ushort)s[t++];
        ushort b2 = (ushort)s[t++];

        b1 <<= 8;
        b1 |= b2;
        out->msg[i] = b1;
    }
    out->msg[i] = 0;

    //rtrim
    if (i > 0)
    {
        --i;
        while (out->msg[i] == ' ')
            out->msg[i--] = 0x00;
    }

    return 1;
}



/***********************************************************
 * msgIN_OUT contiene il messaggio esattamente come è stato inviato dalla CPU, quindi:
 *
 *		@XYYY§param1§param2§...§paramN
 *
 *	dove:
 *		X 	indica l'identifatore della tabella da usare (ogni linguaggio ha n tabella, una per gli usr-msg, una per i msg di off e via dicendo). L'identificatore in questione
            è un char, il che vuol dire che potrebbe essere una lettera maiuscola, minuscola oppure un numero (inteso come carattere compreso tra '0' e '9')
 *		YYY	indica il "numero del messaggio" da 0 a n. Il numero è espresso in ascii, quindi è una stringa di 3 caratteri ognugno dei quali è compreso tra '0' e '9'
 *
 *	opzionalmente possono esserci dei parametri. Un paramentro inizia con § e termina a fine stringa oppure quando incontra un altro §
 *	A sua volta, il parametro potrebbe essere nella forma @xyyy, ovvero è possibile che sia anche lui da tradurre
 *
 * Es:
 *	@a096					-> usa il messaggio 96 della tabella a
 *	@D2001					-> usa il messaggio 1 della tabella D
 *	@z033§pippo§pluto		-> usa il messaggio 33, tabella z con param1="pippo" e param2="pluto"
 *  @a246§100.00§@j123		-> usa il messaggio 246, tabella a, con param1="100.00" e param2="usa la tabella j, msg 123"
 *
 *
 *	ATTENZIONE: msgIN_OUT deve essere effettivamente una variabile. Una cosa del tipo lang_translate(&lang, "@a128§pippo") non va bene perchè il parametro msgIN_OUT deve essere un buffer
 *              manipolabile
 *
 *				[maxNumOfXCharInmsgIN_OUT] 	è il numero massimo di caratteri, escluso lo 0x00. Questo vuol dire che se maxNumOfXCharInmsgIN_OUT==12, allora posso inserire fino a
                                            12 caratteri più un tredicesimo per indicare lo 0x00
 */
void lang_translate (sLanguage *lang, QChar *msgIN_OUT, int maxNumOfXCharInmsgIN_OUT)
{
    int i,t;
    if (msgIN_OUT[0] != LANG_CHIOCCIOLA)
    {
        lang_setAndDisplayErrorCode (lang, msgIN_OUT, LANG_ERR_WRONG_INITIAL_CHAR, ' ', 0);
        return;
    }

    //mi accerto che ci sia uno 0x00 alla fine
    for (i=0; i<=MAXLEN_MSG_LCD; i++)
    {
        if (msgIN_OUT[i] == 0x00)
        {
            i = 0xff;
            break;
        }

        if (msgIN_OUT[i] <= 20)
            msgIN_OUT[i]=' ';
    }

    if (i != 0xff)
    {
        lang_setAndDisplayErrorCode (lang, msgIN_OUT, LANG_ERR_STR_WITH_NO_TERINATION, ' ', 0);
        return;
    }



    //id tabella
    unsigned char tableID = (unsigned char)msgIN_OUT[1].unicode();

    //id msg
    int msgID = (  ((unsigned char)msgIN_OUT[2].unicode() - '0') * 100)
                + (((unsigned char)msgIN_OUT[3].unicode() - '0') * 10)
                +  ((unsigned char)msgIN_OUT[4].unicode() - '0');


    DEBUG_MSG_REPLACE_SPACES(QString("from CPU:") +QString(msgIN_OUT,-1) +QString("!!!"));


    //eventuali parametri
    i = 5;
    unsigned char nParam = 0;
    QChar *params[8] = { 0,0,0,0,0,0,0,0 };
    if (msgIN_OUT[i] == LANG_PARAM_SEPARATOR)
    {
        ++i;
        while (msgIN_OUT[i] != 0x00)
        {
            params[nParam] = &msgIN_OUT[i];
            while (msgIN_OUT[i] != 0x00 && msgIN_OUT[i] != LANG_PARAM_SEPARATOR)
                i++;

            if (msgIN_OUT[i] == LANG_PARAM_SEPARATOR)
            {
                msgIN_OUT[i++] = 0x00;

                //caso speciale di 2 messaggi concatenati.
                //Se uno dei param termina con una sequenza del tipo @Annn, allora vuol dire che quello è l'inizio di un secondo
                //messaggio concatenato al primo

                //lunghezza del parametro
                int n = 0;
                while (params[nParam][n] != 0)
                    n++;

                if (n > 5)
                {
                    if (params[nParam][n - 5] == LANG_CHIOCCIOLA && (params[nParam][n - 4] == 'A' || params[nParam][n - 4] == 'B'))
                    {
                        if ((params[nParam][n - 3] >= '0' && params[nParam][n - 3] <= '9') &&
                            (params[nParam][n - 2] >= '0' && params[nParam][n - 2] <= '9') &&
                            (params[nParam][n - 1] >= '0' && params[nParam][n - 1] <= '9'))
                        {
                            msgIN_OUT[i-1] = LANG_PARAM_SEPARATOR;

                            QChar secondMsg[MAX_GPU_MESSAGE_LEN+2];
                            int t = n - 5;
                            int ct = 0;
                            while (params[nParam][t] != 0x00)
                                secondMsg[ct++] = params[nParam][t++];
                            secondMsg[ct] = 0;

                            params[nParam][n - 5] = 0x00;
                            lang_translate(lang, msgIN_OUT, maxNumOfXCharInmsgIN_OUT);
                            lang_translate(lang, secondMsg, MAX_GPU_MESSAGE_LEN);

                            t = 0;
                            while (msgIN_OUT[t] != 0x00)
                                t++;

                            //devo ignorare gli a capo sul 2ndo msg
                            ct = 0;
                            while (secondMsg[ct] != 0x00)
                            {
                                if (secondMsg[ct] == '\n')
                                    ct++;
                                else
                                    msgIN_OUT[t++] = secondMsg[ct++];
                            }
                            msgIN_OUT[t] = 0;
                            return;
                        }
                    }
                }
            }
            nParam++;
        }
    }



    /* 	Arrivato qui, ho il msgID, tableID e l'elenco degli eventuali parametri.
        C'è da formattare in maniera appropriata il messaggio e poi memcpiarlo in msgIN_OUT
    */
    LangMsg translated;
    if (!lang_getMessage(lang, tableID, msgID, &translated))
    {
        //c'è stato un problema. translated->msg contiene un msg di errore
        i=0;
        while (translated.msg[i] != 0x00)
        {
            msgIN_OUT[i] = translated.msg[i];
            i++;
        }
        msgIN_OUT[i] = 0;
        return;
    }

    DEBUG_MSG_REPLACE_SPACES(QString("from file:") + QString(translated.msg, -1) + QString("!!!"));


    QChar out[64];
    t = 0;
    i = 0;
    while (translated.msg[i] != 0x00)
    {
        QChar c = translated.msg[i++];

        //devo ignorare i \n
        if (c == '\n')
            c=' ';

        if (c == LANG_PARAM_SEPARATOR)
        {

            unsigned char paramNum = ((unsigned char)translated.msg[i++].unicode() -'0');
            paramNum--;
            if (paramNum < nParam)
            {
                unsigned char t2 = t;
                const QChar *p = params[paramNum];
                while (p[0] != 0x00)
                {
                    out[t++] = p[0];
                    p++;
                }

                if (out[t2] == LANG_CHIOCCIOLA)
                {
                    //il parametro è a sua volta nella forma @xYYY e quindi è da tradurre
                    tableID = (unsigned char)out[t2+1].unicode();
                    msgID = 	(  ((unsigned char)out[t2+2].unicode() - '0') * 100)
                                + (((unsigned char)out[t2+3].unicode() - '0') * 10)
                                +  ((unsigned char)out[t2+4].unicode() - '0');

                    LangMsg paramTranslated;
                    lang_getMessage (lang, tableID, msgID, &paramTranslated);

                    t = t2;
                    t2 = 0;
                    while (paramTranslated.msg[t2] != 0x00)
                        out[t++] = paramTranslated.msg[t2++];

                    //se un parametro ha un \n alla fine, lo rimuovo
                    if (out[t - 1] == '\n')
                    {
                        t--;
                        out[t] = 0;
                    }
                }
            }
        }
        else
            out[t++] = c;
    }
    out[t] = 0x00;

    DEBUG_MSG_REPLACE_SPACES(QString("out:[") +QString(t) +QString("] ") +QString(out,-1) +QString("!!!"));

    /*sprintf (sDebug, "xlen=%d t=%d, max=%d", XCHAR_len (translated.msg), t, maxNumOfXCharInmsgIN_OUT);
    GDEBUG_console_XYWH (0,90,320,15, sDebug);*/

    if (t > maxNumOfXCharInmsgIN_OUT)
        t = maxNumOfXCharInmsgIN_OUT;

    lang->errorCode = 0;
    memcpy (msgIN_OUT, out, t*sizeof(QChar));
    msgIN_OUT[t] = 0;
}
