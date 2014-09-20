/**************************************************************************
*
* File:		MdlAnim.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Model anim.
*		
*
*
* 
**************************************************************************/

#ifndef __MDLANIM_H__
#define __MDLANIM_H__

#include "MdlTypes.h"

//////////////////////////////////////////////////////////////////////////
// MdlAnim
class MdlAnim
{
public:
	MdlAnim();
	~MdlAnim();

	void addNode( const MdlAnimNode& Node );

	size_t nNodes() const;
	MdlAnimNode* pNode( size_t iNode );

private:
	std::vector< MdlAnimNode >		NodeList_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline void MdlAnim::addNode( const MdlAnimNode& Node )
{
	NodeList_.push_back( Node );
}

inline size_t MdlAnim::nNodes() const
{
	return NodeList_.size();
}

inline MdlAnimNode* MdlAnim::pNode( size_t iNode )
{
	return &NodeList_[ iNode ];
}

#endif
