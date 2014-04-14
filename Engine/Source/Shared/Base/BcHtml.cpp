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
RootNode_("html")
{
}

BcHtmlNode& BcHtml::getRootNode()
{
	return RootNode_;
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
BcHtmlNode::BcHtmlNode(std::string tag)
: Tag_(tag)
{

}

BcHtmlNode* BcHtmlNode::operator[](BcU32 idx)
{
	if (idx < Children.size())
		return &Children[idx];
	return 0;
}

BcHtmlNode* BcHtmlNode::operator[](std::string tag)
{
	for (BcU32 Idx = 0; Idx < Children.size(); ++Idx)
	{
		if (Children[Idx].Tag_ == tag)
		{
			return &Children[Idx];
		}
	}
	return 0;
}


BcHtmlNode& BcHtmlNode::createChildNode(std::string tag)
{
	Children.push_back(BcHtmlNode(tag));
	return Children[Children.size() - 1];
}

std::string BcHtmlNode::getTag()
{
	return Tag_;
}

std::string BcHtmlNode::getContents()
{
	return Contents_;
}

void BcHtmlNode::setAttribute(std::string attr, std::string value)
{
	Attributes_[attr] = value;
}

void BcHtmlNode::setTag(std::string tag)
{
	Tag_ = tag;
}

void BcHtmlNode::setContents(std::string contents)
{
	Contents_ = contents;
}

std::string BcHtmlNode::getOuterXml()
{
	if (Tag_ == "")
		return Contents_;
	std::string output = "<" + Tag_ + " ";
	for each (auto attr in Attributes_)
	{
		output += attr.first;
		output += "=\"";
		output += attr.second;
		output += "\" ";
	}
	if ((Contents_ == "") && (Children.size() == 0))
	{
		return output + "/>";
	}
	else
	{
		

		output += ">";
		output += Contents_;
		for each (BcHtmlNode var in Children)
		{
			output += var.getOuterXml();
		}
		output += "</" + Tag_ + ">";
		return output;
	}
	return "";
}
