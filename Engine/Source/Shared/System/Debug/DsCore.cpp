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

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////
// Creator
// SYS_CREATOR( DsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCore::DsCore()
{
	registerPage("", &cmdMenu);
	registerPage("Content", &cmdContent, "Content");
	registerPage("Scene", &cmdScene, "Scene");
	registerPage("Log", &cmdLog, "Log");
	registerPage("Resource/(?<Id>.*)", &cmdResource);
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCore::~DsCore()
{

}


//////////////////////////////////////////////////////////////////////////
// cmdContent
void DsCore::cmdContent(DsParameters params, BcHtmlNode& Output)
{
	Output.createChildNode("h1").setContents("Contents");
	BcHtmlNode table = Output.createChildNode("table");
	//table.setAttribute("style", "width:100%;");
	table.createChildNode("col").setAttribute("width", "100px");
	table.createChildNode("col").setAttribute("width", "100px");
	BcHtmlNode row = table.createChildNode("tr");
	row.createChildNode("th").setContents("Name");
	row.createChildNode("th").setContents("Type");
	row.createChildNode("th").setContents("Package");

	for( BcU32 Idx = 0; Idx < CsCore::pImpl()->getNoofResources(); ++Idx )
	{
		ReObjectRef< CsResource > Resource( CsCore::pImpl()->getResource( Idx ) );
		BcHtmlNode row = table.createChildNode("tr");
		std::string id = boost::lexical_cast<std::string>(Resource->getUniqueId());
		row.createChildNode("td").createChildNode("a").setAttribute("href", "Resource/" + id).setContents(*Resource->getName());
		row.createChildNode("td").setContents(*Resource->getClass()->getName());
		row.createChildNode("td").setContents(*Resource->getPackageName());

	}

}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::registerFunction(std::string Display, std::function<void()> Function)
{
	ButtonFunctions_.push_back(DsFunctionDefinition(Display, Function));
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::deregisterFunction(std::string Display)
{
	for (auto iter = ButtonFunctions_.begin(); iter != ButtonFunctions_.end(); ++iter)
	{
		if ((*iter).DisplayText_.compare(Display)){
			ButtonFunctions_.erase(iter);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn, std::string display)
{
	DsPageDefinition cm(regex, display);
	cm.Function_ = fn;
	PageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// registerPage
void DsCore::registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&)> fn)
{
	DsPageDefinition cm(regex);
	cm.Function_ = fn;
	PageFunctions_.push_back(cm);
}

//////////////////////////////////////////////////////////////////////////
// deregisterPage
void DsCore::deregisterPage(std::string regex)
{
	for (auto iter = PageFunctions_.begin(); iter != PageFunctions_.end(); ++iter)
	{
		if ((*iter).Text_.compare(regex.c_str())){
			PageFunctions_.erase(iter);
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
	BcHtmlNode mainNode = Output.createChildNode("div");
	mainNode.setAttribute("id", "menuWrapper");
	BcHtmlNode pages = mainNode.createChildNode("div");
	pages.setAttribute("id", "pages");
	BcHtmlNode ul = pages.createChildNode("ul");
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
	BcHtmlNode functions = mainNode.createChildNode("div");
	functions.setAttribute("id", "pages");
	for (auto Item : core->ButtonFunctions_)
	{
		BcHtmlNode ahref = functions.createChildNode("a");
		ahref.setAttribute("href", "Functions/" + Item.DisplayText_);

		BcHtmlNode button = ahref.createChildNode("button");
		button.setAttribute("type", "button");
		button.setContents(Item.DisplayText_);
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
	li.createChildNode("").setContents(" (" + *Component->getClass()->getName() + ")");
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

	EntityId = params[0];
	if (!BcStrIsNumber(EntityId.c_str()))
	{
		Output.createChildNode("").setContents("Invalid resource Id");
		Output.createChildNode("br");
		return;
	}
	BcU32 id = BcStrAtoi(EntityId.c_str());

	ReObjectRef< CsResource > Resource(CsCore::pImpl()->getResourceByUniqueId(id));

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

void DsCore::cmdLog(DsParameters params, BcHtmlNode& Output)
{
	BcLog* log = BcLog::pImpl();

	BcHtmlNode ul = Output.createChildNode("ul");
	std::vector<std::string> logs = log->getLogData();
	for (auto val : logs)
	{
		ul.createChildNode("li").setContents(val);
	}
}
