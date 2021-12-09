#include "rhFSProtocol.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace rhFSx;

//********************************************************************
u16	proto::encodeMsg (u16 what, u16 userValue, const void *payload, u32 sizeOfPayload, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return encodeMsg (what, userValue, payload, sizeOfPayload, NULL, 0, out_buffer, sizeOfOutBuffer);
}
u16	proto::encodeMsg (u16 what, u16 userValue, const void *payload1, u32 sizeOfPayload1, const void *payload2, u32 sizeOfPayload2, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 byteNeeded = 4 + sizeOfPayload1 + sizeOfPayload2;
	if (sizeOfOutBuffer < byteNeeded)
	{
		DBGBREAK;
		return 0;
	}

	u32 ct = 0;
	
	rhea::utils::bufferWriteU16 (&out_buffer[ct], static_cast<u16>(what));
	ct+=2;

	rhea::utils::bufferWriteU16 (&out_buffer[ct], userValue);
	ct+=2;

	if (NULL != payload1 && sizeOfPayload1 > 0)
	{
		memcpy (&out_buffer[ct], payload1, sizeOfPayload1);
		ct += sizeOfPayload1;
	}

	if (NULL != payload2 && sizeOfPayload2 > 0)
	{
		memcpy (&out_buffer[ct], payload2, sizeOfPayload2);
		ct += sizeOfPayload2;
	}

	return static_cast<u16>(ct);
}

/********************************************************************
bool proto::decodeMsg (const rhea::ProtocolBuffer &bufferR, sDecodedMsg *out)
{
	return decodeMsg (bufferR._getPointer(0), bufferR.getCursor(), out);
}
*/
//********************************************************************
bool proto::decodeMsg (const u8 *msgBuffer, u32 msgBufferSize, sDecodedMsg *out)
{
	assert (out != NULL);
	if (msgBufferSize < 4)
	{
		DBGBREAK;
		return false;
	}

	out->what = rhea::utils::bufferReadU16(msgBuffer);
	out->userValue = rhea::utils::bufferReadU16(&msgBuffer[2]);
	if (msgBufferSize > 4)
	{
		out->payload = &msgBuffer[4];
		out->payloadLen = msgBufferSize - 4;
	}
	else
	{
		out->payload = NULL;
		out->payloadLen = 0;
	}
	return true;
}
