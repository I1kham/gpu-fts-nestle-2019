#ifndef _main_h_
#define _main_h_
#include <qstring.h>

#define VERSION             "0.9"

#define UNUSED_PARAM        __attribute__((unused))
#define USB_MOUNTPOINT      "/media/SDA1"
#define RHEA_USB_FOLDER     USB_MOUNTPOINT "/rhea/GPU210"


extern QString APPLICATION_FOLDER;


void hideMouse();

#endif // _main_h_
