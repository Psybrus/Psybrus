/**************************************************************************
*
* File:		DsCore.h
* Author:	Neil Richardson 
* Ver/Date:	11/12/12	
* Description:
*		Debug system core.
*		
*
*
* 
**************************************************************************/

#ifndef __DSCORE_H__
#define __DSCORE_H__

#include "Base/BcGlobal.h"
#include <mutex>

#include "Base/BcHtml.h"
#include "Base/BcRegex.h"
#include "System/SysSystem.h"

#include "System/Content/CsCore.h"
#include "System/Scene/ScnCore.h"

#include <functional>
#include <map>

//////////////////////////////////////////////////////////////////////////
/**	\struct DsCoreMessage
*	\brief Storage struct for functions
*
*	
*/

// typedef const std::map < std::string, std::string>& DsParameters;
typedef const std::vector< std::string>& DsParameters;

struct DsPageDefinition
{
	DsPageDefinition(std::string r, std::string display)
	: Regex_(r.c_str()),
		Text_(r),
		Display_(display),
		Visible_(true),
		IsHtml_(true) 
	{}

	DsPageDefinition(std::string r)
		: Regex_(r.c_str()),
		Text_(r),
		Visible_(false),
		IsHtml_(true)
	{}

	BcRegex Regex_;
	std::string Text_;
	std::string Display_;
	bool Visible_;
	bool IsHtml_;
	std::function <void(DsParameters, BcHtmlNode&, std::string)> Function_;
};

struct DsFunctionDefinition
{
	DsFunctionDefinition(std::string text, std::function<void()> fn)
	:
	DisplayText_(text), Function_(fn)
	{}

	std::string DisplayText_;
	std::function<void()> Function_;
};


//////////////////////////////////////////////////////////////////////////
/**	\class DsCore
*	\brief Debug System Core
*
*	Debugging system core.
*/
class DsCore :
	public BcGlobal< DsCore >,
	public SysSystem
{
public:
	static BcU32 JOB_QUEUE_ID;
	static BcU32 DsCoreSerialised = 0x00008000;
public:
	DsCore();
	virtual ~DsCore();

	virtual void				open() = 0;
	virtual void				update() = 0;
	virtual void				close() = 0;

	static void					cmdMenu(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	static void					cmdContent(DsParameters params, BcHtmlNode& Output, std::string PostContent);

	void						cmdContent_Resource(BcHtmlNode& Output);

	static void					cmdScene(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	static void					cmdScene_Entity(ScnEntityRef Entity, BcHtmlNode& Output, BcU32 Depth);
	static void					cmdScene_Component(ScnComponentRef Entity, BcHtmlNode& Output, BcU32 Depth);
	static void					cmdLog(DsParameters params, BcHtmlNode& Output, std::string PostContent);

	static void					cmdResource(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	static void					cmdResourceEdit(DsParameters params, BcHtmlNode& Output, std::string PostContent);

	static void					cmdWADL(DsParameters params, BcHtmlNode& Output, std::string PostContent);
	static void					cmdJsonSerialiser(DsParameters params, BcHtmlNode& Output, std::string PostContent);

	void						writeHeader(BcHtmlNode& Output);
	void						writeFooter(BcHtmlNode& Output);
	char*						writeFile(std::string filename, int& OutLength, std::string& type);
	void						registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn, std::string display);
	void						registerPage(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn);
	void						registerPageNoHtml(std::string regex, std::function < void(DsParameters, BcHtmlNode&, std::string)> fn);
	void						deregisterPage(std::string regex);

	void						registerFunction(std::string Display, std::function<void()> Function);
	void						deregisterFunction(std::string Display);
	char*						handleFile(std::string Uri, int& FileSize, std::string Content);
	std::string					loadHtmlFile(std::string Uri, std::string Content);

	static void					cmdJson(DsParameters params, BcHtmlNode& Output, std::string PostContent);					
private:
//	BcMutex						Lock_;
	SysFence					GameThreadWaitFence_;
protected:
	std::vector<DsPageDefinition>	PageFunctions_;
	std::vector<DsFunctionDefinition>	ButtonFunctions_;
};


#endif
