/**************************************************************************
*
* File:		BcHtml.cpp
* Author:	Daniel de Zwart
* Ver/Date:	2014/04/14
* Description:
*		Html Creating system
*
*
*
*
**************************************************************************/

#include "BcHtml.h"

BcHtml::BcHtml() :
RootNode_( "html", 0 )
{
}

BcHtmlNode BcHtml::getRootNode()
{
	return BcHtmlNode( &RootNode_ );
}

std::string BcHtml::getHtml()
{
	return RootNode_.getOuterXml();
}

/**************************************************************************
*
* BcHtmlNode implementation
*
*/
BcHtmlNode::BcHtmlNode( BcHtmlNodeInternal* Node )
: InternalNode_( Node )
{

}

BcHtmlNode::BcHtmlNode( BcHtmlNode& Cpy )
{
	InternalNode_ = Cpy.InternalNode_;
	NextTag_ = Cpy.NextTag_;
}


BcHtmlNode BcHtmlNode::operator[]( BcU32 Idx )
{
	if ( Idx < InternalNode_->Children.size() )
		return BcHtmlNode( InternalNode_->Children[ Idx ] );
	return 0;
}

BcHtmlNode BcHtmlNode::operator[]( std::string Tag )
{
	for ( BcU32 Idx = 0; Idx < InternalNode_->Children.size(); ++Idx )
	{
		if ( InternalNode_->Children[ Idx ]->Tag_ == Tag )
		{
			return BcHtmlNode( InternalNode_->Children[ Idx ] );
		}
	}
	return 0;/**/
}


BcHtmlNode BcHtmlNode::createChildNode( std::string Tag )
{
	BcHtmlNodeInternal* ret = InternalNode_->createChildNode( Tag );
	return BcHtmlNode( ret );
}

std::string BcHtmlNode::getTag()
{
	return InternalNode_->getTag();
}

std::string BcHtmlNode::getContents()
{
	return InternalNode_->getContents();
}

BcHtmlNode& BcHtmlNode::setAttribute( std::string Attr, std::string Value )
{
	InternalNode_->setAttribute( Attr, Value );
	return *this;
}

BcHtmlNode& BcHtmlNode::setTag( std::string Tag )
{
	InternalNode_->setTag( Tag );
	return *this;
}

BcHtmlNode& BcHtmlNode::setContents( std::string Contents )
{
	InternalNode_->setContents( Contents );
	return *this;
}

std::string BcHtmlNode::getOuterXml()
{
	return InternalNode_->getOuterXml();
}

bool BcHtmlNode::operator == ( const BcHtmlNode& V )
{
	return ( V.InternalNode_ == InternalNode_ );
}

BcHtmlNode BcHtmlNode::NextSiblingNode()
{
	if ( InternalNode_->Parent_ == 0 )
		return BcHtmlNode( 0 );
	BcU32 Idx;
	for ( Idx = 0; Idx < InternalNode_->Parent_->Children.size(); ++Idx )
	{
		if ( InternalNode_->Parent_->Children[ Idx ] == InternalNode_ )
		{
			break;
		}
	}
	Idx = Idx + 1;
	for ( ; Idx < InternalNode_->Parent_->Children.size(); ++Idx )
	{
		if ( ( InternalNode_->Parent_->Children[ Idx ]->Tag_ == NextTag_ ) || ( NextTag_ == "" ) )
		{
			BcHtmlNode ret( InternalNode_->Parent_->Children[ Idx ] );
			ret.NextTag_ = NextTag_;
			return ret;
		}
	}
	return BcHtmlNode( 0 );
}


/**************************************************************************
*
* BcHtmlNodeInternal implementation
*
*/

BcHtmlNodeInternal::~BcHtmlNodeInternal()
{
	for ( BcU32 Idx = 0; Idx < Children.size(); ++Idx )
		delete Children[ Idx ];
}

BcHtmlNodeInternal::BcHtmlNodeInternal( std::string Tag, BcHtmlNodeInternal* Parent )
: Tag_( Tag ), Parent_( Parent )
{

}

BcHtmlNodeInternal* BcHtmlNodeInternal::createChildNode( std::string Tag )
{
	Children.push_back( new BcHtmlNodeInternal( Tag, this ) );
	return Children[ Children.size() - 1 ];
}

std::string BcHtmlNodeInternal::getTag()
{
	return Tag_;
}

std::string BcHtmlNodeInternal::getContents()
{
	return Contents_;
}

void BcHtmlNodeInternal::setAttribute( std::string Attr, std::string Value )
{
	Attributes_[ Attr ] = Value;
}

void BcHtmlNodeInternal::setTag( std::string Tag )
{
	Tag_ = Tag;
}

void BcHtmlNodeInternal::setContents( std::string Contents )
{
	Contents_ = Contents;
}

std::string BcHtmlNodeInternal::getOuterXml()
{
	if ( Tag_ == "" )
		return Contents_;
	std::string output = "<" + Tag_ + " ";
	for each ( auto attr in Attributes_ )
	{
		output += attr.first;
		output += "=\"";
		output += attr.second;
		output += "\" ";
	}
	if ( ( Contents_ == "" ) && ( Children.size() == 0 ) &&
		( ( Tag_ == "p" ) || ( Tag_ == "br" ) ) )
	{
		output += ">";
		return output;
	}
	else
	{
		output += ">";
		output += Contents_;
		for each ( BcHtmlNodeInternal* var in Children )
		{
			output += var->getOuterXml();
		}
		output += "</" + Tag_ + ">";
		return output;
	}
	return "";
}
