#ifndef _stres_h_INCLUSO_
#define _stres_h_INCLUSO_
#include "../rheaAppLib/rheaApp.h"
#include "../rheaAppLib/rheaAppUtils.h"
#include "../rheaAppLib/rheaAppFileTransfer.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaCommonLib/Protocol/ProtocolChSocketTCP.h"
#include "../rheaCommonLib/Protocol/ProtocolConsole.h"


//****************************************************************************
class Stress
{
public:
	Stress();
	~Stress();


	void	setup (HThreadMsgR &msgQHandleR, rhea::ProtocolConsole *proto, rhea::ProtocolChSocketTCP *ch, rhea::LinearBuffer *bufferR, OSWaitableGrp *waitGrp, rhea::ISimpleLogger *logger);

	bool	uploadDA3 (const char *filename);
	u16		downloadDA3();

private:
	bool	priv_onMsgRcv (rhea::app::sDecodedMsg &decoded);

private:
	HThreadMsgR					msgQHandleR;
	rhea::ProtocolConsole		*proto;
	rhea::ProtocolChSocketTCP	*ch;
	rhea::LinearBuffer			*bufferR;
	OSWaitableGrp				*waitGrp;
	rhea::ISimpleLogger			*logger;

};

#endif