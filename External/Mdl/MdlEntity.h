/**************************************************************************
*
* File:		MdlEntity.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Static mesh container.
*		
*
*
* 
**************************************************************************/

#ifndef __MdlEntity_H__
#define __MdlEntity_H__

#include "MdlTypes.h"

#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MdlNode;

//////////////////////////////////////////////////////////////////////////
// MdlEntity
class MdlEntity
{
public:
	MdlEntity();
	~MdlEntity();

	void addProp( const MdlEntityProp& Property );
	const MdlEntityProp& prop( BcU32 iProp );
	BcU32 nProps() const;

	void pNode( MdlNode* pNode ); 
	MdlNode* pNode() const;

private:
	MdlNode*				pNode_;

	MdlEntityPropArray		aProperties_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline void MdlEntity::addProp( const MdlEntityProp& Property )
{
	aProperties_.push_back( Property );
}

inline const MdlEntityProp& MdlEntity::prop( BcU32 iProp )
{
	return aProperties_[ iProp ];
}

inline BcU32 MdlEntity::nProps() const
{
	return (BcU32)aProperties_.size();
}

inline void MdlEntity::pNode( MdlNode* pNode )
{
	pNode_ = pNode;
}

inline MdlNode* MdlEntity::pNode() const
{
	return pNode_;
}

#endif
