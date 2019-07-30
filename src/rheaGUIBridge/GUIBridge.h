#ifndef _GUIBridge_h_
#define _GUIBridge_h_
#include "GUIBridgeServer.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"



namespace guibridge
{
    bool        startServer (rhea::HThread *out_hServer, HThreadMsgR *out_hQMessageFromWebserver, HThreadMsgW *out_hQMessageToWebserver, rhea::ISimpleLogger *logger);

    void        sendAjaxAnwer (rhea::ProtocolServer *server, HWebsokClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData);

    bool        prepareEventBuffer (eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength);
    void        sendEvent (rhea::ProtocolServer *server, HWebsokClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData);


    void        CmdHandler_selAvailability_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u16 handlerID, int numSel, const void *oneBitPerSelectionArray);
    void        CmdHandler_selPrices_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u16 handlerID, int numSel, const unsigned int *priceListIN);
    void        CmdHandler_selStatus_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u8 status);
} // namespace guibridge


#endif // _GUIBridge_h_

