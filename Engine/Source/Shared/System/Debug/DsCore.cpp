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

#include "System/Debug/DsCore.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( DsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCore::DsCore():
	pContext_( NULL )
{
	registerFunction( "", &cmdMenu);
	registerFunction("Content", &cmdContent, "Content");
	registerFunction("Scene", &cmdScene, "Scene");
	registerFunction("Resource/(?<Id>.*)", &cmdResource);
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCore::~DsCore()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void DsCore::open()
{
	// Configuration.
	const char* Options[] = {
		"document_root", "./",
		"listening_ports", "1337",
		NULL
	};

	pContext_ = mg_start( &DsCore::MongooseCallback, NULL, Options );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void DsCore::update()
{

}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void DsCore::close()
{
	mg_stop( pContext_ );
	pContext_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// cmdContent
void DsCore::cmdContent(DsParameters params, std::string& Output)
{
	Output += "<ul>";

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		CsResourceRef<> Resource( CsCore::pImpl()->getResource( Idx ) );

		Output += "<li>Resource: ";
		Output += *Resource->getName();
		Output += "</li>";

		Output += "<ul>";

		Output += "<li>Type:";
		Output += *Resource->getTypeName();
		Output += "</li>";

		Output += "<li>Package:";
		Output += *Resource->getPackageName();
		Output += "</li>";

		Output += "</ul>";
	}

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// registerFunction
void DsCore::registerFunction(std::string regex, std::function < void(DsParameters, std::string&)> fn, std::string display)
{
	DsCoreMessage cm(regex, display);
	cm.Function_ = fn;
	MessageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// registerFunction
void DsCore::registerFunction(std::string regex, std::function < void(DsParameters, std::string&)> fn)
{
	DsCoreMessage cm(regex);
	cm.Function_ = fn;
	MessageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// deregisterFunction
void DsCore::deregisterFunction(std::string regex)
{
	for (auto iter = MessageFunctions_.begin(); iter != MessageFunctions_.end(); ++iter)
	{
		if ((*iter).Text_.compare(regex.c_str())){
			MessageFunctions_.erase(iter);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// cmdScene
void DsCore::cmdScene(DsParameters params, std::string& Output)
{
	BcU32 Idx = 0;
	while( ScnEntityRef Entity = ScnCore::pImpl()->getEntity( Idx++ ) )
	{
		if( Entity->getParentEntity() == NULL )
		{
			cmdScene_Entity( Entity, Output, 0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdMenu
void DsCore::cmdMenu(DsParameters params, std::string& Output)
{

	Output += "<ul>";
	DsCore* core = pImpl();
	for (BcU32 Idx = 0; Idx < core->MessageFunctions_.size(); ++Idx)
	{
		if (core->MessageFunctions_[Idx].Visible_)
		{
			Output += "<li>";
			Output += "<a href=\"/";
			Output += core->MessageFunctions_[Idx].Text_;
			Output += "\">";
			Output += core->MessageFunctions_[Idx].Display_;
			Output += "</a>";
			Output += "</li>";
		}
	}

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Entity
void DsCore::cmdScene_Entity( ScnEntityRef Entity, std::string& Output, BcU32 Depth)
{
	Output += "<ul>";
	BcChar Id[32];
	BcSPrintf(Id, "%d", Entity->getUniqueId());
	
	// Entity name.
	Output += "<li>";
	Output += "Entity: ";
	Output += "<a href=\"/Resource/";
	Output += Id;
	Output += "\">";
	Output += *Entity->getName();
	Output += "</a>";
	Output += "</li>";
	for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Entity->getComponent( Idx ) );
	
		if( Component->isTypeOf< ScnEntity >() )
		{
			cmdScene_Entity( ScnEntityRef( Component ), Output, Depth + 1);
		}
		else
		{
			cmdScene_Component( Component, Output, Depth + 1 );
		}
	}

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Component
void DsCore::cmdScene_Component( ScnComponentRef Component, std::string& Output, BcU32 Depth )
{
	BcChar Id[32];
	BcSPrintf(Id, "%d", Component->getUniqueId());
	Output += "<ul>";

	// Component name.
	Output += "<li>";
	Output += "Component: ";
	Output += "<a href=\"/Resource/";
	Output += Id;
	Output += "\">";
	Output += *Component->getName();
	Output += "</a>";
	Output += " (";
	Output += *Component->getTypeName();
	Output += ")";
	Output += "</li>";

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// gameThreadMongooseCallback
void DsCore::gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn )
{
	const struct mg_request_info* pRequestInfo = mg_get_request_info( pConn );
	std::string type = "html";
	std::string Content;
	BcU8* Output;
	int OutLength = 0;
	if (BcStrStr(pRequestInfo->uri, "/files/"))
	{
		Output = writeFile(&pRequestInfo->uri[7], OutLength, type);
	}
	else
	{
		writeHeader(Content);
		//std::map<std::string, std::string> data;
		std::vector<std::string> data;
		for (BcU32 Idx = MessageFunctions_.size() - 1; Idx >= 0; --Idx)
		{
			BcRegexMatch match;
			BcU32 res = MessageFunctions_[Idx].Regex_.match(&pRequestInfo->uri[1], match);
			if (res > 0)
			{
				for (BcU32 Idx2 = 1; Idx2 < match.noofMatches(); ++Idx2)
				{
					std::string u;
					match.getMatch(Idx2, u);
					data.push_back(u);
				}
				MessageFunctions_[Idx].Function_(data, Content);
				break;
			}
		}

		writeFooter(Content);
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
void* DsCore::mongooseCallback( enum mg_event Event, struct mg_connection* pConn )
{
	if( Event == MG_NEW_REQUEST )
	{
		BcScopedLock< BcMutex > Lock( Lock_ );

		// Dispatch this event to the game thread.
		GameThreadWaitFence_.increment();
		BcDelegate< void (*)( enum mg_event, struct mg_connection* ) > Delegate( BcDelegate< void (*)( enum mg_event, struct mg_connection* ) >::bind< DsCore, &DsCore::gameThreadMongooseCallback >( this ) );
		SysKernel::pImpl()->enqueueCallback( Delegate, Event, pConn );
		GameThreadWaitFence_.wait();
	
		return "";
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// MongooseCallback
//static
void* DsCore::MongooseCallback( enum mg_event Event, struct mg_connection* pConn )
{
	return DsCore::pImpl()->mongooseCallback( Event, pConn );
}

//////////////////////////////////////////////////////////////////////////
// writeHeader
void DsCore::writeHeader(std::string& Output)
{
	Output += "<!DOCTYPE html>";
	Output += "<html>";
	Output += "<link rel =\"stylesheet\" type=\"text/css\" href=\"/files/style.css\">";
	Output += "<body>";
	Output += "<h1>";
	Output += GPsySetupParams.Name_;
	Output += "</h1>";

	Output += "<a href=\"Menu\">Menu</a><br />";
}

//////////////////////////////////////////////////////////////////////////
// writeFooter
void DsCore::writeFooter(std::string& Output)
{
	Output += "Footer";

	Output += "</body></html>";
}

//////////////////////////////////////////////////////////////////////////
// Gets a file for the output stream
BcU8* DsCore::writeFile(std::string filename, int& OutLength, std::string& type)
{
	BcFile file;
	std::string f = "Content/Debug/";
	f += filename;
	file.open(f.c_str());
	if (!file.isOpen())
		return 0;
	BcU8* data;// = new BcU8[file.size()];
	data = file.readAllBytes();
	OutLength = file.size();
	type = "css";
	// TODO: Actually load files
	return data;
}

//////////////////////////////////////////////////////////////////////////
// cmdResource
void DsCore::cmdResource(DsParameters params, std::string& Output)
{
	std::string EntityId = "";
	/*auto par = params.find("id");
	if (par != params.end())
		EntityId = (*par).second;/**/
	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output += "Invalid resource Id <br/>";
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());

	CsResourceRef<> Resource(CsCore::pImpl()->getResourceByUniqueId(id));

	if (Resource == NULL)
	{
		Output += "Invalid resource Id <br/>";
		return;
	}

	if (Resource->getClass() == ScnEntity::StaticGetClass())
	{
		cmdScene_Entity(ScnEntityRef(Resource), Output, 0);
	}
	else
	{
		Output += "<ul>";

		Output += "<li>Resource: ";
		Output += *Resource->getName();
		Output += "</li>";

		Output += "<ul>";


		Output += "<li>Type:";
		Output += *Resource->getTypeName();
		Output += "</li>";

		Output += "<li>Package:";
		Output += *Resource->getPackageName();
		Output += "</li>";

		Output += "</ul>";

		Output += "</ul>";
	}
}
