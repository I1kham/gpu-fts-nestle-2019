#include "formdebug.h"
#include "ui_formdebug.h"
#include "header.h"
#include "history.h"
#include "mainwindow.h"

extern MainWindow *myMainWindow;

extern unsigned char VMCstate;

//*************************************************
FormDEBUG::FormDEBUG(QWidget *parentIN) :
    QFrame(parentIN),
    ui(new Ui::FormDEBUG)
{
    ui->setupUi(this);
    lastGPUCommand=(char)0x00;
    lastCPUStatus = 0xff;
    lastErrCode = 0xff;
    lastErrType = lastStatoPreparazioneBevanda = 0xff;
    lastSelection_CPU_current=0xff;
    bForceStateRefresh = false;

    ui->textBox1->setCenterOnScroll(true);
    ui->textBox1->ensureCursorVisible();
    bShrinked = false;
    bPaused = false;
}


//*************************************************
FormDEBUG::~FormDEBUG()
{
    delete ui;
}

//*************************************************
void FormDEBUG::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent(e);
    int w = this->width();
    int h = this->height();

    if (bShrinked)
    {
        ui->textBox1->hide();
        ui->textBox2->hide();
        ui->btnHistory->hide();
        ui->btnX->move(0,0);
        return;
    }

    ui->textBox1->show();
    ui->textBox2->show();


    const int btnX_w = 32;
    const int textBox2_h = 31;

    ui->btnX->move (w - btnX_w -3, 0);
    ui->btnX->resize (btnX_w, textBox2_h);

    ui->btnHistory->move (ui->btnX->geometry().left() - btnX_w -3, 0);
    ui->btnHistory->resize (btnX_w, textBox2_h);
    ui->btnHistory->show();

    ui->textBox2->resize(ui->btnHistory->geometry().left() - 3, textBox2_h);

    int y = ui->textBox1->geometry().top();
    ui->textBox1->resize(w, h - y);


}

void FormDEBUG::on_btnX_clicked()
{
    if (!bShrinked)
    {
        bShrinked=true;
        wWhenNotShrinked = this->width();
        hWhenNotShrinked = this->height();
        this->resize(50,50);
    }
    else
    {
        bShrinked=false;
        this->resize(wWhenNotShrinked, hWhenNotShrinked);
    }

}



void FormDEBUG::addStatusMessage(const char *text)
{
    if (text == NULL)
        return;
    if (text[0] == 0x00)
        return;

    QString s = ui->textBox2->toPlainText();
    if (s.length() > 1000)
    {
        s = s.mid (500);
        ui->textBox2->moveCursor (QTextCursor::Start);
        ui->textBox2->clear();
        ui->textBox2->insertPlainText (s);
        ui->textBox2->moveCursor (QTextCursor::End);
    }

    ui->textBox2->moveCursor (QTextCursor::End);
    ui->textBox2->insertPlainText (text);
    ui->textBox2->insertPlainText ("   ");
    ui->textBox2->moveCursor (QTextCursor::End);
}


void FormDEBUG::addString(const char *text)
{
    if (text == NULL)
        return;
    if (text[0] == 0x00)
        return;
    addString (QString(text));
}

void FormDEBUG::addString (const QString &text)
{
    if (bPaused)
        return;

    QString s = ui->textBox1->toPlainText();
    if (s.length() > 12000)
    {
        s = s.mid (6000);
        ui->textBox1->moveCursor (QTextCursor::Start);
        ui->textBox1->clear();
        ui->textBox1->insertPlainText (s);
        ui->textBox1->moveCursor (QTextCursor::End);
    }

    long timeNowMSec = getTimeNowMSec();
    long sec  = timeNowMSec / 1000;
    long msec = timeNowMSec - sec*1000;
    char tt[16];
    sprintf (tt, "%05ld.%03ld ", sec, msec);

    ui->textBox1->moveCursor (QTextCursor::End);
    ui->textBox1->insertPlainText (tt);
    ui->textBox1->insertPlainText (text);
    ui->textBox1->insertPlainText ("\r\n");
    ui->textBox1->moveCursor (QTextCursor::End);


    myMainWindow->myFormProg.addDebugString(text);
}


