#ifndef _AlipayChina_h_
#define _AlipayChina_h_
#include "AlipayChinaEnumAndDefine.h"

namespace rhea
{
    namespace AlipayChina
    {
		bool        startThread (const char *serverIP, u16 serverPort, const char *machineID, const char *cryptoKey, rhea::ISimpleLogger *logger, Context *out_context);
						/*	crea il thread che gestisce la comunicazione con il server cinese per il pagamento Alipay.
							Ritorna true se tutto ok e filla [out_context] con il context da utilizzare per comunicare con il thread
						*/

		void		subscribe (Context &ctx, HThreadMsgW &hNotifyHere);
					/*	Qualcuno vuole iscriversi alla coda di notifiche di questo thread
						Da ora in poi, ogni notifica generata dal thread verrà inviata anche a hNotifyHere
					*/

		void		kill (Context &ctx);


		void		ask_ONLINE_STATUS (Context &ctx);
					//il thread risponde con una notifica ALIPAYCHINA_NOTIFY_ONLINE_STATUS_CHANGED


		//void		startOrder (Context &ctx, const u8 *selectionName, u8 selectionNum, const char *selectionPrice);
		//void		endOrder (Context &ctx, bool bSelectionWasDelivered);
    } //namespace AlipayChina
} // namespace rhea

#endif // _AlipayChina_h_
