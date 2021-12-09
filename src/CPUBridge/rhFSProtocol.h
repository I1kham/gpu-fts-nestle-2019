#ifndef _rhFSProtocol_h_
#define _rhFSProtocol_h_
#include "rhFSProtocolEnumAndDefine.h"

namespace rhFSx
{
	namespace proto
	{
		u16			encodeMsg (u16 what, u16 userValue, const void *payload, u32 sizeOfPayload, u8 *out_buffer, u32 sizeOfOutBuffer);
		u16			encodeMsg (u16 what, u16 userValue, const void *payload1, u32 sizeOfPayload1, const void *payload2, u32 sizeOfPayload2, u8 *out_buffer, u32 sizeOfOutBuffer);
		bool		decodeMsg (const u8 *msgBuffer, u32 msgBufferSize, sDecodedMsg *out);

	} //namespace proto
} //namespace rhFSx

#endif //_rhFSProtocol_h_