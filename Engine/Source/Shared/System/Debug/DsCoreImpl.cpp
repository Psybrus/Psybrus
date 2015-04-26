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

#include "Base/BcFile.h"
#include "Base/BcHtml.h"
#include "System/SysKernel.h"
#include "Serialisation/SeJsonWriter.h"
#include "Psybrus.h"
#include "DsTemplate.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "System/Debug/DsCoreLogging.h"


//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( DsCoreImpl );

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCoreImpl::DsCoreImpl()
{
	using namespace std::placeholders;

	NextHandle_ = 0;
	registerPage( "", {}, std::bind( &DsCoreImpl::cmdMenu, this, _1, _2, _3 ) );
	registerPage( "Content", {}, std::bind( &DsCoreImpl::cmdContent, this, _1, _2, _3 ), "Content" );
	registerPage( "Scene", {}, std::bind( &DsCoreImpl::cmdScene, this, _1, _2, _3 ), "Scene" );
	registerPage( "Log", {}, std::bind( &DsCoreImpl::cmdLog, this, _1, _2, _3 ), "Log" );
	registerPage( "Functions", {}, std::bind( &DsCoreImpl::cmdViewFunctions, this, _1, _2, _3 ) );

	registerPage( "Resource/(.*)", { "Id" }, std::bind( &DsCoreImpl::cmdResource, this, _1, _2, _3 ) );
	registerPage( "ResourceEdit/(.*)", { "Id" }, std::bind( &DsCoreImpl::cmdResourceEdit, this, _1, _2, _3 ) );
	registerPageNoHtml( "Json/(\\d*)", { "Id" }, std::bind( &DsCoreImpl::cmdJson, this, _1, _2, _3 ) );
	registerPageNoHtml( "JsonSerialise/(\\d*)", { "Id" }, std::bind( &DsCoreImpl::cmdJsonSerialiser, this, _1, _2, _3 ) );
	registerPageNoHtml("Wadl", {}, std::bind( &DsCoreImpl::cmdWADL, this, _1, _2, _3 ) );
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
			PSY_LOG( "WSAStartup failed" );
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
		PSY_LOG( "Failed to initialise Webby server" );
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
    WebbyServerUpdate( Server_ );
#endif

	for( auto& Panel : PanelFunctions_ )
	{
		Panel.Function_( Panel.Handle_ );
	}
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

//////////////////////////////////////////////////////////////////////////
// registerPanel
BcU32 DsCoreImpl::registerPanel( std::string Name, std::function < void( BcU32 )> Func )
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;
	PanelFunctions_.emplace_back( Name, Func, Handle );
	PSY_LOG( "Function registered." );
	PSY_LOG( "\t%s (%u)", Name.c_str(), Handle );
	return Handle;

}

//////////////////////////////////////////////////////////////////////////
// deregisterPanel
void DsCoreImpl::deregisterPanel( BcU32 Handle )
{
	auto FoundIt = std::find_if( PanelFunctions_.begin(), PanelFunctions_.end(),
		[ & ]( DsPanelDefinition& PanelDef )
		{
			return PanelDef.Handle_ == Handle;
		} );
	if( FoundIt != PanelFunctions_.end() )
	{
		PSY_LOG( "Panel deregistered." );
		PSY_LOG( "\t%s (%u)", FoundIt->Name_.c_str(), Handle );
	}
	else
	{
		PSY_LOG( "Panel deregister failed." );
		PSY_LOG( "\tHandle: %u", Handle );
	}
	BcAssert( BcIsGameThread() );

}

//////////////////////////////////////////////////////////////////////////
// registerFunction
BcU32 DsCoreImpl::registerFunction(std::string Display, std::function<void()> Function)
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;
	ButtonFunctions_.push_back( DsFunctionDefinition( Display, Function, Handle ) );
	PSY_LOG( "Function registered." );
	PSY_LOG( "\t%s (%u)", Display.c_str(), Handle );
	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCoreImpl::deregisterFunction(BcU32 Handle)
{
	BcAssert( BcIsGameThread() );
	BcBool functionRemoved = false;
	for (auto iter = ButtonFunctions_.begin(); iter != ButtonFunctions_.end(); ++iter)
	{
		if ((*iter).Handle_ == Handle)
		{
			PSY_LOG( "Function deregistered." );
			PSY_LOG( "\t%s (%u)", (*iter).DisplayText_.c_str(), Handle );
			ButtonFunctions_.erase( iter );
			functionRemoved = true;
			break;
		}
	}
	if ( !functionRemoved )
	{
		PSY_LOG( "Function deregister failed." );
		PSY_LOG( "\tHandle: %u", Handle );

	}
}

