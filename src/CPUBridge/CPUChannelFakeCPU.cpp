#include "CPUChannelFakeCPU.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaDateTime.h"

using namespace cpubridge;


//*****************************************************************
CPUChannelFakeCPU::CPUChannelFakeCPU()
{
	bShowDialogStopSelezione = true;
	statoPreparazioneBevanda = eStatoPreparazioneBevanda_doing_nothing;
	VMCState = eVMCState_DISPONIBILE;
	memset(&runningSel, 0, sizeof(runningSel));

	memset(cpuMessage1, 0x00, sizeof(cpuMessage1));
	memset(cpuMessage2, 0x00, sizeof(cpuMessage2));

	sprintf_s(cpuMessage1, sizeof(cpuMessage1), "CPU message example 1");
	sprintf_s(cpuMessage2, sizeof(cpuMessage2), "CPU message example 2");
	curCPUMessage = cpuMessage2;
	timeToSwapCPUMsgMesc = 0;
}

//*****************************************************************
CPUChannelFakeCPU::~CPUChannelFakeCPU()
{
}

//*****************************************************************
bool CPUChannelFakeCPU::open(rhea::ISimpleLogger *logger)
{
	assert(logger != NULL);

	logger->log("CPUChannelFakeCPU::open\n");
	logger->incIndent();
	logger->log("OK\n");
	logger->decIndent();
	return true;
}

//*****************************************************************
void CPUChannelFakeCPU::close(rhea::ISimpleLogger *logger)
{
	logger->log("CPUChannelFakeCPU::close\n");
}

//*****************************************************************
void CPUChannelFakeCPU::priv_updateCPUMessageToBeSent(u64 timeNowMSec)
{
	if (timeNowMSec < timeToSwapCPUMsgMesc)
		return;
	timeToSwapCPUMsgMesc = timeNowMSec + 20000;
	if (curCPUMessage == cpuMessage1)
		curCPUMessage = cpuMessage2;
	else
		curCPUMessage = cpuMessage1;
}

/*****************************************************************
 * Qui facciamo finta di mandare il msg ad una vera CPU e forniamo una risposta d'ufficio sempre valida
 */
bool CPUChannelFakeCPU::sendAndWaitAnswer(const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger)
{
	const eCPUCommand cpuCommand = (eCPUCommand)bufferToSend[1];
	u8 msgLen = bufferToSend[2];
	u32 ct = 0;

	switch (cpuCommand)
	{
	default:
		DBGBREAK;
		logger->log("CPUChannelFakeCPU::sendAndWaitAnswer() => ERR, cpuCommand not supported [%d]\n", (u8)cpuCommand);
		*out_answer = NULL;
		*in_out_sizeOfAnswer = 0;
		return false;
		break;

	case eCPUCommand_checkStatus_B:
		//ho ricevuto una richiesta di stato, rispondo in maniera appropriata
		{
			/*unsigned char rawData[147] = {
				0x23, 0x42, 0x40, 0x02, 0x00, 0x00, 0xFF, 0x0F, 0x2C, 0x40, 0x03, 0x40,
				0x41, 0x30, 0x32, 0x39, 0xA7, 0x04, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30,
				0x2E, 0x30, 0x30, 0x40, 0x41, 0x30, 0x32, 0x36, 0xA7, 0x01, 0x20, 0x20,
				0x20, 0x20, 0x20, 0x30, 0x2E, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x0D, 0x00, 0x31, 0x47, 0x42, 0x00, 0x20, 0x20, 0x20, 0x20, 0x30,
				0x2E, 0x30, 0x30, 0x6D, 0x97, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0xFF, 0x00, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
				0x00, 0x1C, 0x01, 0x0C, 0x10, 0x00, 0x0C, 0x0E, 0x60, 0xF4, 0xEB, 0x20,
				0x00, 0x00, 0x00, 0x04, 0x01, 0x20, 0x00, 0x20, 0x91, 0x30, 0x00, 0x00,
				0x28, 0x91, 0x00, 0x21, 0x00, 0xFE, 0x00, 0x00, 0x06, 0x00, 0xF0, 0x00,
				0x00, 0xFE, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0x00, 0x00
			};
			*in_out_sizeOfAnswer = 64;
			memcpy(out_answer, rawData, *in_out_sizeOfAnswer);
			return true;*/
			


			priv_updateCPUMessageToBeSent(rhea::getTimeNowMSec());

			const u8 tastoPremuto = bufferToSend[3];

			if (statoPreparazioneBevanda == eStatoPreparazioneBevanda_doing_nothing)
			{
				if (tastoPremuto != 0)
				{
					//hanno richiesto una selezione!!!
					statoPreparazioneBevanda = eStatoPreparazioneBevanda_wait;
					runningSel.selNum = tastoPremuto;
					runningSel.timeStartedMSec = rhea::getTimeNowMSec();
					VMCState = eVMCState_PREPARAZIONE_BEVANDA;
				}
			}
			else
			{
				//sto facendo una selezione, rispondo "eStatoPreparazioneBevanda_wait" per un paio di secondi, poi vado in running per un altro paio di secondo
				const u64 timeElapsedMSec = rhea::getTimeNowMSec() - runningSel.timeStartedMSec;
				bool bFinished = false;
				if (timeElapsedMSec < 1500)
				{
					statoPreparazioneBevanda = eStatoPreparazioneBevanda_wait;
					if (tastoPremuto != 0)
						bFinished = true; //simula il tasto stop
				}
				else if (timeElapsedMSec < 12000)
				{
					statoPreparazioneBevanda = eStatoPreparazioneBevanda_running;
					if (tastoPremuto != 0)
						bFinished = true; //simula il tasto stop
				}
				else
				{
					//ok, è tempo di terminare la selezione
					bFinished = true;
				}

				if (bFinished)
				{
					VMCState = eVMCState_DISPONIBILE;
					statoPreparazioneBevanda = eStatoPreparazioneBevanda_doing_nothing;
				}
			}

			priv_buildAnswerTo_checkStatus_B(out_answer, in_out_sizeOfAnswer);
			*in_out_sizeOfAnswer = out_answer[2];
			return true;
		}
		break;

	case eCPUCommand_initialParam_C:
		//ho ricevuto i parametri iniziali, devo rispondere in maniera appropriata
		{
			assert(*in_out_sizeOfAnswer >= 116);

			u16 year, month, day;
			rhea::Date::getDateNow(&year, &month, &day);

			u8 hour, minute, seconds;
			rhea::Time24::getTimeNow(&hour, &minute, &seconds);


			out_answer[ct++] = '#';
			out_answer[ct++] = 'C';
			out_answer[ct++] = 0; //lunghezza

			out_answer[ct++] = (u8)(year - 2000);
			out_answer[ct++] = (u8)(month);
			out_answer[ct++] = (u8)(day);

			out_answer[ct++] = (u8)(hour);
			out_answer[ct++] = (u8)(minute);
			out_answer[ct++] = (u8)(seconds);

			//versione sw	8 caratteri del tipo "1.4 WIP"
			out_answer[ct++] = 'F';
			out_answer[ct++] = 'A';
			out_answer[ct++] = 'K';
			out_answer[ct++] = 'E';
			out_answer[ct++] = ' ';
			out_answer[ct++] = 'C';
			out_answer[ct++] = 'P';
			out_answer[ct++] = 'U';

			//98 btyes composti da 49 prezzi ciascuno da 2 bytes (byte basso byte alto)
			for (u8 i = 0; i < 49; i++)
			{
				out_answer[ct++] = (i + 1);
				out_answer[ct++] = 0;
			}

			//Da qui in poi sono dati nuovi, introdotti a dicembre 2018
			//115			versione protocollo.Inizialmente = 1, potrebbe cambiare in futuro
			out_answer[ct++] = 3;

			//116 ck
			out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
			ct++;
			out_answer[2] = (u8)ct;

			*in_out_sizeOfAnswer = out_answer[2];
			return true;
		}
		break;

	case eCPUCommand_restart:
		*in_out_sizeOfAnswer = 0;
		*out_answer = NULL;
		return true;
	}
}

