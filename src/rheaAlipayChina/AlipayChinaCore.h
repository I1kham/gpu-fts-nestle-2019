#ifndef _AlipayChinaCore_h_
#define _AlipayChinaCore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"

namespace rhea
{
    namespace AlipayChina
    {
        class Core
        {
        public:
                                    Core();
                                    ~Core() { }

            void                    useLogger (rhea::ISimpleLogger *loggerIN);

            bool                    setup (const char *serverIP, u16 serverPort, const char *machineID, const char *cryptoKey, HThreadMsgW *out_hWrite);
            void                    run ();

            void                    onSelectionFinished(bool bSelectionWasDelivered);
            //comunica al server che la selezione è terminata.
            //Se [bSelectionWasDelivered] == true, allora selezione terminata con successo
            //Se [bSelectionWasDelivered] == false, la selezione non è andata a buon fine

        private:
            const u32               SIZE_OF_RCV_BUFFER = 4096;
            const u32               TIMEOUT_FOR_PAYMENT_MSec = 60000; //tempo massimo che l'utente ha per pagare
            const u32               PAYMENT_POLL_MSec = 2000; //posto che un ordine sia stato inizializzato, ogni quanto chiedo al server se il cliente ha pagato?

        private:
            enum eResponseCommand
            {
                eResponseCommand_E11 = 11,
                eResponseCommand_E12 = 12,
                eResponseCommand_E13 = 13,
                eResponseCommand_E14 = 14,
                eResponseCommand_E15 = 15,
                eResponseCommand_none = 0xff
            };

            struct sResponseE11
            {
                char status;
            };
            struct sResponseE12
            {
                char status;
            };
            struct sResponseE13
            {
                char orderNumber[32];
                char qrCodeURL[256];
                char paymentMethod[256];
            };
            struct sResponseE14
            {
                char orderNumber[32];
                char status;
            };
            struct sResponseE15
            {
                char orderNumber[32];
                char status;
            };

            struct sResponse
            {
                eResponseCommand    command;
                char                timestamp[16];
                union
                {
                    sResponseE11    asE11;
                    sResponseE12    asE12;
                    sResponseE13    asE13;
                    sResponseE14    asE14;
                    sResponseE15    asE15;
                };
            };

            enum eOrderStatus
            {
                eOrderStatus_none = 0,
                eOrderStatus_requestSent = 1,
                eOrderStatus_pollingPaymentStatus = 2,
                eOrderStatus_paymentOK = 3,
                eOrderStatus_waitingBeverageEnd = 4,
                eOrderStatus_finishedOK = 5,
                eOrderStatus_finishedKO = 6
            };

            enum eOrderCloseStatus
            {
                eOrderCloseStatus_SUCCESS = 0,
                eOrderCloseStatus_FAILED = 1,
                eOrderCloseStatus_TIMEOUT = 2,
            };

            struct sOrder
            {
                eOrderStatus    status;
                u8              param_selectionName[128];
                u8              param_selectionNumber;
                char            param_selectionPrice[16];
                char            orderNumber[32];
                char            qrCodeURL[256];
                char            acceptedPaymentMethods[256];
                u64             timeoutMSec;
                u64             nextTimePollPaymentoMSec;
            };

        private:
            char                    serverIP[32];
            u16                     serverPort;
            char                    machineID[128];
            char                    cryptoKey[128];
            rhea::Allocator         *localAllocator;
            rhea::ISimpleLogger     *logger;
            rhea::NullLogger        nullLogger;
            HThreadMsgR             hMsgQRead;
            HThreadMsgW             hMsgQWrite;
            u8                      *rcvBuffer;
            u32                     nInRCVBuffer;
            u64                     lastTimeHeartBeatRCVMSec;
            bool                    bQuit;
            bool                    bIsLoggedIntoChinaServer;
            sOrder                  theOrder;
            FastArray<HThreadMsgW>  toBeNotifiedThreadList;

        private:
            bool                    priv_handleIncomingSocketMsg(OSSocket &sok);
            u32                     priv_parserRCVCommand (const u8 *buffer, u32 sizeOfBuffer, sResponse *out);
            void                    priv_handleIncomingThreadMsg();
            bool                    priv_openSocket (OSSocket *sok);
            void                    priv_getTimestamp(char *out, u32 sizeofOut) const;
            void                    priv_sendLogin(OSSocket &sok);
            void                    priv_sendHeartBeat(OSSocket &sok);
            bool                    priv_orderStart (OSSocket &sok, const u8 *selectionName, u8 selectionNum, const char *selectionPrice);
            void                    priv_orderAskForPaymentStatus (OSSocket &sok);
            void                    priv_orderClose (OSSocket &sok, eOrderCloseStatus status);
            u32                     priv_buildCommand (const char *command, const u8 *optionalData, u8 *out_buffer, u32 sizeofOutBuffer) const;
            void                    priv_sendCommand (OSSocket &sok, const u8 *buffer, u32 nBytesToSend);
            void                    priv_doNotifyAll (u16 what, u32 paramU32);
            void                    priv_notify_ONLINE_STATUS_CHANGED();
        };
    } //namespace AlipayChina
} // namespace rhea

#endif // _AlipayChinaCore_h_
