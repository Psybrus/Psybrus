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
DsCoreImpl::DsCoreImpl() /*:
pContext_(NULL)/**/
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

	int memory_size;
	WebbyServerConfig config;

#if defined(_WIN32)
	{
		WORD wsa_version = MAKEWORD(2, 2);
		WSADATA wsa_data;
		if (0 != WSAStartup(wsa_version, &wsa_data))
		{
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
		fprintf(stderr, "failed to init server\n");
		
	}


	//pContext_ = mg_start(&DsCoreImpl::MongooseCallback, NULL, Options);
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCoreImpl::update()
{
    WebbyServerUpdate(Server_);

}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCoreImpl::close()
{
	//mg_stop( pContext_ );
	//pContext_ = NULL;
}


//////////////////////////////////////////////////////////////////////////
// gameThreadMongooseCallback
void DsCoreImpl::gameThreadMongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
/*	const struct mg_request_info* pRequestInfo = mg_get_request_info( pConn );
	std::string type = "html";
	BcHtml HtmlContent;
	BcU8* Output;
	int OutLength = 0;
	if (BcStrStr(pRequestInfo->uri, "/files/"))
	{
		Output = writeFile(&pRequestInfo->uri[7], OutLength, type);
	}
	else
	{
		HtmlContent.getRootNode().createChildNode("title").setContents(GPsySetupParams.Name_);
		BcHtmlNode node = HtmlContent.getRootNode();
		BcHtmlNode link = node.createChildNode("link");
		link.setAttribute("rel", "stylesheet");
		link.setAttribute("type", "text/css");
		link.setAttribute("href", "/files/style.css");
		int t = sizeof(BcHtmlNode);
		BcHtmlNode redirect = node.createChildNode("meta");
		BcHtmlNode body = node.createChildNode("body").createChildNode("div").setAttribute("id", "mainBody");
		//redirect = node["meta"];
		writeHeader(body);
		BcHtmlNode innerBody = body.createChildNode("div").setAttribute("id", "innerBody");
		//std::map<std::string, std::string> data;
		std::vector<std::string> data;
		bool success = false;
		std::string uri = &pRequestInfo->uri[1];

		for (auto Item : ButtonFunctions_)
		{
			if (uri == ("Functions/" + Item.DisplayText_))
			{
				
				redirect.setAttribute("http-equiv", "refresh");
				redirect.setAttribute("content", "0; url=/Menu");
				Item.Function_();
				success = true;
			} 
		}

		if (!success)
		{
			for (BcU32 Idx = PageFunctions_.size() - 1; Idx >= 0; --Idx)
			{
				BcRegexMatch match;
				BcU32 res = PageFunctions_[Idx].Regex_.match(&pRequestInfo->uri[1], match);
				if (res > 0)
				{
					for (BcU32 Idx2 = 1; Idx2 < match.noofMatches(); ++Idx2)
					{
						std::string u;
						match.getMatch(Idx2, u);
						data.push_back(u);
					}
					PageFunctions_[Idx].Function_(data, innerBody);
					break;
				}
			}
		}

		writeFooter(body);

		std::string Content = HtmlContent.getHtml();
		OutLength = Content.size();
		Output = new BcU8[OutLength + 1];
		BcMemCopy(Output, &Content[0], OutLength);
	}
	BcChar Buffer[ 1024 ];

	BcSPrintf( Buffer,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/%s\r\n"
			"Content-Length: %d\r\n"        // Always set Content-Length
			"\r\n",
			type.c_str(), OutLength );

	BcU32 ContentHeaderLength = BcStrLength( Buffer );
	BcU32 TotalBufferLength = ContentHeaderLength + OutLength + 1;
	BcChar* pSendBuffer = new BcChar[ TotalBufferLength ];
	BcStrCopy( pSendBuffer, Buffer );

	BcMemCopy(pSendBuffer + ContentHeaderLength, Output, OutLength);
	mg_write( pConn, pSendBuffer, TotalBufferLength );
	delete [] Output;
	delete [] pSendBuffer;
	pSendBuffer = NULL;

	GameThreadWaitFence_.decrement();/**/
}

//////////////////////////////////////////////////////////////////////////
// mongooseCallback
void* DsCoreImpl::mongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
/*	if( Event == MG_NEW_REQUEST )
	{
		BcScopedLock< BcMutex > Lock( Lock_ );

		// Dispatch this event to the game thread.
		GameThreadWaitFence_.increment();
		BcDelegate< void(*)(enum mg_event, struct mg_connection*) > Delegate(BcDelegate< void(*)(enum mg_event, struct mg_connection*) >::bind< DsCoreImpl, &DsCoreImpl::gameThreadMongooseCallback >(this));
		SysKernel::pImpl()->enqueueCallback( Delegate, Event, pConn );
		GameThreadWaitFence_.wait();
	
		return "";
	}/**/
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// MongooseCallback
//static
void* DsCoreImpl::MongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->mongooseCallback(Event, pConn);
}




int DsCoreImpl::webbyDispatch(WebbyConnection *connection)
{
	int size = 0;
	char* file = handleFile(connection->request.uri, size);
	WebbyBeginResponse(connection, 200, size, NULL, 0);
	WebbyWrite(connection, file, size);
	WebbyEndResponse(connection);
	return 0;
	/*if (0 == strcmp("/foo", connection->request.uri))
	{
		WebbyBeginResponse(connection, 200, 14, NULL, 0);
		WebbyWrite(connection, "Hello, world!\n", 14);
		WebbyEndResponse(connection);
		return 0;
	}
	else if (0 == strcmp("/bar", connection->request.uri))
	{
		WebbyBeginResponse(connection, 200, -1, NULL, 0);
		WebbyWrite(connection, "Hello, world!\n", 14);
		WebbyWrite(connection, "Hello, world?\n", 14);
		WebbyEndResponse(connection);
		return 0;
	}
	else
		return 1;/**/
}

int DsCoreImpl::webbyConnect(struct WebbyConnection *connection)
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
	int i = 0;

	printf("WebSocket frame incoming\n");
	printf("  Frame OpCode: %d\n", frame->opcode);
	printf("  Final frame?: %s\n", (frame->flags & WEBBY_WSF_FIN) ? "yes" : "no");
	printf("  Masked?     : %s\n", (frame->flags & WEBBY_WSF_MASKED) ? "yes" : "no");
	printf("  Data Length : %d\n", (int)frame->payload_length);

	while (i < frame->payload_length)
	{
		unsigned char buffer[16];
		int remain = frame->payload_length - i;
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

