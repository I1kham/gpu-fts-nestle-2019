#include "CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"

/***************************************************
 * ritorna 0 se out_buffer non è abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 *
 *  normale messaggio cpu-gpu   [#] [W] [len] [...] [ck]
 *  messaggio rasPI             [#] [W] [len_LSB] [len_MSB] [command] [...] [ck]
 */
u16 cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u32 sizeOfOutBuffer)
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
    out_buffer[ct++] = (u8)command;

    if (optionalData && sizeOfOptionaData)
    {
        memcpy(&out_buffer[ct], optionalData, sizeOfOptionaData);
        ct += sizeOfOptionaData;
    }

    rhea::utils::bufferWriteU16_LSB_MSB(&out_buffer[2], ct+1);	//length
    out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
    ct++;

    assert(ct <= 0xffff);
    return (u16)ct;
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_areYouThere (u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_ARE_YOU_THERE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_startSocketBridge (u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_START_SOCKETBRIDGE, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_sendAndDoNotWait (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_SEND_AND_DO_NOT_WAIT, bufferToSend, nBytesToSend, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_getWifiIP (u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_GET_WIFI_IP, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_unzipTouchscreenGUI (const u8* const srcFilename, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_UNZIP_TS_GUI, srcFilename, rhea::string::utf8::lengthInBytes(srcFilename)+1, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_serializedSMsg (const rhea::thread::sMsg &msg, u8 *out_buffer, u32 sizeOfOutBuffer)
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
    out_buffer[ct++] = (u8)eRasPISubcommand_SERIALIZED_sMSG;

    ct += rhea::thread::serializeMsg (msg, &out_buffer[ct], sizeOfOutBuffer - ct);

    rhea::utils::bufferWriteU16_LSB_MSB (&out_buffer[2], ct+1);	//length
    out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
    ct++;

    assert(ct <= 0xffff);
    return (u16)ct;
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_waitSpecificChar (u8 expectedChar, u64 timeoutMSec, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    assert (timeoutMSec < u32MAX);
    u8 optionalData[8];
    rhea::utils::bufferWriteU32(optionalData, (u32)timeoutMSec);
    optionalData[4] = expectedChar;

    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_WAIT_SPECIFIC_CHAR, optionalData, 5, out_buffer, sizeOfOutBuffer);
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

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_GET_WIFI_IP (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_WIFI_IP, handlerID);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_GET_WIFI_IP (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const char *ipAddress)
{
    u8 ipPart[4];
    rhea::netaddr::ipstrTo4bytes (ipAddress, &ipPart[0], &ipPart[1], &ipPart[2], &ipPart[3]);
    notify_CPU_RASPI_MITM_GET_WIFI_IP (to, handlerID, logger, ipPart[0], ipPart[1], ipPart[2], ipPart[3]);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_GET_WIFI_IP (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 ipPart0, u8 ipPart1, u8 ipPart2, u8 ipPart3)
{
	logger->log("notify_CPU_RASPI_MITM_GET_WIFI_IP\n");

    const u8 state[4] = { ipPart0, ipPart1, ipPart2, ipPart3 };
    rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_GET_WIFI_IP, handlerID, state, 4);
}

//***************************************************
void cpubridge::translateNotify_CPU_RASPI_MITM_GET_WIFI_IP(const rhea::thread::sMsg &msg, char *out_ipAddress, u32 sizeof_outIpAddress)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_GET_WIFI_IP);
	const u8 *p = (const u8*)msg.buffer;
    sprintf_s (out_ipAddress, sizeof_outIpAddress, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}


