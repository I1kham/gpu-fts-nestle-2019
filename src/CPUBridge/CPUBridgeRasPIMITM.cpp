#include "CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"

/***************************************************
 * ritorna 0 se out_buffer non è abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 */
u8 cpubridge::buildMsg_rasPI_MITM (u16 command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u8 sizeOfOutBuffer)
{
    //calcolo della dimensione totale
    if (sizeOfOutBuffer < 4 + sizeOfOptionaData)
        return 0;

    u8 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'W';
    out_buffer[ct++] = 0;	//length
    out_buffer[ct++] = 0xff;
    out_buffer[ct++] = 0xff;
    rhea::utils::bufferWriteU16 (&out_buffer[ct], command);
    ct+=2;

    if (optionalData && sizeOfOptionaData)
    {
        memcpy(&out_buffer[ct], optionalData, sizeOfOptionaData);
        ct += sizeOfOptionaData;
    }

    out_buffer[2] = (ct+1);	//length
    out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
    ct++;

    return ct;
}

//***************************************************
u8 cpubridge::buildMsg_rasPI_MITM_AreYouThere (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_ARE_YOU_THERE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_rasPI_MITM_START_SOCKETBRIDGE (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_START_SOCKETBRIDGE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_ARE_YOU_THERE (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_ARE_YOU_THERE, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_ARE_YOU_THERE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 version, u8 futureUse1, u8 futureUse2, u8 futureUse3)
{
	logger->log("notify_CPU_RASPI_MITM_ARE_YOU_THERE\n");

    u8 state[4];
    state[0] = version;
    state[1] = futureUse1;
    state[2] = futureUse2;
    state[3] = futureUse3;
    rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_ARE_YOU_THERE, handlerID, state, 4);
}

//***************************************************
void cpubridge::translateNotify_CPU_RASPI_MITM_ARE_YOU_THERE(const rhea::thread::sMsg &msg, u8 *out_version, u8 *out_futureUse1, u8 *out_futureUse2, u8 *out_futureUse3)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_ARE_YOU_THERE);
	const u16 *p = (const u16*)msg.buffer;
    *out_version = p[0];
    *out_futureUse1 = p[1];
    *out_futureUse2 = p[2];
    *out_futureUse3 = p[3];
}

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_START_SOCKETBRIDGE (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_START_SOCKETBRIDGE, (u32)0);
}