/* specifica per i messaggi inviati tramite seriale */
void FormDEBUG::addBuffer (const unsigned char *buffer, int offset, int nBytes, bool bIsGPUSending)
{
    buffer += offset;
    char s[1024];
    s[0]=0x00;

    if (bIsGPUSending)
        priv_handle_GPU_to_CPU_Msg (buffer, nBytes, s);
    else
        priv_handle_CPU_to_GPU_Msg (buffer, nBytes, s);

    addString(s);
}

void FormDEBUG::priv_appendHexToString (int h, char *out_s)
{
    char hex[16];
    sprintf(hex, "%02X  ", h);
    strcat (out_s, hex);
}

void FormDEBUG::priv_appendCharToString (char h, char *out_s)
{
    char cc[2];
    cc[0] = h;
    cc[1] = 0;
    strcat (out_s, cc);
}

void FormDEBUG::addRawBuffer(const unsigned char *buffer, int offset, int nBytes)
{
    char s[1024];
    s[0]=0x00;
    priv_addRawBuffer (buffer, offset, nBytes, s);
    addString(s);
}

void FormDEBUG::priv_addRawBuffer(const unsigned char *buffer, int offset, int nBytes, char *s)
{
    char hex[16];
    for (int i=0; i < nBytes; i++)
    {
        unsigned char b = buffer[offset+i];
        sprintf(hex, "%02X  ", b);
        strcat (s, hex);
    }
}

void FormDEBUG::priv_handle_GPU_to_CPU_Msg(const unsigned char *buffer, int nBytes, char *s)
{
    lastGPUCommand = (char)buffer[1];
    sprintf (s,"GPU: %c %02X     ", lastGPUCommand, buffer[2]);
    switch (lastGPUCommand)
    {
        case CommandCPUInitialParam_C:
            strcat (s, " initial param A");
            break;

        case CommandCPUCheckStatus:   //CommandCPUCheckStatus
        case CommandCPUCheckStatus_Unicode:   //CommandCPUCheckStatus
            {
                s[0] = 0;
                int tastoPremuto = (int)buffer[3];
                if (tastoPremuto != 0)
                    sprintf (s,"GPU: # selezione %d", tastoPremuto);
            }
            break;


        case 'S':   // VMCcom.CommandCPUStartSelection
            {
                unsigned char numAcc = buffer[9];
                priv_addRawBuffer(buffer, 3, 6, s);
                strcat (s, "num_acc:");
                priv_appendHexToString (buffer[9], s);
                strcat (s, "\r\n");


                int cur_offset = 10;
                nBytes -= 10;
                while (numAcc > 0)
                {

                    strcat (s,"     ");
                    priv_addRawBuffer(buffer, cur_offset, 6,s);
                    strcat (s, "\r\n");
                    cur_offset += 6;
                    nBytes-=6;
                    numAcc--;
                }
                if (nBytes>0)
                    priv_addRawBuffer(buffer, cur_offset, nBytes, s);
            }
            break;

        case 'G':   // VMCcom.CommandCPUGett comandi per gettoniera e data audit
        default:
            priv_addRawBuffer(buffer, 3, nBytes - 3, s);
            break;
    }
}


