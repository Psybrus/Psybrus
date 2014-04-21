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
DsCoreImpl::DsCoreImpl() :
	pContext_( NULL )
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
	// Configuration.
	const char* Options[] = {
		"document_root", "./",
		"listening_ports", "1337",
		NULL
	};

	pContext_ = mg_start(&DsCoreImpl::MongooseCallback, NULL, Options);
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCoreImpl::update()
{

}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCoreImpl::close()
{
	mg_stop( pContext_ );
	pContext_ = NULL;
}


//////////////////////////////////////////////////////////////////////////
// gameThreadMongooseCallback
void DsCoreImpl::gameThreadMongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
	const struct mg_request_info* pRequestInfo = mg_get_request_info( pConn );
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
		/*Output += "<title>";
		Output += GPsySetupParams.Name_;
		Output += "</title>";
		Output += "<link rel =\"stylesheet\" type=\"text/css\" href=\"/files/style.css\">";/**/
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

	GameThreadWaitFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// mongooseCallback
void* DsCoreImpl::mongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
	if( Event == MG_NEW_REQUEST )
	{
		BcScopedLock< BcMutex > Lock( Lock_ );

		// Dispatch this event to the game thread.
		GameThreadWaitFence_.increment();
		BcDelegate< void(*)(enum mg_event, struct mg_connection*) > Delegate(BcDelegate< void(*)(enum mg_event, struct mg_connection*) >::bind< DsCoreImpl, &DsCoreImpl::gameThreadMongooseCallback >(this));
		SysKernel::pImpl()->enqueueCallback( Delegate, Event, pConn );
		GameThreadWaitFence_.wait();
	
		return "";
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// MongooseCallback
//static
void* DsCoreImpl::MongooseCallback(enum mg_event Event, struct mg_connection* pConn)
{
	return static_cast<DsCoreImpl*>(DsCore::pImpl())->mongooseCallback(Event, pConn);
}