//*****************************************************************
void CPUChannelFakeCPU::priv_buildAnswerTo_checkStatus_B(u8 *out_answer, u16 *in_out_sizeOfAnswer) const
{
	u32 ct = 0;

	/*
	0		#
	1		B
	2		lunghezza in byte del messaggio
	*/
	out_answer[ct++] = '#';
	out_answer[ct++] = 'B';
	out_answer[ct++] = 0; //lunghezza

	/*
	3		eVMCState
	4		VMCerrorCode
	5		VMCerrorType
	*/
	out_answer[ct++] = (u8)VMCState;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;

	/*
	6	??
	7	??
	8	??
	*/
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;

	//9		CupAbsentStatus_flag & bShowDialogStopSelezione && statoPreparazioneBevanda
	out_answer[ct] = ((u8)statoPreparazioneBevanda << 5);
	if (bShowDialogStopSelezione)
		out_answer[ct] |= 0x10;
	++ct;

	//10		selection_CPU_current
	out_answer[ct++] = 0;


	//messaggio di CPU
	if (out_answer[1] == 'B')
	{
		memset(&out_answer[ct], 0x00, 32);
		u32 n = (u32)strlen(curCPUMessage);
		if (n)
			memcpy(&out_answer[ct], curCPUMessage, n);
		ct += 32;
	}
	else
	{
		//11-74		32 unicode char col messaggio di stato
		assert(ct == 11);
		memset(&out_answer[ct], 0x00, 64);
		for (u8 i = 0; i < 32; i++)
		{
			out_answer[ct++] = 0;
			out_answer[ct++] = curCPUMessage[i];
			if (curCPUMessage[i] == 0x00)
				break;
		}

		ct = 11 + 64;
	}

	/*
	75		6 byte con lo stato di disponibilità delle 48 selezioni
	76		ATTENZIONE che il bit a zero significa che la selezione è disponibile, il bit
	77		a  significa che NON è disponibile
	78
	79
	80
	*/
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;
	out_answer[ct++] = 0;

	/*
	81		beepTime dSec LSB
	82		beepTime dSec MSB
	*/
	out_answer[ct++] = 3;
	out_answer[ct++] = 0;


	//83		linguaggio (default=='0' o ML=='1')
	out_answer[ct++] = '0';

	//84-85		se linguaggio ML=='1', questi 2 byte indicano la lingua in ASCII
	out_answer[ct++] = 'G';
	out_answer[ct++] = 'B';

	//86		1 byte per indicare importanza del msg di CPU (0=poco importante, 1=importante)
	out_answer[ct++] = 1;

	//87-94		8 byte stringa con l'attuale credito
	out_answer[ct++] = '0';
	out_answer[ct++] = '.';
	out_answer[ct++] = '0';
	out_answer[ct++] = '0';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';


	//116 ck
	out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
	ct++;
	out_answer[2] = (u8)ct;
}