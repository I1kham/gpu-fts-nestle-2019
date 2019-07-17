#ifndef _GUIBridge_h_
#define _GUIBridge_h_
#include "GUIBridgeServer.h"




namespace guibridge
{
    bool        startServer (HThreadMsgR *out_hQMessageFromWebserver, HThreadMsgW *out_hQMessageToWebserver);

    void        sendAjaxAnwer (WebsocketServer *server, HWebsokClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData);

    bool        prepareEventBuffer (eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength);
    void        sendEvent (WebsocketServer *server, HWebsokClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData);

} // namespace guibridge


#endif // _GUIBridge_h_

