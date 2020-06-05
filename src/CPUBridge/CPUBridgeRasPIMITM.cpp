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

    rhea::utils::bufferWriteU16_LSB_MSB(&out_buffer[2], (u16)(ct+1));	//length
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
u16 cpubridge::buildMsg_rasPI_MITM_getWifiIPandSSID (u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_GET_WIFI_IPandSSID, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u16 cpubridge::buildMsg_rasPI_MITM_unzipTSGUI (const u8* const srcZipFilename, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    return cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_UNZIP_TS_GUI, srcZipFilename, rhea::string::utf8::lengthInBytes(srcZipFilename)+1, out_buffer, sizeOfOutBuffer);
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
void cpubridge::ask_CPU_RASPI_MITM_GET_WIFI_IPandSSID (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_WIFI_IPandSSID, handlerID);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_GET_WIFI_IPandSSID (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const char *ipAddress, const char *ssid)
{
    u8 ipPart[4];
    rhea::netaddr::ipstrTo4bytes (ipAddress, &ipPart[0], &ipPart[1], &ipPart[2], &ipPart[3]);
    notify_CPU_RASPI_MITM_GET_WIFI_IPandSSID (to, handlerID, logger, ipPart[0], ipPart[1], ipPart[2], ipPart[3], ssid);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_GET_WIFI_IPandSSID (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 ipPart0, u8 ipPart1, u8 ipPart2, u8 ipPart3, const char *ssid)
{
	logger->log("notify_CPU_RASPI_MITM_GET_WIFI_IPandSSID\n");

    u8 data[256];
    data[0] = ipPart0;
    data[1] = ipPart1;
    data[2] = ipPart2;
    data[3] = ipPart3;
    sprintf_s ((char*)&data[4], sizeof(data) - 4, "%s", ssid);
    const u32 len = strlen(ssid);
    data[4 + len] = 0;
    rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_GET_WIFI_IPandSSID, handlerID, data, 5+len);
}

//***************************************************
void cpubridge::translateNotify_CPU_RASPI_MITM_GET_WIFI_IPandSSID(const rhea::thread::sMsg &msg, char *out_ipAddress, u32 sizeof_outIpAddress, char *out_ssid, u32 sizeof_outssid)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_GET_WIFI_IPandSSID);
	const u8 *p = (const u8*)msg.buffer;
    sprintf_s (out_ipAddress, sizeof_outIpAddress, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

    const u32 nToCopy = msg.bufferSize - 4;
    if (sizeof_outssid >= nToCopy)
        memcpy (out_ssid, &p[4], nToCopy);
    else
    {
        DBGBREAK;
        out_ssid[0] = 0;
    }
}

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_FileUpload (const sSubscriber &from, u16 handlerID, const u8* const fullFilePathAndName)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_FILE_UPLOAD, handlerID, fullFilePathAndName, rhea::string::utf8::lengthInBytes(fullFilePathAndName) + 1);
}

//***************************************************
void cpubridge::translate_CPU_RASPI_MITM_FileUpload(const rhea::thread::sMsg &msg, u8 *out_srcFullFileNameAndPath, u32 sizeOfOut)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_FILE_UPLOAD);
	u32 n = msg.bufferSize;
	if (n > sizeOfOut)
	{
		DBGBREAK;
		n = sizeOfOut;
	}

	memcpy(out_srcFullFileNameAndPath, msg.buffer, n);
}

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_Upload_GUI_TS (const sSubscriber &from, u16 handlerID, const u8* const fullRheaZipFilePathAndName)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_GUI_TS_UPLOAD, handlerID, fullRheaZipFilePathAndName, rhea::string::utf8::lengthInBytes(fullRheaZipFilePathAndName) + 1);
}

//***************************************************
void cpubridge::translate_CPU_RASPI_MITM_Upload_GUI_TS(const rhea::thread::sMsg &msg, u8 *out_fullRheaZipFilePathAndName, u32 sizeOfOut)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_GUI_TS_UPLOAD);
	u32 n = msg.bufferSize;
	if (n > sizeOfOut)
	{
		DBGBREAK;
		n = sizeOfOut;
	}
	memcpy(out_fullRheaZipFilePathAndName, msg.buffer, n);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_Upload_GUI_TS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool bSuccess)
{
	logger->log("notify_CPU_RASPI_MITM_Upload_GUI_TS\n");

    u8 data[4];
    data[0] = 0;
    if (bSuccess)
        data[0] = 0x01;
    rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_UPLOAD_GUI_TS, handlerID, data, 1);
}

//***************************************************
void cpubridge::translateNotify_CPU_RASPI_MITM_Upload_GUI_TS(const rhea::thread::sMsg &msg, bool *out_bSuccess)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_UPLOAD_GUI_TS);
    const u8 *p = (const u8*)msg.buffer;
    *out_bSuccess = false;
    if (p[0] == 0x01)
        *out_bSuccess = true;
}
