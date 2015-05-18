/**************************************************************************
*
* File:		DsCoreImpl.h
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug system core.
*		
*
*
* 
**************************************************************************/

#ifndef __DSCOREIMPL_H__
#define __DSCOREIMPL_H__

#include "Base/BcGlobal.h"
#include "Base/BcHtml.h"
#include "System/SysSystem.h"

#include "System/Content/CsCore.h"
#include "System/Scene/ScnCore.h"
#include "DsCore.h"

#include <functional>
#include <map>

#if USE_WEBBY
#include "webby.h"
#endif // USE_WEBBY

//////////////////////////////////////////////////////////////////////////
/**	\class DsPanelDefinition
*/
struct DsPanelDefinition
{
	DsPanelDefinition( std::string Name, std::function< void( BcU32 ) > Func, BcU32 Handle ) :
		Name_( Name ),
		Function_( Func ),
		Handle_( Handle )
	{}

	std::string Name_;
	std::function< void( BcU32 ) > Function_;
	BcU32 Handle_;
};

//////////////////////////////////////////////////////////////////////////
/**	\class DsPageDefinition
*/
struct DsPageDefinition
{
	DsPageDefinition(std::string r, std::vector<std::string> namedCaptures, std::string display)
		: Regex_(r.c_str()),
		NamedCaptures_( namedCaptures ),
		Text_(r),
		Display_(display),
		Visible_(true),
		IsHtml_(true) 
	{}

	DsPageDefinition(std::string r, std::vector<std::string> namedCaptures)
		: Regex_(r.c_str()), 
		NamedCaptures_( namedCaptures ),
		Text_(r),
		Visible_(false),
		IsHtml_(true)
	{}

	std::regex Regex_;
	std::vector<std::string> NamedCaptures_; 
	std::string Text_;
	std::string Display_;
	bool Visible_;
	bool IsHtml_;
	std::function <void(DsParameters, BcHtmlNode&, std::string)> Function_;
	BcU32 Handle_;
};

//////////////////////////////////////////////////////////////////////////
/**	\class DsFunctionDefinition
*/
struct DsFunctionDefinition
{
	DsFunctionDefinition(std::string text, std::function<void()> fn, BcU32 handle)
	:
	DisplayText_( text ), Function_( fn ), Handle_( handle )
	{}

	std::string DisplayText_;
	std::function<void()> Function_;
	BcU32 Handle_;
};

//////////////////////////////////////////////////////////////////////////
/**	\class DsCoreImpl
*	\brief Debug System Core
*
*	Debugging system core.
*/
class DsCoreImpl :
	public DsCore
{
public:
	static BcU32 JOB_QUEUE_ID;

public:
	DsCoreImpl();
	virtual ~DsCoreImpl();

	void open() override;
	void update() override;
	void close() override;

	BcU32 registerPanel( std::string Name, std::function < void( BcU32 )> Func ) override;
	void deregisterPanel( BcU32 Handle ) override;
	void drawObjectEditor( DsImGuiFieldEditor* ThisFieldEditor, void* Data, const ReClass* Class, BcU32 Flags ) override;
	BcU32 registerPage( std::string regex, std::vector< std::string > namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn, std::string display ) override;
	BcU32 registerPage( std::string regex, std::vector< std::string > namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn ) override;
	BcU32 registerPageNoHtml( std::string regex, std::vector<std::string> namedCaptures, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn ) override;
	void deregisterPage( BcU32 Handle ) override;
	BcU32 registerFunction( std::string Display, std::function< void() > Function ) override;
	void deregisterFunction( BcU32 Handle ) override;

private:
	void setupReflectionEditorAttributes();
	char* handleFile( std::string Uri, int& FileSize, std::string Content );
	std::string loadHtmlFile( std::string Uri, std::string Content );
	std::string loadTemplateFile( std::string TemplateName );

	void writeHeader( BcHtmlNode& Output );
	void writeFooter( BcHtmlNode& Output );
	char* writeFile( std::string filename, int& OutLength, std::string& type );

private:
	void cmdMenu(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdContent(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdScene(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdScene_Entity(ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth);
	void cmdScene_Component(ScnComponentRef Entity, BcHtmlNode& Output, BcU32 Depth);
	void cmdLog(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdResource(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdResourceEdit(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdWADL(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdJsonSerialiser(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdViewFunctions(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	void cmdJson(DsParameters params, BcHtmlNode& Output, std::string PostContent);					

private:
#if USE_WEBBY
	// Variables for Webby
	int							ConnectionCount_;
	WebbyServer *				Server_;
	void*						ServerMemory_;
#ifndef MAX_WSCONN
#define MAX_WSCONN 8
#endif // MAX_WSCONN
	WebbyConnection*			ws_connections[MAX_WSCONN];


	static int					externalWebbyDispatch(WebbyConnection *connection);
	static int					externalWebbyConnect(WebbyConnection *connection);
	static void					externalWebbyConnected(WebbyConnection *connection);
	static void					externalWebbyClosed(WebbyConnection *connection);
	static int					externalWebbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame);

	int							webbyDispatch(WebbyConnection *connection);
	int							webbyConnect(WebbyConnection *connection);
	void						webbyConnected(WebbyConnection *connection);
	void						webbyClosed(WebbyConnection *connection);
	int							webbyFrame(WebbyConnection *connection, const WebbyWsFrame *frame);
#endif // USE_WEBBY
	
protected:
	bool DrawPanels_;
	std::vector< DsPanelDefinition > PanelFunctions_;
	std::vector< DsPageDefinition > PageFunctions_;
	std::vector< DsFunctionDefinition > ButtonFunctions_;
	BcU32 NextHandle_;

};



#endif
