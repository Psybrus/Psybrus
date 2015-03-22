#include "Reflection/ReField.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReField );

void ReField::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Offset_", &ReField::Offset_ ),
		new ReField( "Type_", &ReField::Type_ ),
		new ReField( "FieldFlags_", &ReField::FieldFlags_ ),
		new ReField( "KeyType_", &ReField::KeyType_ ),
		new ReField( "ValueType_", &ReField::ValueType_ ),
		new ReField( "KeyFlags_", &ReField::KeyFlags_ ),
		new ReField( "ValueFlags_", &ReField::ValueFlags_ ),
	};
		
	ReRegisterClass< ReField, RePrimitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Field
ReField::ReField():
	Offset_( 0 ),
	Type_( nullptr ),
	Size_( 0 ),
	FieldFlags_( 0 ),
	KeyType_( nullptr ),
	ValueType_( nullptr ),
	KeyFlags_( 0 ),
	ValueFlags_( 0 )
{
	
}
	
//////////////////////////////////////////////////////////////////////////
// setType
void ReField::setType( const ReClass* Type )
{
	Type_ = Type;
}
	
//////////////////////////////////////////////////////////////////////////
// getType
const ReClass* ReField::getType() const
{
	return Type_;
}
	
//////////////////////////////////////////////////////////////////////////
// setSize
void ReField::setSize( BcSize Size )
{
	Size_ = Size;
}

//////////////////////////////////////////////////////////////////////////
// getSize
BcSize ReField::getSize() const
{
	return Size_;
}

//////////////////////////////////////////////////////////////////////////
// setOffset
void ReField::setOffset( BcSize Offset )
{
	Offset_ = Offset;
}
	
//////////////////////////////////////////////////////////////////////////
// getOffset
BcSize ReField::getOffset() const
{
	return Offset_;
}
	
//////////////////////////////////////////////////////////////////////////
// setFlags
void ReField::setFlags( BcU32 Flags )
{
	FieldFlags_ = Flags;
}
	
//////////////////////////////////////////////////////////////////////////
// getFlags
BcU32 ReField::getFlags() const
{
	return FieldFlags_;
}

//////////////////////////////////////////////////////////////////////////
// isContainer
bool ReField::isContainer() const
{
	return ContainerAccessor_ != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// isPointerType
bool ReField::isPointerType() const
{
	return ( getFlags() & bcRFF_ANY_POINTER_TYPE ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// getKeyType
const ReClass* ReField::getKeyType() const
{
	return KeyType_;
}

//////////////////////////////////////////////////////////////////////////
// getValueType
const ReClass* ReField::getValueType() const
{
	return ValueType_;
}

//////////////////////////////////////////////////////////////////////////
// getKeyFlags
BcU32 ReField::getKeyFlags() const
{
	return KeyFlags_;
}

//////////////////////////////////////////////////////////////////////////
// getValueFlags
BcU32 ReField::getValueFlags() const
{
	return ValueFlags_;
}

//////////////////////////////////////////////////////////////////////////
// newWriteIterator
ReContainerAccessor::WriteIterator* ReField::newWriteIterator( void* pContainerData ) const
{
	if( ContainerAccessor_ != nullptr )
	{
		return ContainerAccessor_->newWriteIterator( pContainerData );
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// newReadIterator
ReContainerAccessor::ReadIterator* ReField::newReadIterator( void* pContainerData ) const
{
	if( ContainerAccessor_ != nullptr )
	{
		return ContainerAccessor_->newReadIterator( pContainerData );
	}
	return nullptr;
}
