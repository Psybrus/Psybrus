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
void DsCore::cmdContent( std::string& Output )
{
	BcChar Buffer[ 1024 ];
	Output += "<ul>";

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		CsResourceRef<> Resource( CsCore::pImpl()->getResource( Idx ) );

		Output += "<li>Resource: ";
		Output += *Resource->getName();
		Output += "</li>";

		Output += "<ul>";

		Output += "<li>Type:";
		Output += *Resource->getType();
		Output += "</li>";

		Output += "<li>Package:";
		Output += *Resource->getPackageName();
		Output += "</li>";

		Output += "<li>Ref Count:";
		BcSPrintf( Buffer, "%u", Resource->refCount() );
		Output += Buffer;
		Output += "</li>";

		Output += "</ul>";
	}

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// cmdScene
void DsCore::cmdScene( std::string& Output )
{
	BcU32 Idx = 0;
	while( ScnEntityRef Entity = ScnCore::pImpl()->getEntity( Idx++ ) )
	{
		cmdScene_Entity( Entity, Output, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Entity
void DsCore::cmdScene_Entity( ScnEntityRef Entity, std::string& Output, BcU32 Depth )
{
	Output += "<ul>";

	// Entity name.
	Output += "<li>";
	Output += "Entity: ";
	Output += *Entity->getName();
	Output += "</li>";

	// Parent entity.
	Output += "<ul>";
	Output += "<li>";
	Output += "Parent Entity: ";
	Output += Entity->getParentEntity().isValid() ? *Entity->getParentEntity()->getName() : "NULL";
	Output += "</li>";
	Output += "</ul>";

	for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Entity->getComponent( Idx ) );
	
		if( Component->isTypeOf< ScnEntity >() )
		{
			cmdScene_Entity( ScnEntityRef( Component ), Output, Depth + 1 );
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
	Output += "<ul>";

	// Component name.
	Output += "<li>";
	Output += "Component: ";
	Output += *Component->getName();
	Output += " (";
	Output += *Component->getType();
	Output += ")";
	Output += "</li>";

	// Parent entity.
	Output += "<ul>";
	Output += "<li>";
	Output += "Parent Entity: ";
	Output += Component->getParentEntity().isValid() ? *Component->getParentEntity()->getName() : "NULL";
	Output += "</li>";
	Output += "</ul>";

	Output += "</ul>";
}

//////////////////////////////////////////////////////////////////////////
// gameThreadMongooseCallback
void DsCore::gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn )
{
	const struct mg_request_info* pRequestInfo = mg_get_request_info( pConn );

	std::string Content;
	Content += "Output for: ";
	Content += pRequestInfo->uri;
	Content += "<br/><br/>";

	if( BcStrCompare( pRequestInfo->uri, "/Content" ) )
	{
		cmdContent( Content );
	}
	else if( BcStrCompare( pRequestInfo->uri, "/Scene" ) )
	{
		cmdScene( Content );
	}
	
	mg_printf(pConn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: %d\r\n"        // Always set Content-Length
			"\r\n"
			"%s",
			Content.size(), Content.c_str());

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
