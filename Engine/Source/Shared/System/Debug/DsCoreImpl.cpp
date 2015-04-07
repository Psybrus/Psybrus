/**************************************************************************
*
* File:		DsCore.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug core.
*		
*
*
* 
**************************************************************************/

#include "System/Debug/DsCoreImpl.h"
#include "Base/BcHtml.h"
#include "System/SysKernel.h"
#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR(DsCoreImpl);

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCoreImpl::DsCoreImpl()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCoreImpl::~DsCoreImpl()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void DsCoreImpl::open()
{
#if USE_WEBBY
	int memory_size;
	WebbyServerConfig config;

#if defined(_WIN32)
	{
		WORD wsa_version = MAKEWORD(2, 2);
		WSADATA wsa_data;
		if (0 != WSAStartup(wsa_version, &wsa_data))
		{
			BcLog* log = BcLog::pImpl();
			log->write(0, "WSAStartup failed");
			fprintf(stderr, "WSAStartup failed\n");
			return;
		}
	}
#endif

	memset(&config, 0, sizeof config);
	config.bind_address = "127.0.0.1";
	config.listening_port = 1337;
	config.flags = WEBBY_SERVER_WEBSOCKETS;
	config.connection_max = 4;
	config.request_buffer_size = 2048;
	config.io_buffer_size = 8192;
	config.dispatch = &DsCoreImpl::externalWebbyDispatch;
	//config.log = &DsCoreImpl::externalWebbyLog;
	config.ws_connect = &DsCoreImpl::externalWebbyConnect;
	config.ws_connected = &DsCoreImpl::externalWebbyConnected;
	config.ws_closed = &DsCoreImpl::externalWebbyClosed;
	config.ws_frame = &DsCoreImpl::externalWebbyFrame;
	memory_size = WebbyServerMemoryNeeded(&config);
	ServerMemory_ = malloc(memory_size);
	Server_ = WebbyServerInit(&config, ServerMemory_, memory_size);

	if (!Server_)
	{
		BcLog* log = BcLog::pImpl();
		log->write(0, "Failed to initialise Webby server");
		fprintf(stderr, "failed to init server\n");
		
	}


#endif
	//pContext_ = mg_start(&DsCoreImpl::MongooseCallback, NULL, Options);
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCoreImpl::update()
{
#if USE_WEBBY
     WebbyServerUpdate(Server_);
#endif
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCoreImpl::close()
{
	//mg_stop( pContext_ );
	//pContext_ = NULL;
#if USE_WEBBY
	free(ServerMemory_);
#endif
}


#if USE_WEBBY

int DsCoreImpl::webbyDispatch(WebbyConnection *connection)
{
	int size = 0;
	char* data = new char[connection->request.content_length + 1];
	BcMemZero(data, connection->request.content_length + 1);
	WebbyRead(connection, data, connection->request.content_length);
	char* file = handleFile(connection->request.uri, size, data);
	WebbyBeginResponse(connection, 200, size, NULL, 0);
	WebbyWrite(connection, file, size);
	WebbyEndResponse(connection);

	delete data;
	return 0;
}

int DsCoreImpl::webbyConnect(WebbyConnection *connection)
{
	/* Allow websocket upgrades on /wstest */
	if (0 == strcmp(connection->request.uri, "/wstest") && ConnectionCount_ < MAX_WSCONN)
		return 0;
	else
		return 1;
}

void DsCoreImpl::webbyConnected(WebbyConnection *connection)
{
	printf("WebSocket connected\n");
	ws_connections[ConnectionCount_++] = connection;
}

void DsCoreImpl::webbyClosed(WebbyConnection *connection)
{
	int i;
	printf("WebSocket closed\n");

	for (i = 0; i < ConnectionCount_; i++)
	{
		if (ws_connections[i] == connection)
		{
			int remain = ConnectionCount_ - i;
			memmove(ws_connections + i, ws_connections + i + 1, remain * sizeof(WebbyConnection *));
			--ConnectionCount_;
			break;
		}
	}
}

int DsCoreImpl::webbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame)
{
	size_t i = 0;

	printf("WebSocket frame incoming\n");
	printf("  Frame OpCode: %d\n", frame->opcode);
	printf("  Final frame?: %s\n", (frame->flags & WEBBY_WSF_FIN) ? "yes" : "no");
	printf("  Masked?     : %s\n", (frame->flags & WEBBY_WSF_MASKED) ? "yes" : "no");
	printf("  Data Length : %d\n", (int)frame->payload_length);

	while (i < frame->payload_length)
	{
		unsigned char buffer[16];
		size_t remain = frame->payload_length - i;
		size_t read_size = remain >(int) sizeof buffer ? sizeof buffer : (size_t)remain;
		size_t k;

		printf("%08x ", (int)i);

		if (0 != WebbyRead(connection, buffer, read_size))
			break;

		for (k = 0; k < read_size; ++k)
			printf("%02x ", buffer[k]);

		for (k = read_size; k < 16; ++k)
			printf("   ");

		printf(" | ");

		for (k = 0; k < read_size; ++k)
			printf("%c", isprint(buffer[k]) ? buffer[k] : '?');

		printf("\n");

		i += read_size;
	}

	return 0;
}
int DsCoreImpl::externalWebbyDispatch(WebbyConnection *connection)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->webbyDispatch(connection);
}

int DsCoreImpl::externalWebbyConnect(WebbyConnection *connection)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->webbyConnect(connection);
}

void DsCoreImpl::externalWebbyConnected(WebbyConnection *connection)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->webbyConnected(connection);
}

void DsCoreImpl::externalWebbyClosed(WebbyConnection *connection)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->webbyClosed(connection);
}

int DsCoreImpl::externalWebbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->webbyFrame(connection, frame);
}

#endif // USE_WEBBY
