/***************************************************************************************
	Modulo M.I.T.M (Man In The Middle)

	GPU <---> MITM <---> CPU
				^
				|
			subscriber

	MITM riceve msg da GPU nel formato classico dei msg GPU CPU.
	Analizza il messaggio. Se il messaggio è specifico per MITM (comando W nel protocollo GPU/CPU), allora decodifica il messaggio (vedi
	rhea::thread::deserializMsg()) e lo passa al suo attuale subscriber.
	Se il msg letto non è 'W', allora lo passa pari pari a CPU e si mette in attesa di una risposta con un timeout di 5 secondi (vedi rasPI::MITM::Core::TIMEOUT_CPU_ANSWER_MSec).
	Alla ricezione della risposta da parte di CPU, passa il msg pari pari a GPU.
	
	NB: I 5 secondi sono una sovrastima del tempo massimo di risposta della CPU.
		Nel funzionamento normale GPU/CPU, la GPU generalmente usa un timeout di 1/1.5 secondi, a seconda del tipo di comando. In certe situazioni
		il timeout arriva anche a 3 o 5 secondi. Dato che il rasPI non ha modo di sapere il timeout di GPU per lo specifico comando,
		sono costretto a farne una sovrastima.

	Il subscriber di MITM può comunicare direttamente con la GPU (in particolare, comunica con CPUBridge) inviando a MITM dei messagi sMsg
	usando la msgQ del thread di MITM.
	MITM infila questi messaggi nella comunicazione CPU/GPU un maniera trasparente.
	Dal punto di vista del subscriber, è come se fosse direttamente collegato a CPUBridge.
*/
#ifndef _rasPIMITM_h_
#define _rasPIMITM_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"

namespace rasPI
{
	namespace MITM
	{
		bool        start (rhea::ISimpleLogger *logger, const char *serialPortGPU, const char *serialPortCPU, rhea::HThread *out_hThread, HThreadMsgW *out_msgQW_toMITM);
		bool		subscribe(const HThreadMsgW &msgQW_toMITM, cpubridge::sSubscriber *out);
	} //namespace MITM
} // namespace rasPI
#endif // _rasPIMITM_h_
