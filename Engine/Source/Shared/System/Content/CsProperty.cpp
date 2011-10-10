/**************************************************************************
*
* File:		CsProperty.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "CsProperty.h"

//////////////////////////////////////////////////////////////////////////
// CsPropertyField
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField():
	ValueType_( csPVT_NULL ),
	ContainerType_( csPCT_NULL )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType ):
	Name_( Name ),
	ValueType_( ValueType ),
	ContainerType_( ContainerType )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField( const CsPropertyField& Other ):
	Name_( Other.Name_ ),
	ValueType_( Other.ValueType_ ),
	ContainerType_( Other.ContainerType_ )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPropertyField::~CsPropertyField()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getName
const std::string& CsPropertyField::getName() const
{
	return Name_;
}
	
//////////////////////////////////////////////////////////////////////////
// getValueType
CsPropertyValueType CsPropertyField::getValueType() const
{
	return ValueType_;
}

//////////////////////////////////////////////////////////////////////////
// getContainerType
CsPropertyContainerType CsPropertyField::getContainerType() const
{
	return ContainerType_;
}

//////////////////////////////////////////////////////////////////////////
// CsPropertyTable
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyTable::CsPropertyTable()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPropertyTable::~CsPropertyTable()
{
	
}

//////////////////////////////////////////////////////////////////////////
// begin
CsPropertyTable& CsPropertyTable::begin()
{
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// end
void CsPropertyTable::end()
{
	
}

//////////////////////////////////////////////////////////////////////////
// field
CsPropertyTable& CsPropertyTable::field( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType )
{
	BcVerifyMsg( ValueType != csPVT_NULL, "CsPropertyTable: Trying to add a null value type." );
	BcVerifyMsg( ContainerType != csPCT_NULL, "CsPropertyTable: Trying to add a null container type." );

	// Find if the field already exists, and remove it.
	for( CsPropertyFieldListIterator It( FieldList_.begin() ); It != FieldList_.end(); ++It )
	{
		const CsPropertyField& Field = (*It);
		
		if( Field.getName() == Name )
		{
			FieldList_.erase( It );
			break;
		}
	}
	
	// Add to list!
	FieldList_.push_back( CsPropertyField( Name, ValueType, ContainerType ) );

	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// fieldCount
BcU32 CsPropertyTable::fieldCount() const
{
	return FieldList_.size();
}

//////////////////////////////////////////////////////////////////////////
// field
const CsPropertyField& CsPropertyTable::getField( BcU32 FieldIdx ) const
{
	return FieldList_[ FieldIdx ];
}
