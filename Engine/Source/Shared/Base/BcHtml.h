/**************************************************************************
*
* File:		BcHtml.h
* Author: 	Daniel de Zwart
* Ver/Date:
* Description:
*		Html Creating system
*
*
*
*
**************************************************************************/

#ifndef __BCHTML_H__
#define __BCHTML_H__

#include "Base/BcTypes.h"

class BcHtml;

//////////////////////////////////////////////////////////////////////////
// BcHtmlNode
class BcHtmlNode
{
	friend BcHtml;
public:
	BcHtmlNode& createChildNode(std::string tag);
	std::string getTag();
	std::string getContents();

	void setTag(std::string tag);
	void setContents(std::string contents);
	void setAttribute(std::string attr, std::string value);
	BcHtmlNode* operator[](BcU32 idx);
	BcHtmlNode* operator[](std::string tag);
	std::string getOuterXml();
private:
	BcHtmlNode(std::string tag);
	std::map<std::string, std::string> Attributes_;
	std::string Tag_;
	std::vector<BcHtmlNode> Children;
	std::string Contents_;
};


//////////////////////////////////////////////////////////////////////////
// BcHtml
class BcHtml
{
public:
	BcHtml();
	BcHtmlNode& getRootNode();
	std::string getHtml();
private:
	/**
	* Private write using va_list.
	*/
	BcHtmlNode RootNode_;
private:

};


#endif
