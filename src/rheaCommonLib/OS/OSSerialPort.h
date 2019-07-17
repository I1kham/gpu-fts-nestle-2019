#ifndef _OSSerialPort_h_
#define _OSSerialPort_h_
#include "OSInclude.h"
#include "OSEnum.h"

inline bool     OSSerialPort_open (OSSerialPort *out_serialPort, const char *deviceName,
                                    OSSerialPortConfig::eBaudRate baudRate,
                                    bool RST_on,
                                    bool DTR_on,
                                    OSSerialPortConfig::eDataBits dataBits =    OSSerialPortConfig::Data8,
                                    OSSerialPortConfig::eParity parity =        OSSerialPortConfig::NoParity,
                                    OSSerialPortConfig::eStopBits stop =        OSSerialPortConfig::OneStop,
                                    OSSerialPortConfig::eFlowControl flowCtrl = OSSerialPortConfig::NoFlowControl)  { return platform::serialPort_open (out_serialPort, deviceName, baudRate, RST_on, DTR_on, dataBits, parity, stop, flowCtrl); }
                /* apre la seriale e la imposta in modalità "non bloccante", RTS=on, DTR=on
                 */

inline void     OSSerialPort_close (OSSerialPort &sp)                                                           { return platform::serialPort_close(sp); }

inline void     OSSerialPort_setRTS (OSSerialPort &sp, bool bON_OFF)                                            { platform::serialPort_setRTS(sp, bON_OFF); }
inline void     OSSerialPort_setDTR (OSSerialPort &sp, bool bON_OFF)                                            { platform::serialPort_setDTR(sp, bON_OFF); }

inline bool     OSSerialPort_setAsBlocking (OSSerialPort &sp, u8 minNumOfCharToBeReadInOrderToSblock)           { return platform::serialPort_setAsBlocking(sp, minNumOfCharToBeReadInOrderToSblock); }
                /* imposta la seriale come "bloccante". Questo vuol dire che le chiamate a readBuffer() sono bloccanti e ritorneranno
                 * solo quando almeno [minNumOfCharToBeReadInOrderToSblock] sono stati letti dalla seriale
                 */

inline bool     OSSerialPort_setAsNonBlocking (OSSerialPort &sp, u8 numOfDSecToWaitBeforeReturn = 0)            { return platform::serialPort_setAsNonBlocking(sp, numOfDSecToWaitBeforeReturn); }
                /* imposta la seriale come "non bloccante". Le chiamate a readBuffer() quindi ritornano subito SE c'è almeno un byte pronto da leggere altrimenti
                 * aspettano un massimo di [numOfDSecToWaitBeforeReturn] dSec dopodichè ritornano anche se non ci sono byte da leggere
                 */

inline void     OSSerialPort_flushIO (OSSerialPort &sp)                                                         { platform::serialPort_flushIO(sp); }
                /* flusha i buffer di input e output discardando tutto quanto
                 */



inline u32      OSSerialPort_readBuffer (OSSerialPort &sp, void *out_byteRead, u32 numMaxByteToRead)            { return platform::serialPort_readBuffer(sp, out_byteRead, numMaxByteToRead); }
                /* legge al massimo [numMaxByteToRead] bytes dalla seriale e li memorizza in [out_byteRead]
                 * Ritorna il numero di byte letti e memorizzati in [out_byteRead]
                 *
                 * Se la seriale è in modalità bloccante, questa fn è a sua volta bloccante
                 */

inline bool     OSSerialPort_readByte (OSSerialPort &sp, u8 *out_b)                                             { return (platform::serialPort_readBuffer(sp, out_b, 1) == 1); }
                /* true se ha letto un btye dalla seriale nel qual caso [out_b] contiene il byte letto
                 * Valgono le stesse indicazioni valide per readBuffer()
                 */


inline u32      OSSerialPort_writeBuffer (OSSerialPort &sp, const void *buffer, u32 nBytesToWrite)              { return platform::serialPort_writeBuffer(sp, buffer, nBytesToWrite); }
                /* prova a scrivere fino a [nBytesToWrite].
                 * Ritorna il numero di bytes scritti con successo
                 *
                 * Ritorna 0 o <0 in caso di errore
                 */

inline bool     OSSerialPort_writeByte (OSSerialPort &sp, u8 byteToWrite)                                       { return (platform::serialPort_writeBuffer(sp, &byteToWrite, 1) == 1); }
#endif //_OSSerialPort_h_
