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
#include "Base/BcHtml.h"
#include "System/SysKernel.h"

#include "Psybrus.h"

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
void DsCore::cmdContent(DsParameters params, BcHtmlNode& Output)
{
	BcHtmlNode& ul = Output.createChildNode("ul");

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		CsResourceRef<> Resource( CsCore::pImpl()->getResource( Idx ) );
		BcHtmlNode& li = ul.createChildNode("li");
		li.setContents("Resource: " + *Resource->getName());

		BcHtmlNode& ul2 = ul.createChildNode("ul");
		ul2.createChildNode("li").setContents("Type: " + *Resource->getTypeName());
		ul2.createChildNode("li").setContents("Package: " + *Resource->getPackageName());
	}

}

//////////////////////////////////////////////////////////////////////////
// registerFunction
void DsCore::registerFunction(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn, std::string display)
{
	DsCoreMessage cm(regex, display);
	cm.Function_ = fn;
	MessageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// registerFunction
void DsCore::registerFunction(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn)
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
void DsCore::cmdScene(DsParameters params, BcHtmlNode& Output)
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
void DsCore::cmdMenu(DsParameters params, BcHtmlNode& Output)
{
	BcHtmlNode& ul = Output.createChildNode("ul");
	DsCore* core = pImpl();
	for (BcU32 Idx = 0; Idx < core->MessageFunctions_.size(); ++Idx)
	{
		if (core->MessageFunctions_[Idx].Visible_)
		{
			BcHtmlNode& a = ul.createChildNode("li").createChildNode("a");
			a.setAttribute("href", core->MessageFunctions_[Idx].Text_);
			a.setContents(core->MessageFunctions_[Idx].Display_);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Entity
void DsCore::cmdScene_Entity( ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth)
{
	BcHtmlNode& ul = Output.createChildNode("ul");
	BcChar Id[32];
	BcSPrintf(Id, "%d", Entity->getUniqueId());
	
	// Entity name.
	BcHtmlNode& li = ul.createChildNode("li");
	li.setContents("Entity; ");
	BcHtmlNode& a = li.createChildNode("a");
	a.setAttribute("href", "/Resource/" + std::string(Id));
	a.setContents(*Entity->getName());
	
	for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Entity->getComponent( Idx ) );
	
		if( Component->isTypeOf< ScnEntity >() )
		{
			cmdScene_Entity( ScnEntityRef( Component ), li, Depth + 1);
		}
		else
		{
			cmdScene_Component( Component, li, Depth + 1 );
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Component
void DsCore::cmdScene_Component( ScnComponentRef Component, BcHtmlNode& Output, BcU32 Depth )
{
	BcChar Id[32];
	BcSPrintf(Id, "%d", Component->getUniqueId());
	BcHtmlNode& ul = Output.createChildNode("ul");
	// Component name.
	BcHtmlNode& li = ul.createChildNode("li");
	li.setContents("Component: ");
	BcHtmlNode& a = li.createChildNode("a");
	a.setAttribute("href", "/Resource/" + std::string(Id));
	a.setContents(*Component->getName());
	li.createChildNode("").setContents(" (" + *Component->getTypeName() + ")");
}

//////////////////////////////////////////////////////////////////////////
// gameThreadMongooseCallback
void DsCore::gameThreadMongooseCallback( enum mg_event Event, struct mg_connection* pConn )
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
		HtmlContent.getRootNode().createChildNode("title");
		HtmlContent.getRootNode().createChildNode("title").setContents(GPsySetupParams.Name_);
		BcHtmlNode& link = HtmlContent.getRootNode().createChildNode("link");
		link.setAttribute("rel", "stylesheet");
		link.setAttribute("type", "text/css");
		link.setAttribute("href", "/files/style.css");
		BcHtmlNode& body = HtmlContent.getRootNode().createChildNode("body");
		writeHeader(body);
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
				MessageFunctions_[Idx].Function_(data, body);
				break;
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
void DsCore::writeHeader(BcHtmlNode& Output)
{
	/*Output += "<!DOCTYPE html>";
	Output += "<html>";
	Output += "<title>";
	Output += GPsySetupParams.Name_;
	Output += "</title>";
	Output += "<link rel =\"stylesheet\" type=\"text/css\" href=\"/files/style.css\">";
	Output += "<body>";/**/
	Output.createChildNode("h1").setContents(GPsySetupParams.Name_);
	BcHtmlNode& a = Output.createChildNode("a");
	a.setAttribute("href", "/Menu");
	a.setContents("Menu");
	Output.createChildNode("br");
}

//////////////////////////////////////////////////////////////////////////
// writeFooter
void DsCore::writeFooter(BcHtmlNode& Output)
{
	Output.createChildNode("").setContents("Footer");

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
void DsCore::cmdResource(DsParameters params, BcHtmlNode& Output)
{
	std::string EntityId = "";
	/*auto par = params.find("id");
	if (par != params.end())
		EntityId = (*par).second;/**/
	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());

	CsResourceRef<> Resource(CsCore::pImpl()->getResourceByUniqueId(id));

	if (Resource == NULL)
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}

	if (Resource->getClass() == ScnEntity::StaticGetClass())
	{
		cmdScene_Entity(ScnEntityRef(Resource), Output, 0);
	}
	else
	{
		BcHtmlNode& ul = Output.createChildNode("ul");

		ul.createChildNode("li").setContents("Resource: " + *Resource->getName());

		ul = ul.createChildNode("ul");

		ul.createChildNode("li").setContents("Type: " + *Resource->getTypeName());
		ul.createChildNode("li").setContents("Package: " + *Resource->getPackageName());


		// Iterate over all properties and do stuff.
		const BcReflectionClass* pClass = Resource ->getClass();

		// NOTE: Do not want to hit this. Ever.
		if (pClass == NULL)
		{
			int a = 0; ++a;
		}

		BcU8* pClassData = reinterpret_cast< BcU8* >(&Resource);
		BcChar temp[256];
		// Iterate over to grab offsets for classes.
		while (pClass != NULL)
		{
			BcHtmlNode& ul2 = Output.createChildNode("ul");
			BcSPrintf(temp, "Class: %s (Size: 0x%x)\n", (*pClass->getName()).c_str(), pClass->getSize());
			ul2.setContents(temp);

			BcHtmlNode& ul3 = ul2.createChildNode("ul");
			ul3.setContents(" ");
			for (BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx)
			{
				const BcReflectionField* pField = pClass->getField(Idx);
				const BcReflectionType* pType = pField->getType();
				if (pType != NULL)
				{
					// Output += "<li>";
					BcHtmlNode& div = ul3.createChildNode("div");
					div.setAttribute("id", "fieldData");
					//BcSPrintf(temp, "%s %s; // Offset 0x%x, Size 0x%x, Flags: 0x%x\n", (*pType->getName()).c_str(), (*pField->getName()).c_str(), pField->getOffset(), pType->getSize(), pField->getFlags());
					//BcSPrintf(temp, "%s")
					//Output += temp;
					BcHtmlNode& fName = div.createChildNode("div");
					fName.setAttribute("id", "fName");
					fName.setContents(*pField->getName());

					BcHtmlNode& fType = div.createChildNode("div");
					fType.setAttribute("id", "fType");
					fType.setContents(*pType->getName());

					BcHtmlNode& fValue = div.createChildNode("div");
					fValue.setAttribute("id", "fValue");
					if (pType->getName() == "BcU8")
					{
						const BcU8* pData = reinterpret_cast<const BcU8*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%u\n", *pData);
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcU16")
					{
						const BcU16* pData = reinterpret_cast<const BcU16*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%u\n", *pData);
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcU32")
					{
						const BcU32* pData = reinterpret_cast<const BcU32*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%u\n", *pData);
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcF32")
					{
						const BcF32* pData = reinterpret_cast<const BcF32*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%f\n", *pData);
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcName")
					{
						const BcName* pData = reinterpret_cast<const BcName*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%s\n", (**pData).c_str());
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcBool")
					{
						const BcBool* pData = reinterpret_cast<const BcBool*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%u\n", *pData);
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcVec2d")
					{
						const BcVec2d* pData = reinterpret_cast<const BcVec2d*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%f, %f\n", pData->x(), pData->y());
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcVec3d")
					{
						const BcVec3d* pData = reinterpret_cast<const BcVec3d*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%f, %f, %f\n", pData->x(), pData->y(), pData->z());
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcVec4d")
					{
						const BcVec4d* pData = reinterpret_cast<const BcVec4d*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%f, %f, %f, %f\n", pData->x(), pData->y(), pData->z(), pData->w());
						fValue.setContents(temp);
					}
					else if (pType->getName() == "BcMat4d")
					{
						/*const BcMat4d* pData = reinterpret_cast<const BcMat4d*>(&pClassData[pField->getOffset()]);
						BcSPrintf(temp, "%f, %f, %f, %f\n", pData->row0().x(), pData->row0().y(), pData->row0().z(), pData->row0().w());
						Output += temp;
						BcSPrintf(temp, "%f, %f, %f, %f\n", pData->row1().x(), pData->row1().y(), pData->row1().z(), pData->row1().w());
						Output += temp;
						BcSPrintf(temp, "%f, %f, %f, %f\n", pData->row2().x(), pData->row2().y(), pData->row2().z(), pData->row2().w());
						Output += temp;
						BcSPrintf(temp, "%f, %f, %f, %f\n", pData->row3().x(), pData->row3().y(), pData->row3().z(), pData->row3().w());
						Output += temp;/**/
						fValue.setContents("Not yet implimented");
					}
					else if (pType->getName() == "ScnEntity")
					{
						/*const ScnEntityRef* pData = reinterpret_cast<const ScnEntityRef*>(&pClassData[pField->getOffset()]);
						const BcReflectionClass* pClass = static_cast< const BcReflectionClass* >(pType);
						void* pNewData = reinterpret_cast< BcU8* >(pClassData)+pField->getOffset();
						ScnEntity* ptr = (ScnEntity*)pNewData;
						BcSPrintf(temp, "<a href=\"/Resource/%u\">Link</a>", (*pData)->getUniqueId());/**/
						fValue.setContents("Not yet implemented");
					}

					// Output += "</li>";
				}
			}

			pClass = pClass->getSuper();
			
		}

	}
}
