#ifndef _rhFSProtocolEnumAndDefine_h_
#define _rhFSProtocolEnumAndDefine_h_
#include "../rheaCommonLib/rheaEnumAndDefine.h"



namespace rhFSx
{
	namespace proto
	{
		//Diverse applicazioni possono collegarsi a SMU. Nel messaggio eAsk::allApp_identify devono inviare un 32bit che identifica il tipo
		//di applicazione. L'enum seguente rappresenta tutte le applicazioni ad oggi riconosciute da SMU
		enum class eApplicationType : u32
		{
			RSPROTO					= 0x63A5BE78,	//app che fa da ponte tra i servizi cloud di SECO e la SMU
			unknown					= 0xFFFFFFFF
		};


		//l'enum eAsk racchiude tutti i comandi riconosciuti da SMU
		//I comandi sono divisi in blocchi specifici per singole applicazioni.
		//Le define sottostanti definiscono l'inidice di inizio e di fine dei messaggi che le specifiche applicazioni possono
		//inviare
		enum class eAsk : u16
		{
			//========================================================================
			//messaggi di protocollo, validi per tutti quelli che si connettono a SMU indipendentemente
			//dal tipo di applicazione che rappresentano
			allApp_ping						= 0x0001,
			allApp_pong						= 0x0002,	//risposta di SMU ad un eventuale msg allApp_ping
			allApp_identify					= 0x0003,	//inviato automaticamente rhea::SMUConn alla connessione con SMU
			allApp_identifyAnsw				= 0x0004,	//risposta di SMU al msg precedente (allApp_identify)
			allApp_die						= 0x0005,	//inviato da SMU ai suoi client per chiederne la morte

			//========================================================================
			//messaggi della APP RSProto
			rsproto_service_cmd				= 0x9000,
			rsproto_variabile				= 0x9001,
			rsproto_file					= 0x9002,


			invalid				= 0xFFFF
		};

		struct sDecodedMsg
		{
			u16			what;
			u16			userValue;
			const u8*	payload;
			u32			payloadLen;
		};


	} //namespace proto
} //namespace smu

#endif //_rhFSProtocolEnumAndDefine_h_