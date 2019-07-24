#include "header.h"
#include "mainwindow.h"
#include "formboot.h"
#include "formdebug.h"
#include <QApplication>
#include <qdir.h>
#include <qfile.h>
#include "../rheaGUIBridge/GUIBridge.h"
#include "Utils.h"


MainWindow *myMainWindow = NULL;
FormDEBUG *_formDEBUG = NULL;



//****************************************************
long long timeNowStartedAt = 0;
long getTimeNowMSec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long milliseconds = ts.tv_sec*1000LL + ts.tv_nsec/1000000;


    //struct timeval te;
    //gettimeofday(&te, NULL);
    //long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

    if (timeNowStartedAt == 0)
    {
        timeNowStartedAt = milliseconds;
        return 0;
    }
    else
    {
//        if (milliseconds < timeNowStartedAt)timeNowStartedAt=milliseconds;
        return (long)(milliseconds - timeNowStartedAt);
    }
}



//****************************************************
bool bEnabledFormDEBUG = false;

void enableFormDEBUG()                { bEnabledFormDEBUG=true; }

FormDEBUG* getFormDEBUG()
{
    if (!bEnabledFormDEBUG)
        return NULL;

    if (NULL == myMainWindow)
        return NULL;
    if (NULL == _formDEBUG)
    {
        _formDEBUG = new FormDEBUG(myMainWindow);
        int w = (myMainWindow->width() * 3) / 4;
        int h = (myMainWindow->height() * 3) / 4;
        _formDEBUG->resize (w, h);
        _formDEBUG->setWindowFlags( Qt::WindowStaysOnTopHint );
        _formDEBUG->show ();
    }


    return _formDEBUG;
}



//****************************************************
void DEBUG_COMM_MSG (const unsigned char *buffer, int start, int lenInBytes, bool bIsGPUSending)
{
    FormDEBUG *f = getFormDEBUG();
    if (f)
        f->addBuffer( buffer, start, lenInBytes, bIsGPUSending);
}

//****************************************************
void DEBUG_rawBuffer (const unsigned char *buffer, int start, int lenInBytes)
{
    FormDEBUG *f = getFormDEBUG();
    if (f)
        f->addRawBuffer( buffer, start, lenInBytes);
}

//****************************************************
void DEBUG_MSG (const char* format, ...)
{
    FormDEBUG *f = getFormDEBUG();
    if (f)
    {
        char buffer[1024];
        va_list args;
        va_start (args, format);
        vsnprintf (buffer, sizeof(buffer), format, args);
        va_end (args);

        f->addString(buffer);
    }
}

//****************************************************
void DEBUG_MSG_REPLACE_SPACES (const QString &q)
{
    FormDEBUG *f = getFormDEBUG();
    if (f)
    {
        QString qs = q;
        qs.replace(' ', '*');
        f->addString(qs);
    }

}

//****************************************************
void DEBUG_MSG_REPLACE_SPACES (const char* format, ...)
{
    FormDEBUG *f = getFormDEBUG();
    if (f)
    {
        char buffer[1024];
        va_list args;
        va_start (args, format);
        vsnprintf (buffer, sizeof(buffer), format, args);
        va_end (args);

        int i=0;
        while (buffer[i] != 0x00)
        {
            if (buffer[i] == ' ')
                buffer[i]= '*';
            i++;
        }
        f->addString(buffer);
    }
}

//****************************************************
unsigned char _button_keyNum_tx=0;
unsigned char getButtonKeyNum ()    { return _button_keyNum_tx; }
void setButtonKeyNum (unsigned char i)
{
    if (_button_keyNum_tx != i)
    {
        DEBUG_MSG ("Button keynum=%d", (int)i);
        _button_keyNum_tx=i;
    }
}

//****************************************************
void hideMouse()
{
    #ifndef DEBUG_SHOW_MOUSE
        QApplication::setOverrideCursor(Qt::BlankCursor);
    #endif
}


/****************************************************
 * double updateCPUStats()
 *
 * ritorna la % di utilizzo attuale della CPU
 */
struct sCPUStats
{
    long double v[8];
    double loadavg;
    unsigned char i;
    unsigned long timerMsec;
};
sCPUStats cpuStats;

double updateCPUStats()
{
    if (cpuStats.i == 0)
        cpuStats.i = 1;
    else
        cpuStats.i = 0;

    unsigned char index = cpuStats.i * 4;

    FILE *f = fopen("/proc/stat","r");
    fscanf(f,"%*s %Lf %Lf %Lf %Lf",&cpuStats.v[index],&cpuStats.v[index+1],&cpuStats.v[index+2],&cpuStats.v[index+3]);
    fclose(f);

    if (cpuStats.i == 1)
    {
        cpuStats.timerMsec += TIMER_INTERVAL_MSEC;
        if (cpuStats.timerMsec >= 250)
        {
            cpuStats.timerMsec = 0;
            long double a3 = cpuStats.v[0] + cpuStats.v[1] + cpuStats.v[2];
            long double b3 = cpuStats.v[4] + cpuStats.v[5] + cpuStats.v[6];
            long double d = (b3 + cpuStats.v[7]) - (a3 + cpuStats.v[3]);
            if (d != 0)
            {
                cpuStats.loadavg = (double) ((b3 - a3) / d);
                if (cpuStats.loadavg <= 0)
                    cpuStats.loadavg = 0;
            }
        }
    }

    return cpuStats.loadavg;
}



//****************************************************
int main(int argc, char *argv[])
{
    rhea::init(NULL);

    //inizializza il webserver e recupera un handle sul quale è possibile restare in ascolto per vedere
    //quando il server invia delle richieste (tipo: WEBSOCKET_COMMAND_START_SELECTION)
    HThreadMsgR hQMessageFromWebserver;
    HThreadMsgW hQMessageToWebserver;
    rhea::HThread hServer;
    if (!guibridge::startServer(&hServer, &hQMessageFromWebserver, &hQMessageToWebserver))
    {
        return -1;
    }



    memset (&cpuStats,0, sizeof(cpuStats));
    QApplication app(argc, argv);
    hideMouse();

    utils::gatherFolderInfo (qApp->applicationDirPath());

    myMainWindow = new MainWindow (hQMessageFromWebserver, hQMessageToWebserver);
    myMainWindow->show();

    int ret = app.exec();

    rhea::deinit();
    return ret;
}