//*****************************************************************************
void FormDEBUG::priv_handle_CPU_to_GPU_Msg(const unsigned char *buffer, int nBytes, char *s)
{
    switch ((char)buffer[1])
    {
        case CommandCPUCheckStatus:
        case CommandCPUCheckStatus_Unicode:
            {
                unsigned char cpuStatus = buffer[3];
                unsigned char codiceErrore = buffer[4];
                unsigned char tipoErrore = buffer[5];
                unsigned char selection_CPU_current = buffer[10];
                unsigned char statoPreparazioneBevanda = ((buffer[9] & 0x60) >> 5);
                if (bForceStateRefresh || VMCstate != cpuStatus || cpuStatus != lastCPUStatus || codiceErrore != lastErrCode ||
                    tipoErrore != lastErrType || lastSelection_CPU_current!=selection_CPU_current || statoPreparazioneBevanda!=lastStatoPreparazioneBevanda)
                {
                    bForceStateRefresh = false;
                    lastCPUStatus = cpuStatus;
                    lastErrCode = codiceErrore;
                    lastErrType = tipoErrore;
                    lastSelection_CPU_current = selection_CPU_current;
                    lastStatoPreparazioneBevanda = statoPreparazioneBevanda;

                    s[0] = 0;
                    switch (cpuStatus)
                    {
                        case VMCSTATE_COM_ERROR:            strcat (s,"COM_ERR"); break;
                        case VMCSTATE_DISPONIBILE:          strcat (s, "DISP"); break;
                        case VMCSTATE_PREPARAZIONE_BEVANDA: strcat (s, "PREP_BEVANDA"); break;
                        case VMCSTATE_PROGRAMMAZIONE:       strcat (s, "PROG"); break;
                        case VMCSTATE_INITIAL_CHECK:        strcat (s, "INIT_CHECK"); break;
                        case VMCSTATE_ERROR:                strcat (s, "ERR"); break;
                        case VMCSTATE_LAVAGGIO_MANUALE:     strcat (s, "LAVAG_MAN"); break;
                        case VMCSTATE_LAVAGGIO_AUTO:        strcat (s, "LAVAG_AUTO"); break;
                        case VMCSTATE_RICARICA_ACQUA:       strcat (s, "RICARICA_H2O"); break;
                        case VMCSTATE_ATTESA_TEMPERATURA:   strcat (s, "ATTESA_TEMP"); break;

                        default:
                            priv_appendHexToString (buffer[2], s);
                            break;
                    }

                    this->priv_appendHexToString(lastSelection_CPU_current, s);
                    this->priv_appendHexToString(lastErrCode, s);
                    this->priv_appendHexToString(lastErrType, s);
                    this->priv_appendHexToString(lastStatoPreparazioneBevanda, s);


                    addString (s);
                    addStatusMessage(s);
                    s[0] = 0;
                }
            }
            break;

        case CommandCPUInitialParam_C:
            sprintf (s, "CPU: %c      ", buffer[1]);
            priv_appendCharToString (buffer[2], s); strcat(s," ");    //yes no wait
            priv_addRawBuffer(buffer, 3, nBytes - 3, s);
            break;

        default:
            sprintf (s, "CPU: %c      ", buffer[1]);
            priv_addRawBuffer(buffer, 2, nBytes - 2, s);
            break;
    }
}


//************************************************************************************
void FormDEBUG::on_btnHistory_clicked()
{
    priv_listOfFontFamilies();
    //History::DEBUG_toScreen();
}


//************************************************************************************
void FormDEBUG::priv_listOfFontFamilies()
{
    this->addString("========== FONT FAMILY LIST BEGIN ==============");
    QFontDatabase database;

    {
        this->addString("   == Japanese ==");
        const QStringList fontFamilies = database.families(QFontDatabase::Japanese);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }

    {
        this->addString("   == Hebrew ==");
        const QStringList fontFamilies = database.families(QFontDatabase::Hebrew);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }

    {
        this->addString("   == SimplifiedChinese ==");
        const QStringList fontFamilies = database.families(QFontDatabase::SimplifiedChinese);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }

    {
        this->addString("   == TraditionalChinese ==");
        const QStringList fontFamilies = database.families(QFontDatabase::TraditionalChinese);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }

    {
        this->addString("   == Cyrillic ==");
        const QStringList fontFamilies = database.families(QFontDatabase::Cyrillic);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }

    {
        this->addString("   == Any ==");
        const QStringList fontFamilies = database.families(QFontDatabase::Any);
        for (int i=0; i<fontFamilies.count(); i++)
            this->addString("     " +fontFamilies.at(i));
        this->addString(" ");
    }


    this->addString("========== FONT FAMILY LIST END ==============");
}

//************************************************************************************
void FormDEBUG::on_btnPause_clicked()
{
    if (bPaused)
    {
        bPaused=false;
        addString ("************* RESUME *****************");
    }
    else
    {
        addString ("************* PAUSE *****************");
        bPaused=true;
    }
}
