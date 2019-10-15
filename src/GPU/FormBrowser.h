#ifndef FORMBROWSER_H
#define FORMBROWSER_H

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
    explicit                FormBrowser(QWidget *parent, const cpubridge::sSubscriber &subscriber);
                            ~FormBrowser();

private slots:
    void                    timerInterrupt();

private:
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);

private:
    cpubridge::sSubscriber  subscriber;
    Ui::FormBrowser          *ui;
    bool                    isInterruptActive;
    QTimer                  *timer;

};

#endif // FORMBROWSER_H
