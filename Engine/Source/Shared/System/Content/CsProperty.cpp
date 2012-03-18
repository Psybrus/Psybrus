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

#include "System/Content/CsProperty.h"

//////////////////////////////////////////////////////////////////////////
// CsPropertyCatagory
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyCatagory::CsPropertyCatagory()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyCatagory::CsPropertyCatagory( const std::string& Name ):
	Name_( Name )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyCatagory::CsPropertyCatagory( const CsPropertyCatagory& Other ):
	Name_( Other.Name_ )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPropertyCatagory::~CsPropertyCatagory()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getName
const std::string& CsPropertyCatagory::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// CsPropertyField
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField():
	CatagoryIdx_( BcErrorCode ),
	ValueType_( csPVT_NULL ),
	ContainerType_( csPCT_NULL )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField( BcU32 CatagoryIdx, const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType, const std::string& AdditionalInfo ):
	CatagoryIdx_( CatagoryIdx ),
	Name_( Name ),
	ValueType_( ValueType ),
	ContainerType_( ContainerType ),
	AdditionalInfo_( AdditionalInfo )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyField::CsPropertyField( const CsPropertyField& Other ):
	CatagoryIdx_( Other.CatagoryIdx_ ),
	Name_( Other.Name_ ),
	ValueType_( Other.ValueType_ ),
	ContainerType_( Other.ContainerType_ ),
	AdditionalInfo_( Other.AdditionalInfo_ )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPropertyField::~CsPropertyField()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getCatagoryIdx
BcU32 CsPropertyField::getCatagoryIdx() const
{
	return CatagoryIdx_;
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
// getAdditionalInfo
const std::string& CsPropertyField::getAdditionalInfo() const
{
	return AdditionalInfo_;
}
	

//////////////////////////////////////////////////////////////////////////
// CsPropertyTable
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPropertyTable::CsPropertyTable():
	BeganCatagory_( BcFalse )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPropertyTable::~CsPropertyTable()
{
	
}

//////////////////////////////////////////////////////////////////////////
// beginCatagory
CsPropertyTable& CsPropertyTable::beginCatagory( const std::string& Catagory )
{
	BcAssertMsg( BeganCatagory_ == BcFalse, "CsPropertyTable: Did you forget to call endCatagory?" );
	BeganCatagory_ = BcTrue;
	CatagoryList_.push_back( CsPropertyCatagory( Catagory ) );
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// endCatagory
CsPropertyTable& CsPropertyTable::endCatagory()
{
	BcAssertMsg( BeganCatagory_ == BcTrue, "CsPropertyTable: Did you forget to call beginCatagory?" );
	BeganCatagory_ = BcFalse;
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// field
CsPropertyTable& CsPropertyTable::field( const std::string& Name, CsPropertyValueType ValueType, CsPropertyContainerType ContainerType, const std::string& AdditionalInfo )
{
	BcAssertMsg( BeganCatagory_ == BcTrue, "CsPropertyTable: Did you forget to call beginCatagory?" );
	BcAssertMsg( CatagoryList_.size() > 0, "CsPropertyTable: No catagories." );
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
	FieldList_.push_back( CsPropertyField( ( CatagoryList_.size() - 1 ), Name, ValueType, ContainerType, AdditionalInfo ) );

	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// getCatagoryCount
BcU32 CsPropertyTable::getCatagoryCount() const
{
	return CatagoryList_.size();
}

//////////////////////////////////////////////////////////////////////////
// getCatagory
const CsPropertyCatagory& CsPropertyTable::getCatagory( BcU32 CatagoryIdx ) const
{
	return CatagoryList_[ CatagoryIdx ];
}

//////////////////////////////////////////////////////////////////////////
// getFieldCount
BcU32 CsPropertyTable::getFieldCount() const
{
	return FieldList_.size();
}

//////////////////////////////////////////////////////////////////////////
// field
const CsPropertyField& CsPropertyTable::getField( BcU32 FieldIdx ) const
{
	return FieldList_[ FieldIdx ];
}
