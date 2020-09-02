#include "main.h"

#define GPIO_LED            29
#define LED_ON              digitalWrite (GPIO_LED, 1)
#define LED_OFF             digitalWrite (GPIO_LED, 0)

#define USB_MOUNT_POINT     "/media/pi/usbpendrive"
#define USB_FOLDER          "rhea/rasPI"
#define BIN_FOLDER          "/home/pi/rhea/gpu-fts-nestle-2019/bin"
#define WWW_FOLDER          "/var/www/html/rhea"

/*****************************************************
 * Se trova un file con estensione .rheaRaspi nella cartella USB_FOLDER, lo copia in BIN_FOLDER
 * e lo scompatta sovrascrivendo gli eventuali file già esistenti
 * Ritorna true in quel caso, false altrimenti
 */
bool checkFWUpdate (const u8 *usb_path)
{
    OSFileFind ff;
    if (!rhea::fs::findFirst (&ff, usb_path, (const u8*)"*.rheaRaspi"))
        return false;

    do
    {
        if (!rhea::fs::findIsDirectory(ff))
        {
            const u8 *fname = rhea::fs::findGetFileName(ff);
            if (fname[0] != '.')
            {
                //ho trovato un file xxxx.rheaRaspi
                //devo copiarlo in locale
                u8 s1[512];
                u8 s2[512];

                sprintf_s ((char*)s1, sizeof(s1), "%s/%s", usb_path, fname);
                sprintf_s ((char*)s2, sizeof(s2), "%s/fwupdate.tar", BIN_FOLDER);
                printf ("copying %s to %s\n", s1, s2);
                rhea::fs::fileCopy (s1, s2);

                //ora devo "untar": tar -xvf fwupdate.tar
                chdir(BIN_FOLDER);
                sprintf_s ((char*)s1, sizeof(s1), "/bin/tar -xvf %s/fwupdate.tar", BIN_FOLDER);
                printf ("%s\n", s1);
                system((const char*)s1);
                rhea::fs::findClose(ff);


                //nello zip che ho appena scompattato, c'è la cartella REST che va copiata in WWW_FOLDER/REST
                sprintf_s ((char*)s1, sizeof(s1), "%s/REST", BIN_FOLDER);
                sprintf_s ((char*)s2, sizeof(s2), "%s/REST", WWW_FOLDER);
                rhea::fs::folderCopy (s1, s2);

                return true;
            }
        }
    } while (rhea::fs::findNext(ff));
    rhea::fs::findClose(ff);
    return false;
}

/*****************************************************
 * Se trova un file con estensione .rheaRasGuiTP nella cartella USB_FOLDER, lo copia in WWW_FOLDER/GUITP
 * e lo scompatta sovrascrivendo gli eventuali file già esistenti
 * Ritorna true in quel caso, false altrimenti
 */
bool checkGUI_TP (const u8 *usb_path)
{
    OSFileFind ff;
    if (!rhea::fs::findFirst (&ff, usb_path, (const u8*)"*.rheaRasGuiTP"))
        return false;

    do
    {
        if (!rhea::fs::findIsDirectory(ff))
        {
            const u8 *fname = rhea::fs::findGetFileName(ff);
            if (fname[0] != '.')
            {
                //ho trovato un file con l'estensione corretta
                //Lo copio in locale e poi lo scompatto
                u8 s1[512];
                u8 s2[512];

                sprintf_s ((char*)s1, sizeof(s1), "%s/%s", usb_path, fname);
                sprintf_s ((char*)s2, sizeof(s2), "%s/GUITP/guiupdate.rheazip", WWW_FOLDER);
                printf ("copying %s to %s\n", s1, s2);
                rhea::fs::fileCopy (s1, s2);

                sprintf_s ((char*)s1, sizeof(s1), "%s/GUITP", WWW_FOLDER);
                rhea::CompressUtility::decompresAll (s2, s1);
                rhea::fs::findClose(ff);
                rhea::fs::fileDelete (s2);
                return true;
            }
        }
    } while (rhea::fs::findNext(ff));
    rhea::fs::findClose(ff);
    return false;
}