//////////////////////////////////////////////////////////////////////////
// registerPage
BcU32 DsCoreImpl::registerPage(std::string regex, std::vector<std::string> namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn, std::string display)
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;

	DsPageDefinition cm(regex, namedCaptures, display);
	cm.Function_ = fn;
	PageFunctions_.push_back(cm);
	DsCoreLogging::pImpl()->addLog( "DsCore", rand(), "Registering page: " + display );
	PSY_LOG( "Registered page" );
	PSY_LOG( "\t%s (%s)", regex.c_str(), display.c_str() );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPage
BcU32 DsCoreImpl::registerPage(std::string regex, std::vector<std::string> namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn)
{
	++NextHandle_;
	BcU32 Handle = NextHandle_;

	DsPageDefinition cm( regex, namedCaptures );
	cm.Function_ = fn;
	cm.IsHtml_ = true;
	PageFunctions_.push_back(cm);
	PSY_LOG( "Registered page (No content index)" );
	PSY_LOG( "\t%s (%u)", regex.c_str(), Handle );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPageNoHtml
BcU32 DsCoreImpl::registerPageNoHtml(std::string regex, std::vector<std::string> namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn)
{
	BcAssert( BcIsGameThread() );
	BcU32 Handle = ++NextHandle_;

	DsPageDefinition cm( regex, namedCaptures );
	cm.Function_ = fn;
	cm.IsHtml_ = false;
	PageFunctions_.push_back(cm);
	PSY_LOG( "Registered page without html" );
	PSY_LOG( "\t%s (%u)", regex.c_str(), Handle );

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// deregisterPage
void DsCoreImpl::deregisterPage( BcU32 Handle )
{
	BcAssert( BcIsGameThread() );
	BcBool pageRemoved = false;
	for (auto iter = PageFunctions_.begin(); iter != PageFunctions_.end(); ++iter)
	{
		if ((*iter).Handle_ == Handle)
		{
			PageFunctions_.erase(iter);
			pageRemoved = true;
			break;
		}
	}
	PSY_LOG( "Page deregistration failed." );
	PSY_LOG( "\tHandle: %u", Handle );

}


//////////////////////////////////////////////////////////////////////////
// handleFile
char* DsCoreImpl::handleFile(std::string Uri, int& FileSize, std::string PostContent)
{
	std::string type;
	char* Output;
	if (BcStrStr(Uri.c_str(), "/files/"))
	{
		Output = writeFile(&Uri[7], FileSize, type);
		return Output;
	}
	else
	{
		std::string out = loadHtmlFile(Uri, PostContent);
		FileSize = (int)out.length();
		Output = new char[FileSize + 1];
		BcMemSet(Output, 0, FileSize +1);
		BcMemCopy(Output, &out[0], FileSize);
		return Output;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Gets a plain text file
std::string DsCoreImpl::loadTemplateFile( std::string filename )
{
	BcFile file;
	std::string f = filename;
	file.open( f.c_str() );
	if ( !file.isOpen() )
		return 0;
	char* data;// = new BcU8[file.size()];
	data = ( char* ) file.readAllBytes();
	std::string output = data;
	delete data;
	// TODO: Actually load files
	return output;
}


std::string DsCoreImpl::loadHtmlFile(std::string Uri, std::string Content)
{
	BcHtml HtmlContent;

	DsTemplate::loadTemplate( HtmlContent.getRootNode(), "Content/Debug/fullpage_template.html" );
	HtmlContent.getRootNode().findNodeById( "id-title" ).setContents( GPsySetupParams.Name_ );

	std::vector<std::string> data;
	bool success = false;
	std::string uri = &Uri[1];
		
	for (auto Item : ButtonFunctions_)
	{
		if ( uri == ("Functions/" + std::to_string( Item.Handle_ ) ) )
		{
			BcHtmlNode redirect = HtmlContent.getRootNode().findNodeById( "meta" );
			redirect.setAttribute("http-equiv", "refresh");
			redirect.setAttribute("content", "0; url=/Menu");
			Item.Function_();
			success = true;
		} 
	}

	if (!success)
	{
		for (int Idx = (int)PageFunctions_.size() - 1; Idx >= 0; --Idx)
		{
			std::cmatch match;
			std::regex_match( &Uri[1], match, PageFunctions_[Idx].Regex_ );
			if (match.size() > 0)
			{
				std::string javaScript = "var params = [";
				for (BcU32 Idx2 = 1; Idx2 < match.size(); ++Idx2)
				{
					std::string u = match[Idx2];
					data.push_back(u);
					if (Idx2 > 1)
						javaScript += ",";
					javaScript += "\n\"";
					javaScript += u;
					javaScript += "\"";
				}
				javaScript += "];";
				HtmlContent.getRootNode().findNodeById( "js-params" ).setContents(javaScript);
				auto innerBodyNode = HtmlContent.getRootNode().findNodeById( "innerBody");
				PageFunctions_[Idx].Function_(data, innerBodyNode, Content );
				if (!PageFunctions_[Idx].IsHtml_)
					return HtmlContent.getRootNode().findNodeById( "innerBody" ).getContents();
				break;
			}
		}
	}

	std::string Output = HtmlContent.getHtml();
	return Output;
}

//////////////////////////////////////////////////////////////////////////
// writeHeader
void DsCoreImpl::writeHeader(BcHtmlNode& Output)
{
	BcHtmlNode header = Output.createChildNode("div").setAttribute("id", "headerSection");
	header.createChildNode("h1").setContents(GPsySetupParams.Name_);

	BcHtmlNode link = Output.createChildNode("div").setAttribute("id", "headerLink");
	link.createChildNode("a").setAttribute("href", "/Menu").setContents("Menu").setAttribute("id", "headerLinkText");

}

//////////////////////////////////////////////////////////////////////////
// writeFooter
void DsCoreImpl::writeFooter(BcHtmlNode& Output)
{
	BcHtmlNode footer = Output.createChildNode("div").setAttribute("id", "footer").setContents("Footer");
}

//////////////////////////////////////////////////////////////////////////
// Gets a file for the output stream
char* DsCoreImpl::writeFile( std::string filename, int& OutLength, std::string& type )
{
	BcFile file;
	std::string f = "Content/Debug/";
	f += filename;
	file.open( f.c_str() );
	if ( !file.isOpen() )
		return 0;
	char* data;// = new BcU8[file.size()];
	data = ( char* ) file.readAllBytes();
	OutLength = (int)file.size();
	type = "css";
	// TODO: Actually load files
	return data;
}

//////////////////////////////////////////////////////////////////////////
// cmdMenu
void DsCoreImpl::cmdMenu(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	DsTemplate::loadTemplate(Output, "Content/Debug/main_items.html" );

	BcHtmlNode ul = Output.findNodeById( "page_listing" );
	
	for (BcU32 Idx = 0; Idx < PageFunctions_.size(); ++Idx)
	{
		if (PageFunctions_[Idx].Visible_)
		{
			BcHtmlNode a = ul.createChildNode("li").createChildNode("a");
			a.setAttribute("href", PageFunctions_[Idx].Text_);
			a.setContents(PageFunctions_[Idx].Display_);
		}
	}
	BcHtmlNode functions = Output.findNodeById( "function_listing" );
	for (auto Item : ButtonFunctions_)
	{
		BcHtmlNode ahref = functions.createChildNode("a");
		std::string v = std::to_string( Item.Handle_ );
		ahref.setAttribute( "href", "Functions/" + v );

		BcHtmlNode button = ahref.createChildNode( "button" );
		button.setAttribute( "type", "button" );
		button.setContents( Item.DisplayText_ );
		functions.createChildNode( "br" );
	}
	/**/
}


//////////////////////////////////////////////////////////////////////////
// cmdContent
void DsCoreImpl::cmdContent(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcHtmlNode node = DsTemplate::loadTemplate( Output, "Content/Debug/contents_template.html" );
	node.findNodeById( "id-resources" ).setContents( std::to_string( CsCore::pImpl()->getNoofResources() ) );

	BcHtmlNode table = node.findNodeById( "id-table" );

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResource( Idx ) );
		BcHtmlNode row = DsTemplate::loadTemplate( table, "Content/Debug/content_row_template.html" );

		std::string id = std::to_string(Resource->getUniqueId());
		row.findNodeById( "id-link" ).setAttribute("href", "Resource/" + id).setContents(*Resource->getName());
		row.findNodeById( "id-name" ).setContents( *Resource->getClass()->getName() );
		row.findNodeById( "id-package-name" ).setContents( *Resource->getPackageName() );

	}
}

//////////////////////////////////////////////////////////////////////////
// cmdScene
void DsCoreImpl::cmdScene(DsParameters params, BcHtmlNode& Output, std::string PostContent)
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
// cmdScene_Entity
void DsCoreImpl::cmdScene_Entity( ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth)
{
	BcHtmlNode ul = Output.createChildNode("ul");
	BcChar Id[32];
	BcSPrintf(Id, "%d", Entity->getUniqueId());
	
	// Entity name.
	BcHtmlNode li = ul.createChildNode("li");
	li.setContents("Entity; ");
	BcHtmlNode a = li.createChildNode("a");
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
void DsCoreImpl::cmdScene_Component( ScnComponentRef Component, BcHtmlNode& Output, BcU32 Depth )
{
	BcChar Id[32];
	BcSPrintf(Id, "%d", Component->getUniqueId());
	BcHtmlNode tmp = DsTemplate::loadTemplate( Output, "Content/Debug/scene_component_template.html" );

	tmp.findNodeById( "component-link" ).setAttribute( "href", "/Resource/" + std::string( Id ) );
	tmp.findNodeById( "component-link" ).setContents( *Component->getName() );

}


//////////////////////////////////////////////////////////////////////////
// cmdLog
void DsCoreImpl::cmdLog(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	/*
	BcLog* log = BcLog::pImpl();

	BcHtmlNode ul = Output.createChildNode("ul");
	std::vector<std::string> logs = log->getLogData();
	for (auto val : logs)
	{
		ul.createChildNode("li").setContents(val);
	}//*/
	BcHtmlNode ul = Output.createChildNode("ul");
	std::vector< DsCoreLogEntry > logs = DsCoreLogging::pImpl()->getEntries( nullptr, 0 );

	for ( auto val : logs )
	{
		ul.createChildNode( "li" ).setContents( val.Entry_ ).setAttribute( "id", "Log-" + val.Category_ );
	}//*/

}

//////////////////////////////////////////////////////////////////////////
// cmdResource
void DsCoreImpl::cmdResource(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	std::string EntityId = "";

	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());

	ReObjectRef< CsResource > Resource(CsCore::pImpl()->getResourceByUniqueId(id));

	if (Resource == nullptr)
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
		Output.createChildNode("h2").setContents("Information");
		BcHtmlNode table = Output.createChildNode("table");
		table.createChildNode("col").setAttribute("wid", "150px");
		table.createChildNode("col").setAttribute("wid", "150px");

		BcHtmlNode row = table.createChildNode("tr");
		row.createChildNode("td").setContents("Resource:");
		row.createChildNode("td").setContents(*Resource->getName());

		row = table.createChildNode("tr");
		row.createChildNode("td").setContents("Type:");
		row.createChildNode("td").setContents(*Resource->getClass()->getName());

		row = table.createChildNode("tr");
		row.createChildNode("td").setContents("Package:");
		row.createChildNode("td").setContents(*Resource->getPackageName());
		BcHtmlNode ul = Output.createChildNode("ul");

		Output.createChildNode("h2").setContents("Fields");
		// Iterate over all properties and do stuff.
		const ReClass* pClass = Resource ->getClass();

		// NOTE: Do not want to hit this. Ever.
		if (pClass == NULL)
		{
			int a = 0; ++a;
		}
		// Iterate over to grab offsets for classes.
		while (pClass != NULL)
		{
			BcHtmlNode base = Output.createChildNode("div");
			base.createChildNode("div").setContents(pClass->getName().getValue()).setAttribute("id", "classHeader");
			BcHtmlNode div = base.createChildNode("div").setAttribute("id", "innerData");
			BcHtmlNode tbl = div.createChildNode("table");
			BcHtmlNode header = tbl.createChildNode("tr");
			header.createChildNode("th").setContents("Name").setAttribute("width", "15%");
			header.createChildNode("th").setContents("Type").setAttribute("width", "20%");
			header.createChildNode("th").setContents("Value").setAttribute("width", "70%*");


			for (BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx)
			{
				BcHtmlNode row = tbl.createChildNode("tr");
				ReFieldAccessor SrcFieldAccessor(Resource, pClass->getField(Idx)); 
				auto Field = pClass->getField(Idx);

				row.createChildNode("td").setContents(Field->getName().getValue());
				row.createChildNode("td").setContents(Field->getType()->getName().getValue());
				BcHtmlNode fValue = row.createChildNode("td");

				if (!SrcFieldAccessor.isContainerType())
				{
					const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();
					void* data = SrcFieldAccessor.getData();
					std::string str = "UNKNOWN";
					
					if (SrcFieldAccessor.getUpperClass()->hasBaseClass(CsResource::StaticGetClass()))
					{
						CsResource* resource = static_cast<CsResource*>(SrcFieldAccessor.getData());
						if( resource != nullptr )
						{
							fValue.createChildNode("a").setAttribute("href", "/Resource/" + std::to_string(resource->getUniqueId())).setContents("Resource");
							str = "";
						}
						else
						{
							str = "NULL";
						}

					} else if (FieldClass->getTypeSerialiser() != nullptr)
					{
						FieldClass->getTypeSerialiser()->serialiseToString(data, str);
					}
					fValue.setContents(str);
				}
				else
				{
					fValue.setContents("CONTAINER");
					auto KeyType = Field->getKeyType();

					if (KeyType == nullptr)
					{
						// Do something I guess
					}
					else
					{
					}
				}
				// Ignore null pointers, transients, and shallow copies.
				if (!SrcFieldAccessor.isNullptr() &&
					!SrcFieldAccessor.isTransient() &&
					!SrcFieldAccessor.isShallowCopy())
				{
					if (SrcFieldAccessor.isPointerType())
					{
						//gatherFieldPointer(SrcFieldAccessor);
					}
					else if (SrcFieldAccessor.isContainerType() && SrcFieldAccessor.isContainerOfPointerValues())
					{
						//gatherFieldContainer(SrcFieldAccessor);
					}
				}
			}
			Output.createChildNode("br");
			pClass = pClass->getSuper();
			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// cmdResourceEdit
void DsCoreImpl::cmdResourceEdit(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcHtmlNode root = Output.createChildNode("div");
	BcHtmlNode table = root.createChildNode("table").setAttribute("id", "items");
	BcHtmlNode header = table.createChildNode("th");
	header.createChildNode("td").setContents("Variable");
	header.createChildNode("td").setContents("Value");
}

void DsCoreImpl::cmdWADL(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcHtml html;
	html.getRootNode().setTag("application");
	html.getRootNode().setAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance" )
						.setAttribute("xsi:schemaLocation","http://wadl.dev.java.net/2009/02 wadl.xsd" )
						.setAttribute("xmlns:tns","urn:yahoo:yn")
						.setAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema")
						.setAttribute("xmlns:yn","urn:yahoo:yn")
						.setAttribute("xmlns:ya","urn:yahoo:api")
 						.setAttribute("xmlns","http://wadl.dev.java.net/2009/02");
	BcHtmlNode node = html.getRootNode();
	// TODO: Make this adjust depending on the port somehow :S
	BcHtmlNode resources = node.createChildNode("resources").setAttribute("base", "http://127.0.0.1:1337");
	// THAT REGEX
	// \(\?\<(?<name>\w*)\>\.\*\)
	// THIS WILL LOOK HORRIBLE
#if !PLATFORM_HTML5
	BcBreakpoint; // neilogd: Not sure if this is a correct conversion to std::regex.
	std::regex re("\\(\\?\\<(?<name>\\w*)\\>\\.\\*\\)");
	
	for (BcU32 Idx = 0; Idx < PageFunctions_.size(); ++Idx)
	{
		std::smatch results;
		BcHtmlNode resource = resources.createChildNode("resource");
		std::string replacement = PageFunctions_[Idx].Text_;
		if (std::regex_search(replacement, results, re)) 
		{
			for (auto item : results)
			{
				resource.createChildNode("item").setContents(item.str());
			}
		}

		std::regex_replace(replacement, re, "{$1}");

		//std::string other = boost::regex_replace();
		resource.setAttribute("path", PageFunctions_[Idx].Text_);


	}
#else
	BcBreakpoint; // TODO: Switch to std::regex.
#endif

	Output.setContents(html.getHtml());
}

void DsCoreImpl::cmdJsonSerialiser(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	std::string EntityId = "";

	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());
	std::string OutputString = "{\n";
	ReObjectRef< CsResource > Resource(CsCore::pImpl()->getResourceByUniqueId(id));

	if (Resource == nullptr)
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		OutputString = " { } ";
		Output.setContents(OutputString);
		return;
	}
	Json::Value readRoot;
	Json::Reader reader;
	bool PostContentAvailable = PostContent.size() > 0;
	reader.parse(PostContent, readRoot);
	
	Json::Value root;

	Json::Value classes = Json::Value(Json::arrayValue);
	if (Resource->getClass() == ScnEntity::StaticGetClass())
	{
		cmdScene_Entity(ScnEntityRef(Resource), Output, 0);
	}
	else
	{
		// Iterate over all properties and do stuff.
		const ReClass* pClass = Resource ->getClass();

		// NOTE: Do not want to hit this. Ever.
		if (pClass == NULL)
		{
			int a = 0; ++a;
		}
		// Iterate over to grab offsets for classes.
		while (pClass != NULL)
		{
			Json::Value theClass;
			theClass["className"] = pClass->getName().getValue();

			Json::Value readNode;
			for (auto v : readRoot["classes"])
			{
				if (v["className"].asString() == pClass->getName().getValue())
				{
					readNode = v;
				}
			}
			for (BcU32 Idx = 0; Idx < pClass->getNoofFields(); ++Idx)
			{
				ReFieldAccessor SrcFieldAccessor(Resource, pClass->getField(Idx)); 
				auto Field = pClass->getField(Idx);

				if (!SrcFieldAccessor.isContainerType())
				{
					const ReClass* FieldClass = SrcFieldAccessor.getUpperClass();
					void* data = SrcFieldAccessor.getData();
					std::string str = "UNKNOWN";
					std::string fieldName = Field->getName().getValue();
					
					if (SrcFieldAccessor.getUpperClass()->hasBaseClass(CsResource::StaticGetClass()))
					{
						CsResource* resource = static_cast<CsResource*>(SrcFieldAccessor.getData());
						if( resource != nullptr )
						{
							str = "";
						}

					} else if (FieldClass->getTypeSerialiser() != nullptr)
					{
						if (PostContentAvailable && (Field->getFlags() & ReFieldFlags::bcRFF_DEBUG_EDIT))
						{
							std::string newValue = readNode["data"][fieldName].asString();
							FieldClass->getTypeSerialiser()->serialiseFromString(data, newValue);
						}
						FieldClass->getTypeSerialiser()->serialiseToString(data, str);
					}
					if (Field->getFlags() & ReFieldFlags::bcRFF_DEBUG_EDIT)
					{
						theClass["data"][fieldName] = str;
					}
				}
				else
				{
					auto KeyType = Field->getKeyType();

					if (KeyType == nullptr)
					{
						// Do something I guess
					}
					else
					{
					}
				}
				// Ignore null pointers, transients, and shallow copies.
				if (!SrcFieldAccessor.isNullptr() &&
					!SrcFieldAccessor.isTransient() &&
					!SrcFieldAccessor.isShallowCopy())
				{
					if (SrcFieldAccessor.isPointerType())
					{
						//gatherFieldPointer(SrcFieldAccessor);
					}
					else if (SrcFieldAccessor.isContainerType() && SrcFieldAccessor.isContainerOfPointerValues())
					{
						//gatherFieldContainer(SrcFieldAccessor);
					}
				}
			}
			pClass = pClass->getSuper();
			classes.append(theClass);
		}
	}
	root["classes"] = (classes);
	Output.setContents(root.toStyledString());
}

void DsCoreImpl::cmdViewFunctions(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{

}


void DsCoreImpl::cmdJson(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	std::string EntityId = "";

	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());

	ReObjectRef< CsResource > Resource(CsCore::pImpl()->getResourceByUniqueId(id));

	if (Resource == nullptr)
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}

	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, ( BcU32 ) bcRFF_ALL, ( BcU32 ) bcRFF_TRANSIENT, ( BcU32 ) bcRFF_ALL );
	SeJsonWriter writer( &ObjectCodec );
	std::string output = writer.serialiseToString<CsResource>(Resource, Resource->getClass());
	
	Output.setContents(output);
}