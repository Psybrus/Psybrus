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

BcHtmlNode BcHtml::getRootNode()
{
	return BcHtmlNode(&RootNode_);
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
BcHtmlNode::BcHtmlNode(BcHtmlNodeInternal* node)
: InternalNode_(node)
{

}

BcHtmlNode::BcHtmlNode(BcHtmlNode& cpy)
{
	InternalNode_ = cpy.InternalNode_;
}


BcHtmlNode BcHtmlNode::operator[](BcU32 idx)
{
	if (idx < InternalNode_->Children.size())
		return BcHtmlNode(&InternalNode_->Children[idx]);
	return 0;
}

BcHtmlNode BcHtmlNode::operator[](std::string tag)
{
	for (BcU32 Idx = 0; Idx < InternalNode_->Children.size(); ++Idx)
	{
		if (InternalNode_->Children[Idx].Tag_ == tag)
		{
			return BcHtmlNode(&InternalNode_->Children[Idx]);
		}
	}
	return 0;/**/
}


BcHtmlNode BcHtmlNode::createChildNode(std::string tag)
{
	BcHtmlNodeInternal* ret = InternalNode_->createChildNode(tag);
	return BcHtmlNode(ret);
}

std::string BcHtmlNode::getTag()
{
	return InternalNode_->getTag();
}

std::string BcHtmlNode::getContents()
{
	return InternalNode_->getContents();
}

void BcHtmlNode::setAttribute(std::string attr, std::string value)
{
	InternalNode_->setAttribute(attr, value);
}

void BcHtmlNode::setTag(std::string tag)
{
	InternalNode_->setTag(tag);
}

void BcHtmlNode::setContents(std::string contents)
{
	InternalNode_->setContents(contents);
}

std::string BcHtmlNode::getOuterXml()
{
	return InternalNode_->getOuterXml();
}

bool BcHtmlNode::operator = (const int&v)
{
	return (v == (int)InternalNode_);
}




/**************************************************************************
*
* BcHtmlNodeInternal implementation
*
*/
BcHtmlNodeInternal::BcHtmlNodeInternal(std::string tag)
: Tag_(tag)
{

}

BcHtmlNodeInternal* BcHtmlNodeInternal::createChildNode(std::string tag)
{
	Children.push_back(BcHtmlNodeInternal(tag));
	return &Children[Children.size() - 1];
}

std::string BcHtmlNodeInternal::getTag()
{
	return Tag_;
}

std::string BcHtmlNodeInternal::getContents()
{
	return Contents_;
}

void BcHtmlNodeInternal::setAttribute(std::string attr, std::string value)
{
	Attributes_[attr] = value;
}

void BcHtmlNodeInternal::setTag(std::string tag)
{
	Tag_ = tag;
}

void BcHtmlNodeInternal::setContents(std::string contents)
{
	Contents_ = contents;
}

std::string BcHtmlNodeInternal::getOuterXml()
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
	if ((Contents_ == "") && (Children.size() == 0) &&
		((Tag_ == "p") || (Tag_ == "br")))
	{
		return output + "/>";
	}
	else
	{


		output += ">";
		output += Contents_;
		for each (BcHtmlNodeInternal var in Children)
		{
			output += var.getOuterXml();
		}
		output += "</" + Tag_ + ">";
		return output;
	}
	return "";
}
