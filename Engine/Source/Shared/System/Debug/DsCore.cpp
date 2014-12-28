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
#include "Base/BcFile.h"
#include "Base/BcHtml.h"
#include "System/SysKernel.h"
#include "Serialisation/SeJsonWriter.h"
#include "Psybrus.h"
#include "DsTemplate.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include "System/Content/CsSerialiserPackageObjectCodec.h"

//////////////////////////////////////////////////////////////////////////
// Creator
// SYS_CREATOR( DsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCore::DsCore()
{
	NextHandle_ = 0;
	registerPage("", &cmdMenu);
	registerPage("Content", &cmdContent, "Content");
	registerPage("Scene", &cmdScene, "Scene");
	registerPage("Log", &cmdLog, "Log");
	registerPage("Functions", &cmdViewFunctions);
	registerPage("Resource/(?<Id>.*)", &cmdResource);
	registerPage("ResourceEdit/(?<Id>.*)", &cmdResourceEdit);
	registerPageNoHtml("Json/(?<Id>\\d*)", &cmdJson);
	registerPageNoHtml("JsonSerialise/(?<Id>\\d*)", &cmdJsonSerialiser);
	registerPageNoHtml("Wadl", &cmdWADL);
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCore::~DsCore()
{

}


//////////////////////////////////////////////////////////////////////////
// cmdContent
void DsCore::cmdContent(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcHtmlNode node = DsTemplate::loadTemplate( Output, "Content/debug/contents_template.html" );
	node.findNodeById( "id-resources" ).setContents( boost::lexical_cast< std::string >( CsCore::pImpl()->getNoofResources() ) );

	BcHtmlNode table = node.findNodeById( "id-table" );

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResource( Idx ) );
		BcHtmlNode row = DsTemplate::loadTemplate( table, "Content/debug/content_row_template.html" );

		std::string id = boost::lexical_cast<std::string>(Resource->getUniqueId());
		row.findNodeById( "id-link" ).setAttribute("href", "Resource/" + id).setContents(*Resource->getName());
		row.findNodeById( "id-name" ).setContents( *Resource->getClass()->getName() );
		row.findNodeById( "id-package-name" ).setContents( *Resource->getPackageName() );

	}

}

