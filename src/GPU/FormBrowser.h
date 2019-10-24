#ifndef FORMBROWSER_H
#define FORMBROWSER_H
#include "header.h"
#include <QDialog>
#include "../CPUBridge/CPUBridge.h"

namespace Ui
{
    class FormBrowser;
}

/*******************************************************************
 * FormBrowser
 *
 */
class FormBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit                FormBrowser(QWidget *parent, sGlobal *glob);
                            ~FormBrowser();

    void                    showMe();
    int                     onTick();

private:
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);

private:
    sGlobal                 *glob;
    Ui::FormBrowser          *ui;
    int                     retCode;
};

#endif // FORMBROWSER_H
