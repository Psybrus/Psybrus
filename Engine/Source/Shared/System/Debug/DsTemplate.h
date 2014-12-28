#ifndef __DSTEMPLATE_H__
#define __DSTEMPLATE_H__

#include "Base/BcHtml.h"

//////////////////////////////////////////////////////////////////////////
/**	\class DsTemplate
*	\brief Debug System Core
*
*	Templates for nicely outputting for the debug core.
*/
class DsTemplate
{
public:
	static BcHtmlNode loadTemplate( BcHtmlNode node, std::string filename );
	
private:
	DsTemplate();
	virtual ~DsTemplate();
	static char* loadTemplateFile( std::string TemplateName );
	static BcHtmlNode loadNode( BcHtmlNode node, void* xmlNode );
};


#endif