//////////////////////////////////////////////////////////////////////////
// registerFunction
BcU32 DsCore::registerFunction(std::string Display, std::function<void()> Function)
{
	++NextHandle_;
	BcU32 Handle = NextHandle_;
	ButtonFunctions_.push_back( DsFunctionDefinition( Display, Function, Handle ) );
	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::deregisterFunction(BcU32 Handle)
{
	for (auto iter = ButtonFunctions_.begin(); iter != ButtonFunctions_.end(); ++iter)
	{
		if ((*iter).Handle_ == Handle)
		{
			ButtonFunctions_.erase(iter);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn, std::string display)
{
	DsPageDefinition cm(regex, display);
	cm.Function_ = fn;
	PageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn)
{
	DsPageDefinition cm(regex);
	cm.Function_ = fn;
	cm.IsHtml_ = true;
	PageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// registerPageNoHtml
void DsCore::registerPageNoHtml(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn)
{
	DsPageDefinition cm(regex);
	cm.Function_ = fn;
	cm.IsHtml_ = false;
	PageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// deregisterPage
void DsCore::deregisterPage(std::string regex)
{
	for (auto iter = PageFunctions_.begin(); iter != PageFunctions_.end(); ++iter)
	{
		if ((*iter).Text_.compare(regex.c_str()))
		{
			PageFunctions_.erase(iter);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// cmdScene
void DsCore::cmdScene(DsParameters params, BcHtmlNode& Output, std::string PostContent)
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
void DsCore::cmdMenu(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	DsTemplate::loadTemplate(Output, "Content/Debug/main_items.html" );

	BcHtmlNode ul = Output.findNodeById( "page_listing" );
	DsCore* core = pImpl();
	for (BcU32 Idx = 0; Idx < core->PageFunctions_.size(); ++Idx)
	{
		if (core->PageFunctions_[Idx].Visible_)
		{
			BcHtmlNode a = ul.createChildNode("li").createChildNode("a");
			a.setAttribute("href", core->PageFunctions_[Idx].Text_);
			a.setContents(core->PageFunctions_[Idx].Display_);
		}
	}
	BcHtmlNode functions = Output.findNodeById( "function_listing" );
	for (auto Item : core->ButtonFunctions_)
	{
		BcHtmlNode ahref = functions.createChildNode("a");
		std::string v = boost::lexical_cast< std::string >( Item.Handle_ );
		ahref.setAttribute( "href", "Functions/" + v );

		BcHtmlNode button = ahref.createChildNode( "button" );
		button.setAttribute( "type", "button" );
		button.setContents( Item.DisplayText_ );
		functions.createChildNode( "br" );
	}
	/**/
}

//////////////////////////////////////////////////////////////////////////
// cmdScene_Entity
void DsCore::cmdScene_Entity( ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth)
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
void DsCore::cmdScene_Component( ScnComponentRef Component, BcHtmlNode& Output, BcU32 Depth )
{
	BcChar Id[32];
	BcSPrintf(Id, "%d", Component->getUniqueId());
	BcHtmlNode tmp = DsTemplate::loadTemplate( Output, "Content/Debug/scene_component_template.html" );

	tmp.findNodeById( "component-link" ).setAttribute( "href", "/Resource/" + std::string( Id ) );
	tmp.findNodeById( "component-link" ).setContents( *Component->getName() );

}

//////////////////////////////////////////////////////////////////////////
// writeHeader
void DsCore::writeHeader(BcHtmlNode& Output)
{
	BcHtmlNode header = Output.createChildNode("div").setAttribute("id", "headerSection");
	header.createChildNode("h1").setContents(GPsySetupParams.Name_);

	BcHtmlNode link = Output.createChildNode("div").setAttribute("id", "headerLink");
	link.createChildNode("a").setAttribute("href", "/Menu").setContents("Menu").setAttribute("id", "headerLinkText");

}

//////////////////////////////////////////////////////////////////////////
// writeFooter
void DsCore::writeFooter(BcHtmlNode& Output)
{
	BcHtmlNode footer = Output.createChildNode("div").setAttribute("id", "footer").setContents("Footer");
	

}

//////////////////////////////////////////////////////////////////////////
// Gets a file for the output stream
char* DsCore::writeFile( std::string filename, int& OutLength, std::string& type )
{
	BcFile file;
	std::string f = "Content/Debug/";
	f += filename;
	file.open( f.c_str() );
	if ( !file.isOpen() )
		return 0;
	char* data;// = new BcU8[file.size()];
	data = ( char* ) file.readAllBytes();
	OutLength = file.size();
	type = "css";
	// TODO: Actually load files
	return data;
}

//////////////////////////////////////////////////////////////////////////
// Gets a plain text file
std::string DsCore::loadTemplateFile( std::string filename )
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

//////////////////////////////////////////////////////////////////////////
// cmdResource
void DsCore::cmdResource(DsParameters params, BcHtmlNode& Output, std::string PostContent)
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
		BcU8* pClassData = reinterpret_cast< BcU8* >(&Resource);
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
							fValue.createChildNode("a").setAttribute("href", "/Resource/" + boost::lexical_cast<std::string>(resource->getUniqueId())).setContents("Resource");
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
					auto SrcIter = SrcFieldAccessor.newReadIterator();
					auto KeyType = Field->getKeyType();
					auto ValueType = Field->getValueType();

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

void DsCore::cmdLog(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcLog* log = BcLog::pImpl();

	BcHtmlNode ul = Output.createChildNode("ul");
	std::vector<std::string> logs = log->getLogData();
	for (auto val : logs)
	{
		ul.createChildNode("li").setContents(val);
	}
}

char* DsCore::handleFile(std::string Uri, int& FileSize, std::string PostContent)
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
		FileSize = out.length();
		Output = new char[FileSize + 1];
		BcMemSet(Output, 0, FileSize +1);
		BcMemCopy(Output, &out[0], FileSize);
		return Output;
	}
	return 0;
}

std::string DsCore::loadHtmlFile(std::string Uri, std::string Content)
{
	BcHtml HtmlContent;

	DsTemplate::loadTemplate( HtmlContent.getRootNode(), "Content/Debug/fullpage_template.html" );
	HtmlContent.getRootNode().findNodeById( "id-title" ).setContents( GPsySetupParams.Name_ );

	std::vector<std::string> data;
	bool success = false;
	std::string uri = &Uri[1];
		
	for (auto Item : ButtonFunctions_)
	{
		if ( uri == ("Functions/" + boost::lexical_cast< std::string >( Item.Handle_ ) ) )
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
		for (size_t Idx = PageFunctions_.size() - 1; Idx >= 0; --Idx)
		{
			BcRegexMatch match;
			BcU32 res = PageFunctions_[Idx].Regex_.match(&Uri[1], match);
			if (res > 0)
			{
				std::string javaScript = "var params = [";
				for (BcU32 Idx2 = 1; Idx2 < match.noofMatches(); ++Idx2)
				{
					std::string u;
					match.getMatch(Idx2, u);
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

void DsCore::cmdJson(DsParameters params, BcHtmlNode& Output, std::string PostContent)
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
	CsSerialiserPackageObjectCodec ObjectCodec( nullptr, bcRFF_ALL, bcRFF_TRANSIENT, bcRFF_ALL );
	SeJsonWriter writer( &ObjectCodec);
	std::string output = writer.serialiseToString<CsResource>(Resource, Resource->getClass());
	
	Output.setContents(output);
}

void DsCore::cmdWADL(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	DsCore* core = DsCore::pImpl();
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
	boost::regex re("\\(\\?\\<(?<name>\\w*)\\>\\.\\*\\)");
	
	for (BcU32 Idx = 0; Idx < core->PageFunctions_.size(); ++Idx)
	{
		boost::smatch results;
		BcHtmlNode resource = resources.createChildNode("resource");
		std::string replacement = core->PageFunctions_[Idx].Text_;
		auto wat = re.get_named_subs();
		
		if (boost::regex_search(replacement, results, re)) 
		{
			for (auto item : results)
			{
				resource.createChildNode("item").setContents(item.str());
			}
		}

		boost::replace_all(replacement, re, "{$1}");

		//std::string other = boost::regex_replace();
		resource.setAttribute("path", core->PageFunctions_[Idx].Text_);


	}
	Output.setContents(html.getHtml());
}

void DsCore::cmdJsonSerialiser(DsParameters params, BcHtmlNode& Output, std::string PostContent)
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
	bool success = reader.parse(PostContent, readRoot);
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
		BcU8* pClassData = reinterpret_cast< BcU8* >(&Resource);
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
					auto SrcIter = SrcFieldAccessor.newReadIterator();
					auto KeyType = Field->getKeyType();
					auto ValueType = Field->getValueType();

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

void DsCore::cmdResourceEdit(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{
	BcHtmlNode root = Output.createChildNode("div");
	BcHtmlNode table = root.createChildNode("table").setAttribute("id", "items");
	BcHtmlNode header = table.createChildNode("th");
	header.createChildNode("td").setContents("Variable");
	header.createChildNode("td").setContents("Value");
}

void DsCore::cmdViewFunctions(DsParameters params, BcHtmlNode& Output, std::string PostContent)
{

}
