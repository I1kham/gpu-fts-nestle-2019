#ifndef _AlipayChina_h_
#define _AlipayChina_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"

namespace rhea
{
    namespace AlipayChina
    {
		bool        startThread (const char *serverIP, u16 serverPort, const char *machineID, const char *cryptoKey, rhea::ISimpleLogger *logger, HThreadMsgW *out_hWrite);
				/*	crea il thread che gestisce la comunicazione con il server cinese per il pagamento Alipay.
					Ritorna true se tutto ok e filla [out_hWrite] con l'handle da utilizzare per comunicare con il thread
				*/

		void		startOrder (HThreadMsgW h, const u8 *selectionName, u8 selectionNum, const char *selectionPrice);
		void		endOrder (HThreadMsgW h, bool bSelectionWasDelivered);
    } //namespace AlipayChina
} // namespace rhea

#endif // _AlipayChina_h_
