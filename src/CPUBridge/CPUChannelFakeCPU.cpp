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
	curCPUMessageImportanceLevel = 1;
	timeToSwapCPUMsgMesc = 0;
	macine[0].reset();
	macine[0].posizioneMacina = 100 +(u16)rhea::randomU32(100);

	macine[1].reset();
	macine[1].posizioneMacina = 100 + (u16)rhea::randomU32(100);

	timeToEndTestSelezioneMSec = 0;
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

	memset(&cleaning, 0, sizeof(cleaning));
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
	{
		curCPUMessageImportanceLevel = 0;
		curCPUMessage = cpuMessage2;
	}
	else
	{
		curCPUMessageImportanceLevel = 1;
		curCPUMessage = cpuMessage1;
	}
}



/*****************************************************************
 * Qui facciamo finta di mandare il msg ad una vera CPU e forniamo una risposta d'ufficio sempre valida
 */
bool CPUChannelFakeCPU::sendAndWaitAnswer(const u8 *bufferToSend, u16 nBytesToSend UNUSED_PARAM, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec UNUSED_PARAM)
{
	const eCPUCommand cpuCommand = (eCPUCommand)bufferToSend[1];
    //u8 msgLen = bufferToSend[2];
	u32 ct = 0;

	macine[0].update(rhea::getTimeNowMSec());
	macine[1].update(rhea::getTimeNowMSec());

	switch (cpuCommand)
	{
	default:
        DBGBREAK;
		logger->log("CPUChannelFakeCPU::sendAndWaitAnswer() => ERR, cpuCommand not supported [%d]\n", (u8)cpuCommand);
		*in_out_sizeOfAnswer = 0;
		return false;
		break;

	case eCPUCommand_writePartialVMCDataFile:
		{
            //const u8 packet_uno_di = bufferToSend[3];
            //const u8 packet_num_toto = bufferToSend[4];
			const u8 packet_offset = bufferToSend[5];

			out_answer[ct++] = '#';
			out_answer[ct++] = 'X';
			out_answer[ct++] = 0; //lunghezza
			out_answer[ct++] = packet_offset;

			out_answer[2] = (u8)ct + 1;
			out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
			*in_out_sizeOfAnswer = out_answer[2];
			return true;
		}
		break;

	case eCPUCommand_getVMCDataFileTimeStamp:
		{
			cpubridge::sCPUVMCDataFileTimeStamp ts;
			ts.setInvalid();
			out_answer[ct++] = '#';
			out_answer[ct++] = 'T';
			out_answer[ct++] = 0; //lunghezza
			ts.writeToBuffer(&out_answer[ct]);
			ct += ts.getLenInBytes();
			
			out_answer[2] = (u8)ct + 1;
			out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
			*in_out_sizeOfAnswer = out_answer[2];
			return true;
		}
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
				out_answer[ct++] = (i+1);
				out_answer[ct++] = 0;
			}

			//Da qui in poi sono dati nuovi, introdotti a dicembre 2018
			//115			versione protocollo.Inizialmente = 1, potrebbe cambiare in futuro
			out_answer[ct++] = 5;

			//116 ck
			out_answer[2] = (u8)ct+1;
			out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
			ct++;

			*in_out_sizeOfAnswer = out_answer[2];
			return true;
		}
		break;


	case eCPUCommand_getExtendedConfigInfo:
		{
			//const u8 machine_type = (u8)cpubridge::eCPUMachineType_instant;	const u8 isInduzione = 0;
			const u8 machine_type = (u8)cpubridge::eCPUMachineType_espresso1;	const u8 isInduzione = 1;
			//const u8 machine_type = (u8)cpubridge::eCPUMachineType_espresso2;	const u8 isInduzione = 0;
			out_answer[ct++] = '#';
			out_answer[ct++] = cpuCommand;
			out_answer[ct++] = 0; //lunghezza
			out_answer[ct++] = 0x02;	//versione
			out_answer[ct++] = machine_type;
			out_answer[ct++] = 0x82;	//modello macchina
			out_answer[ct++] = isInduzione;
			out_answer[2] = (u8)ct + 1;
			out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
			*in_out_sizeOfAnswer = out_answer[2];
		}
		return true;

	case eCPUCommand_restart:
		*in_out_sizeOfAnswer = 0;
		return true;

	case eCPUCommand_programming:
		{
			// [#] [P] [len] [subcommand] [optional_data] [ck]
			const eCPUProgrammingCommand subcommand = (eCPUProgrammingCommand)bufferToSend[3];
			switch (subcommand)
			{
			default:
				return false;

			case eCPUProgrammingCommand_testSelezione:
				if (timeToEndTestSelezioneMSec == 0)
				{
					timeToEndTestSelezioneMSec = rhea::getTimeNowMSec() + 8000;
					VMCState = eVMCState_TEST_ATTUATORE_SELEZIONE;
				}
				
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4];
				out_answer[ct++] = bufferToSend[5];

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_setTime:
			case eCPUProgrammingCommand_setDate:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4];
				out_answer[ct++] = bufferToSend[5];
				out_answer[ct++] = bufferToSend[6];

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_setFattoreCalibrazioneMotore:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4]; //motore
				out_answer[ct++] = bufferToSend[5];
				out_answer[ct++] = bufferToSend[6];

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_getStatoGruppo:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = (rhea::random01() > 0.8f) ? 0 : 1;
				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_calcolaImpulsiMacina:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4];
				out_answer[ct++] = bufferToSend[5];
				out_answer[ct++] = bufferToSend[6];

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_getTime:
			{
				rhea::DateTime dt;
				dt.setNow();
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = dt.time.getHour();
				out_answer[ct++] = dt.time.getMin();
				out_answer[ct++] = dt.time.getSec();

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
			}
			break;

			case eCPUProgrammingCommand_getDate:
			{
				rhea::DateTime dt;
				dt.setNow();
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = (u8)(dt.date.getYear() -2000);
				out_answer[ct++] = dt.date.getMonth();
				out_answer[ct++] = dt.date.getDay();

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
			}
			break;

			case eCPUProgrammingCommand_getNomiLinguaCPU:
			{
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = 0; //is unicode

				u8 i2 = ct;
				for (u8 i = 0; i<64; i++)
					out_answer[ct++] = ' ';
				out_answer[i2] = 'E';
				out_answer[i2+1] = 'N';
				out_answer[i2+2] = '1';

				i2 += 32;
				out_answer[i2] = 'E';
				out_answer[i2 + 1] = 'N';
				out_answer[i2 + 2] = '2';

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
			}
			break;

			case eCPUProgrammingCommand_getStatoCalcoloImpulsi:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = 0;
				if (rhea::random01() > 0.7)
				{
					out_answer[ct++] = 0;
					out_answer[ct++] = 0;
				}
				else
				{
					out_answer[ct++] = 100;
					out_answer[ct++] = 1;
				}

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;
				break;

			case eCPUProgrammingCommand_cleaning:
				//fingo un cleaning
				memset(&cleaning, 0, sizeof(cleaning));
				cleaning.cleaningType = (eCPUProgrammingCommand_cleaningType)bufferToSend[4];
				cleaning.fase = 1;
				cleaning.timeToEnd = rhea::getTimeNowMSec() + 4000;
				cleaning.prevState = this->VMCState;

				if (cleaning.cleaningType == eCPUProgrammingCommand_cleaningType_sanitario)
					this->VMCState = eVMCState_LAVAGGIO_SANITARIO;
				else
					this->VMCState = eVMCState_LAVAGGIO_MANUALE;

				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_querySanWashingStatus:
				if (cleaning.cleaningType == eCPUProgrammingCommand_cleaningType_sanitario)
				{
					out_answer[ct++] = '#';
					out_answer[ct++] = 'P';
					out_answer[ct++] = 0; //lunghezza
					out_answer[ct++] = (u8)subcommand;
					out_answer[ct++] = cleaning.fase; //fase
					out_answer[ct++] = cleaning.btn1;
					out_answer[ct++] = cleaning.btn2;

					out_answer[2] = (u8)ct + 1;
					out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
					*in_out_sizeOfAnswer = out_answer[2];
					return true;
				}
				break;

			case eCPUProgrammingCommand_setDecounter:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4]; //which one
				out_answer[ct++] = bufferToSend[5];	//value LSB
				out_answer[ct++] = bufferToSend[6]; //value MSB
				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_getAllDecounterValues:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1001); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1002); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1003); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1004); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1005); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1006); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1007); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1008); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1009); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1010); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1011); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1012); ct += 2;
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 1013); ct += 2;

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_getPosizioneMacina:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4]; //macina

				if (bufferToSend[4] == 11)
					rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], macine[0].posizioneMacina);
				else if (bufferToSend[4] == 12)
					rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], macine[1].posizioneMacina);
				else
				{
					DBGBREAK;
				}
				ct += 2;
				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_setMotoreMacina:
				if (bufferToSend[4] == 11)
					macine[0].tipoMovimentoMacina = bufferToSend[5];
				else if (bufferToSend[4] == 12)
					macine[1].tipoMovimentoMacina = bufferToSend[5];
				else
				{
					DBGBREAK;
				}

				
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;
				out_answer[ct++] = bufferToSend[4]; //macina
				out_answer[ct++] = bufferToSend[5]; //tipo di movimento
				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_EVAresetPartial:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_getVoltAndTemp:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;

				out_answer[ct++] = 30; //temp_camera
				out_answer[ct++] = 31; //temp_tBollitore
				out_answer[ct++] = 32; //temp_cappuccinatore
				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 220); //voltage
				ct += 2;

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_getCPUOFFReportDetails:
				{
					const u8 startIndex = bufferToSend[4];
					out_answer[ct++] = '#';
					out_answer[ct++] = 'P';
					out_answer[ct++] = 0; //lunghezza
					out_answer[ct++] = (u8)subcommand;

					out_answer[ct++] = startIndex; //start index
					if (startIndex == 0)
					{
						out_answer[ct++] = 6; //last index

						out_answer[ct++] = '7'; out_answer[ct++] = 'A';  out_answer[ct++] = 16; out_answer[ct++] = 32; out_answer[ct++] = 3; out_answer[ct++] = 1; out_answer[ct++] = 20; out_answer[ct++] = 1;
						out_answer[ct++] = '8'; out_answer[ct++] = ' '; out_answer[ct++] = 18; out_answer[ct++] = 2; out_answer[ct++] = 4; out_answer[ct++] = 1; out_answer[ct++] = 20; out_answer[ct++] = 0;
					}
					else
					{
						out_answer[ct++] = 19; //last index

						out_answer[ct++] = '9'; out_answer[ct++] = 'B';  out_answer[ct++] = 8; out_answer[ct++] = 49; out_answer[ct++] = 5; out_answer[ct++] = 1; out_answer[ct++] = 20; out_answer[ct++] = 0;
						out_answer[ct++] = '5'; out_answer[ct++] = 'C'; out_answer[ct++] = 1; out_answer[ct++] = 32; out_answer[ct++] = 6; out_answer[ct++] = 1; out_answer[ct++] = 20; out_answer[ct++] = 1;
					}
					out_answer[2] = (u8)ct + 1;
					out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
					*in_out_sizeOfAnswer = out_answer[2];
				}
				return true;

			case eCPUProgrammingCommand_getLastFluxInformation:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;

				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 80 + (u16)rhea::randomU32(800)); //last flux
				ct += 2;

				rhea::utils::bufferWriteU16_LSB_MSB(&out_answer[ct], 80 + (u16)rhea::randomU32(250)); //last grinder position
				ct += 2;

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;

			case eCPUProgrammingCommand_getStringVersionAndModel:
				out_answer[ct++] = '#';
				out_answer[ct++] = 'P';
				out_answer[ct++] = 0; //lunghezza
				out_answer[ct++] = (u8)subcommand;

				out_answer[ct++] = 0; //is unicode

				memset(&out_answer[ct], ' ', 32);
				{
					u8 i = ct;
					out_answer[i++] = 'V'; out_answer[i++] = 'E'; out_answer[i++] = 'R';
					out_answer[i++] = ' '; 
					out_answer[i++] = 'A'; out_answer[i++] = 'N'; out_answer[i++] = 'D';
					out_answer[i++] = ' '; 
					out_answer[i++] = 'M'; out_answer[i++] = 'O'; out_answer[i++] = 'D'; out_answer[i++] = 'E'; out_answer[i++] = 'L';
				}
				ct += 32;

				out_answer[2] = (u8)ct + 1;
				out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
				*in_out_sizeOfAnswer = out_answer[2];
				return true;


			} //switch (subcommand)
		}
		break;
	}
	return false;
}

