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

#include <map>
#include <vector>
#include <string>
class BcHtml;
class BcHtmlNodeInternal;
//////////////////////////////////////////////////////////////////////////
// BcHtmlNode
class BcHtmlNode
{
	friend BcHtml;
public:
	BcHtmlNode createChildNode( std::string Tag );
	std::string getTag();
	std::string getContents();

	BcHtmlNode& setTag( std::string Tag );
	BcHtmlNode& setContents( std::string Contents );
	BcHtmlNode& setAttribute( std::string Attr, std::string Value );
	BcHtmlNode operator[]( BcU32 Idx );
	BcHtmlNode operator[]( std::string Tag );
	std::string getOuterXml();
	bool operator==( const BcHtmlNode& V );
	BcHtmlNode NextSiblingNode();
	BcHtmlNode findNodeById( std::string Id );
private:
	BcHtmlNode( BcHtmlNodeInternal* Node );
	BcHtmlNodeInternal* InternalNode_;
	std::string NextTag_;
};

//////////////////////////////////////////////////////////////////////////
// BcHtmlNodeInternal
class BcHtmlNodeInternal
{
	friend BcHtmlNode;
	friend BcHtml;
public:
	BcHtmlNodeInternal* createChildNode( std::string Tag );
	std::string getTag();
	std::string getContents();

	void setTag( std::string Tag );
	void setContents( std::string Contents );
	void setAttribute( std::string Attr, std::string Value );
	BcHtmlNodeInternal* operator[]( BcU32 Idx );
	BcHtmlNodeInternal* operator[]( std::string Tag );
	std::string getOuterXml();
	BcHtmlNodeInternal* findNodeById( std::string Id );

	~BcHtmlNodeInternal();
private:
	BcHtmlNodeInternal( std::string Tag, BcHtmlNodeInternal* Parent );
	std::map< std::string, std::string > Attributes_;
	std::string Tag_;
	std::vector< BcHtmlNodeInternal* > Children;
	std::string Contents_;
	BcHtmlNodeInternal* Parent_;
};


//////////////////////////////////////////////////////////////////////////
// BcHtml
class BcHtml
{
public:
	BcHtml();
	BcHtmlNode getRootNode();
	std::string getHtml();
	BcHtml( std::string Template );
private:
	/**
	* Private write using va_list.
	*/
	BcHtmlNodeInternal RootNode_;
private:

};


#endif