/*****************************************************
 * copia sulla chiavetta USB i file qrcodeTP.png e qrcodeTS.png
 */
void copyQRCodesToUSB (const u8 *usb_path)
{
    u8 s1[512];
    u8 s2[512];

    sprintf_s ((char*)s1, sizeof(s1), "%s/qrcodeTP.png", BIN_FOLDER);
    sprintf_s ((char*)s2, sizeof(s2), "%s/qrcodeTP.png", usb_path);
    rhea::fs::fileCopy (s1, s2);

    sprintf_s ((char*)s1, sizeof(s1), "%s/qrcodeTS.png", BIN_FOLDER);
    sprintf_s ((char*)s2, sizeof(s2), "%s/qrcodeTS.png", usb_path);
    rhea::fs::fileCopy (s1, s2);
}


/*****************************************************
 * Al primissimo avvio del rasPI, c'è uno script python che crea alcuni file nella cartella BIN_FOLDER.
 * Tra questi file, ci sono anche le 2 png che rappresentano i QR da usare per collegarsi al rasPI tramite rheAPP.
 * Dato che per creare i QR è necessario conoscere il MAX-ADDRESS, e dato che per conoscere il MAC-ADDRESS linux ci impiega un po' (parliamo di una 30a di secondi), al primissimo
 * avvio questi file vengono generati con una certa lentezza (30s circa).
 * A me interessa che si esca dal bootLoader con la consapevolezza che questi file esistono, visto che sono utilizati dall'applicazione
 */
void waitForQRCodeToBeGenerated ()
{
    u8 s1[512];

    sprintf_s ((char*)s1, sizeof(s1), "%s/qrcodeTP.png", BIN_FOLDER);
    while (1)
    {
        if (rhea::fs::fileExists(s1))
            break;
        rhea::thread::sleepMSec(1000);
    }

    sprintf_s ((char*)s1, sizeof(s1), "%s/qrcodeTS.png", BIN_FOLDER);
    while (1)
    {
        if (rhea::fs::fileExists(s1))
            break;
        rhea::thread::sleepMSec(1000);
    }
}


//*****************************************************
void runRASPI()
{
    //esegue "RASPI_RELEASE_rasPISerial"
    const char exePathAndName[] {BIN_FOLDER "/RASPI_RELEASE_rasPISerial"};
    const char* argv[4];
    memset (argv,0,sizeof(argv));
    argv[0] = exePathAndName;

    printf ("running %s\n", exePathAndName);
    execvp(exePathAndName, (char* const*)argv);
}

//*****************************************************
int main()
{
    rhea::init("rheaRasPIESAPIBootLoader", NULL);

    //setup del GPIO per manipolare il LED di segnalazione
    wiringPiSetup();
    pinMode(GPIO_LED, OUTPUT);
    LED_ON;

    //path sulla chiavetta USB
    u8 path[128];
    sprintf_s ((char*)path, sizeof(path), "%s/%s", USB_MOUNT_POINT, USB_FOLDER);


    //attendo che lo script python generi i QRcode (serve solo al primissimo avvio dato che poi i file rimangono sull'hard disk)
    waitForQRCodeToBeGenerated();


    //se la cartella esiste, verifico se c'è da aggiornare qualcosa
    bool bAnyUpdate = false;
    if (rhea::fs::folderExists(path))
    {
        copyQRCodesToUSB(path);
        if (checkFWUpdate(path))        bAnyUpdate = true;
        if (checkGUI_TP(path))          bAnyUpdate = true;
    }

    LED_OFF;


    if (bAnyUpdate)
    {
        //"smonto" la USB
        system ("sudo umount -f " USB_MOUNT_POINT);

        //entro in un loop infinito e faccio lampeggiare il LED
        //Mi aspetto che la chiavetta USB venga rimossa e il PI riavviato
        while (1)
        {
            LED_ON;
            rhea::thread::sleepMSec(500);
            LED_OFF;
            rhea::thread::sleepMSec(500);
        }
    }
    else
    {
        rhea::deinit();

        //lancio il programma
        runRASPI();
    }


    return 0;
}
