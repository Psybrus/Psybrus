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

	BcU32 nNodes() const;
	MdlAnimNode* pNode( BcU32 iNode );

private:
	std::vector< MdlAnimNode >		NodeList_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline void MdlAnim::addNode( const MdlAnimNode& Node )
{
	NodeList_.push_back( Node );
}

inline BcU32 MdlAnim::nNodes() const
{
	return (BcU32)NodeList_.size();
}

inline MdlAnimNode* MdlAnim::pNode( BcU32 iNode )
{
	return &NodeList_[ iNode ];
}

#endif
