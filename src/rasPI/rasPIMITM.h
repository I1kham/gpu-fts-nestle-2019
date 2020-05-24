/***************************************************************************************
	Modulo M.I.T.M (Man In The Middle)

	GPU <---> MITM <---> CPU

	MITM riceve msg da GPU nel formato classico dei msg GPU CPU.
	Analizza il messaggio. Se il messaggio è specifico per MITM (comando W nel protocollo GPU/CPU), allora processa tale messaggio
	e risponde direttamente a GPU con un messaggio W, altrimenti passa il messaggio a CPU e si mette in attesa di una risposta
	con un timeout di 5 secondi (vedi rasPI::MITM::Core::TIMEOUT_CPU_ANSWER_MSec).
	
	NB: I 5 secondi sono una sovrastima del tempo massimo di risposta della CPU.
		Nel funzionamento normale GPU/CPU, la GPU generalmente usa un timeout di 1/1.5 secondi, a seconda del tipo di comando. In certe situazioni
		il timeout arriva anche a 3 o 5 secondi. Dato che il rasPI non ha modo di sapere il timeout di GPU per lo specifico comando,
		sono costretto a farne una sovrastima.

	A volte MITM ha bisogno di chiedere cose alla GPU e lo può fare solo tramite il canale seriale.
	Per evitare di incasinare la trasmissione, MITM attende che GPU invii un qualunque comando (cosa che di solito avviene ogni 300 ms).
	Alla ricezione di "un qualunque comando", MITM procede come al solito (quindi invia a CPU e aspetta risposta) e poi, nel riportare la risposta
	di CPU a GPU, appende in testa tutti i "suoi comandi" che aveva temporaneamente bufferizzato.

	GPU quindi, quando manda un generico comando a CPU, si aspetta o di ricevere la risposta di CPU, o di ricevere n comandi W dal MITM seguiti dalla risposta
	di CPU al comando che aveva inviato.
*/
#ifndef _rasPIMITM_h_
#define _rasPIMITM_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"

namespace rasPI
{
	namespace MITM
	{
		bool        start (rhea::ISimpleLogger *logger, const char *serialPortGPU, const char *serialPortCPU, rhea::HThread *out_hThread, HThreadMsgW *out_msgQW_toMITM);
	} //namespace MITM
} // namespace rasPI
#endif // _rasPIMITM_h_
