#ifdef WIN32
#include <conio.h>
#endif
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaUTF8.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaAlipayChina/AlypayChina.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"



//*****************************************************
void waitKB()
{
	_getch();
}

/*****************************************************
 * Fa il peek() dalla socket e cerca di recuperare l'header della risposta HTTP
 * Ritorna il num di byte letti e messi in [out_buffer].
 * Se ritorna >0, vuol dire che in [out_buffer] c'è un valido header http di risposta
 */
u32 httpExtractResponseHeader (OSSocket &sok, u32 timeoutMSec, u8 *out_buffer, u32 sizeofOutBuffer)
{
	//peek socket per vedere se c'è una risposta in coda
	i32 nRead = rhea::socket::read (sok, out_buffer, sizeofOutBuffer, timeoutMSec, true);
	if (nRead <= 0)
		return 0;

	rhea::utf8::parser::Source iter;
	iter.setup (out_buffer);
	if (!rhea::utf8::parser::find_NoCaseSens (iter, (const u8*)"\r\n\r\n"))
	{
		printf ("httpParseResponse() => \r\n\r\n not found");
		return -1;
	}

	iter.advance(4);
	const u32 headerSize = iter.iNow ;

	//ok, ho trovato un valido header http, lo tolgo dalla socket
	rhea::socket::read (sok, out_buffer, headerSize, timeoutMSec);
	out_buffer[headerSize] = 0;

	return headerSize;
}

//*****************************************************
i32 httpExtractHeaderValueAsIntOrDefault (const u8 *header, const char *fieldName, i32 defaultValue)
{
	rhea::utf8::parser::Source iter;
	iter.setup (header);
	if (!rhea::utf8::parser::find_NoCaseSens (iter, (const u8*)fieldName))
		return defaultValue;

	iter.advance(strlen(fieldName));

	//ora dovrebbe essere un carattere ":"
	if (iter.getCurChar() != ':')
		return defaultValue;
	iter.advance(1);

	rhea::utf8::parser::toNextValidChar(iter);
	i32 ret = defaultValue;
	rhea::utf8::parser::extractInteger(iter, &ret);
	return ret;
}

//*****************************************************
void httpGET (OSSocket &sok, const char *sokIP, const char *url)
{
	char buf_request[1024];

	//invio la richiesta HTTP al server
	sprintf(buf_request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url, sokIP);
	const i32 nWritten = rhea::socket::write(sok, buf_request, (int)strlen(buf_request));

	//aspetto risposta
	u8 responseHeader[1024];
	const u32 sizeOfResponseHeader = httpExtractResponseHeader (sok, 1000, responseHeader, sizeof(responseHeader));
	printf ("%s\n\n", responseHeader);
	
	const i32 contentLength = httpExtractHeaderValueAsIntOrDefault (responseHeader, "Content-Length", 0);

	if (contentLength > 0)
	{
		rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
		u8 *body = (u8*)RHEAALLOC(allocator, contentLength + 4);

		u32 nTotRead = 0;
		while (nTotRead < contentLength)
		{
			const i32 nRead = rhea::socket::read (sok, &body[nTotRead], contentLength - nTotRead, 2000);
			if (nRead > 0)
				nTotRead += (u32)nRead;
		}

		body[nTotRead] = 0;
		printf ("%s\n\n", body);
		
		RHEAFREE(allocator, body);
	}

}

//*****************************************************
void testMD5()
{
	const char machineName[] = {"C20190001"};
	const char command[] = {"E11"};
	const char timestamp[] = {"20191029223010"};
	const char apiVersion[] = {"V1.6"};
	const char key[] = {"1648339973B547DC8DE3D60787079B3D"};

	char hashedKey[64];
	char s[512];
	sprintf_s (s, sizeof(s), "%s|%s|%s|%s%s", machineName, command, timestamp, apiVersion, key);
	rhea::utils::md5 (hashedKey, sizeof(hashedKey), s, (u32)strlen(s));


	sprintf_s (s, sizeof(s), "%s|%s|%s|%s|%s#", machineName, command, timestamp, apiVersion, hashedKey);
	printf (s);
}


//*****************************************************
void testAlipayChina()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD; 
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif


	rhea::AlipayChina server;
	server.useLogger (logger);

	HThreadMsgW hMsgQWrite;
	if (server.setup("121.196.20.39", 6019, "C20190001", "1648339973B547DC8DE3D60787079B3D", &hMsgQWrite))
	{
		server.run();
		server.close();
	}
}

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
	rhea::init("testHTTP", &hInst);
#else
	rhea::init("testHTTP", NULL);
#endif
	
	//testMD5(); waitKB(); 

	testAlipayChina();

	/*apertura socket
	const char serverIP[] = { "127.0.0.1" };
	OSSocket sok;
	{
		eSocketError err = rhea::socket::openAsTCPClient(&sok, serverIP, 80);
		if (err != eSocketError_none)
		{
			printf ("errore in apertura socket [%d]\n", (u32)err);
			waitKB();
			return 0;
		}
	}

	//httpGET (sok, serverIP, "/varie/rheaRESTtest/index.php");
	httpGET (sok, serverIP, "/varie/rheaRESTtest/test.php?op=echo&what=pippo%20fa%20la%20pizza");
	rhea::socket::close (sok);
	waitKB();
	*/

    rhea::deinit();
	return 0;
}


