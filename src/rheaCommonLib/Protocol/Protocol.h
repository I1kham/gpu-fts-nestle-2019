#ifndef _rheaProtocol_h_
#define _rheaProtocol_h_
#include "../rhea.h"


namespace rhea
{
	namespace protocol
	{
		static const u16	RES_CHANNEL_CLOSED = u16MAX;
		static const u16	RES_PROTOCOL_CLOSED = u16MAX - 1;
		static const u16	RES_PROTOCOL_WRITEBUFFER_TOOSMALL = u16MAX - 2;

		static const u16	RES_ERROR = (u16MAX - 10);
	} //namespace protocol
} //namespace rhea

#endif // _rheaIProtocol_h_

