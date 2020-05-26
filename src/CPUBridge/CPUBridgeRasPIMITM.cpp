#include "CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"

/***************************************************
 * ritorna 0 se out_buffer non � abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 *
 *  normale messaggio cpu-gpu   [#] [W] [len] [...] [ck]
 *  messaggio rasPI             [#] [W] [len1] [len2] [command] [...] [ck]
 */
u16 cpubridge::buildMsg_rasPI_MITM (u8 command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    //calcolo della dimensione totale
    if (sizeOfOutBuffer < (u32)(6 + sizeOfOptionaData))
    {
        DBGBREAK;
        return 0;
    }

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'W';
    out_buffer[ct++] = 0;	//len1
    out_buffer[ct++] = 0;	//len2
    out_buffer[ct++] = command;

    if (optionalData && sizeOfOptionaData)
    {
        memcpy(&out_buffer[ct], optionalData, sizeOfOptionaData);
        ct += sizeOfOptionaData;
    }

    rhea::utils::bufferWriteU32 (&out_buffer[2], ct+1);	//length
    out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
    ct++;

    assert(ct <= 0xffff);
    return (u16)ct;
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_areYouThere (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (RASPI_MITM_COMMANDW_ARE_YOU_THERE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_startSocketBridge (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (RASPI_MITM_COMMANDW_START_SOCKETBRIDGE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_sendAndDoNotWait (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_buffer, u8 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (RASPI_MITM_COMMANDW_SEND_AND_DO_NOT_WAIT, bufferToSend, nBytesToSend, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_serializedSMsg (const rhea::thread::sMsg &msg, u8 *out_buffer, u8 sizeOfOutBuffer)
{
    const u32 nBytesForSerializedMsg = rhea::thread::calcSizeNeededToSerializeMsg(msg);

   //calcolo della dimensione totale
    if (sizeOfOutBuffer < (u32)(6 + nBytesForSerializedMsg))
    {
        DBGBREAK;
        return 0;
    }

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'W';
    out_buffer[ct++] = 0;	//len1
    out_buffer[ct++] = 0;	//len2
    out_buffer[ct++] = RASPI_MITM_COMMANDW_SERIALIZED_sMSG;

    ct += rhea::thread::serializeMsg (msg, &out_buffer[ct], sizeOfOutBuffer - ct);

    rhea::utils::bufferWriteU32 (&out_buffer[2], ct+1);	//length
    out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
    ct++;

    assert(ct <= 0xffff);
    return (u16)ct;
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_waitSpecificChar (u8 expectedChar, u64 timeoutMSec, u8 *out_buffer, u8 sizeOfOutBuffer)
{
    assert (timeoutMSec < u32MAX);
    u8 optionalData[8];
    rhea::utils::bufferWriteU32(optionalData, (u32)timeoutMSec);
    optionalData[4] = expectedChar;

    return cpubridge::buildMsg_rasPI_MITM (RASPI_MITM_COMMANDW_MITM_WAIT_SPECIFIC_CHAR, optionalData, 5, out_buffer, sizeOfOutBuffer);
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



