#ifndef FORMDEBUG_H
#define FORMDEBUG_H
#include <QFrame>

namespace Ui {
class FormDEBUG;
}

class FormDEBUG : public QFrame
{
    Q_OBJECT

public:
    explicit FormDEBUG(QWidget *parent = 0);
    ~FormDEBUG();

    void        resizeEvent( QResizeEvent *e );
    void        addRawBuffer(const unsigned char *buffer, int offset, int nBytes);
    void        addBuffer (const unsigned char *buffer, int start, int lenInBytes, bool bIsGPUSending);
    void        addString (const char *message);
    void        addString (const QString &s);



private slots:
    void on_btnX_clicked();

    void on_btnHistory_clicked();

    void on_btnPause_clicked();

private:
    Ui::FormDEBUG   *ui;
    char            lastGPUCommand;
    unsigned char   lastCPUStatus;
    unsigned char   lastErrCode;
    unsigned char   lastErrType;
    unsigned char   lastStatoPreparazioneBevanda;
    unsigned char   lastSelection_CPU_current;
    bool            bForceStateRefresh, bShrinked;
    int             wWhenNotShrinked, hWhenNotShrinked;
    bool            bPaused;

private:
    void    addStatusMessage(const char *text);

    void    priv_handle_GPU_to_CPU_Msg(const unsigned char *buffer, int nBytes, char *s);
    void    priv_handle_CPU_to_GPU_Msg(const unsigned char *buffer, int nBytes, char *s);
    void    priv_addRawBuffer(const unsigned char *buffer, int offset, int nBytes, char *s);
    void    priv_appendHexToString (int h, char *out_s);
    void    priv_appendCharToString (char h, char *out_s);
    void    priv_listOfFontFamilies();

};

#endif // FORMDEBUG_H