//*****************************************************************
void CPUChannelFakeCPU::priv_buildAnswerTo_checkStatus_B(u8 *out_answer, u16 *in_out_sizeOfAnswer)
{
	//gestione fake del cleaning
	if (cleaning.cleaningType != eCPUProgrammingCommand_cleaningType_invalid)
	{
		if (cleaning.cleaningType == eCPUProgrammingCommand_cleaningType_sanitario)
		{
			if (rhea::getTimeNowMSec() >= cleaning.timeToEnd)
			{
				cleaning.timeToEnd += 3000;
				cleaning.btn1 = cleaning.btn2 = 0;
				++cleaning.fase;

				if (cleaning.fase == 3)
				{
					cleaning.btn1 = 10;
					cleaning.timeToEnd += 7000;
				}

				if (cleaning.fase >= 6)
				{
					cleaning.cleaningType = eCPUProgrammingCommand_cleaningType_invalid;
					this->VMCState = cleaning.prevState;
				}
			}
		}
		else
		{
			if (rhea::getTimeNowMSec() >= cleaning.timeToEnd)
			{
				cleaning.cleaningType = eCPUProgrammingCommand_cleaningType_invalid;
				this->VMCState = cleaning.prevState;
			}
		}
	}

	//gestione fake del "test selezione"
	if (VMCState == eVMCState_TEST_ATTUATORE_SELEZIONE)
	{
		if (rhea::getTimeNowMSec() > timeToEndTestSelezioneMSec)
		{
			timeToEndTestSelezioneMSec = 0;
			VMCState = eVMCState_DISPONIBILE;
		}
	}
	

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
	77		a 1 significa che NON è disponibile
	78
	79
	80
	*/
	const u8 selAvailability[6][8] = { 
		{ 0, 0, 0, 0, 0, 0,	0, 0 },	//01-08
		{ 0, 0, 0, 0, 0, 0,	0, 0 },	//09-16
		{ 0, 0, 0, 0, 0, 0,	0, 0 },	//17-24
		{ 0, 0, 0, 0, 0, 0,	0, 0 },	//25-32
		{ 0, 0, 0, 0, 0, 0,	0, 0 },	//33-40
		{ 0, 0, 0, 0, 0, 0,	0, 0 }	//41-48
	};

	for (u8 i2 = 0; i2 < 6; i2++)
	{
		u8 b = 0;
		for (u8 i3 = 0; i3 < 8; i3++)
		{
			if (selAvailability[i2][i3] != 0)
				b |= (0x01 << i3);
		}
		out_answer[ct++] = b;
	}

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
	out_answer[ct++] = curCPUMessageImportanceLevel;

	//87-94		8 byte stringa con l'attuale credito
	out_answer[ct++] = '0';
	out_answer[ct++] = '.';
	out_answer[ct++] = '0';
	out_answer[ct++] = '0';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';
	out_answer[ct++] = ' ';


	//protocl version 4
	//1 byte per indicare se btn prog è cliccato
	out_answer[ct++] = 0;

	//protocol version 5
	//1 byte a mo' di 8 bit flag per usi futuri
	out_answer[ct] = 0;
	out_answer[ct] |= 0x01; //indica che CPU è pronta per fornire il data-audit
	ct++;


	//116 ck
	out_answer[2] = (u8)ct+1;
	out_answer[ct] = rhea::utils::simpleChecksum8_calc(out_answer, ct);
	ct++;
	(*in_out_sizeOfAnswer) = (u16)ct;
}
