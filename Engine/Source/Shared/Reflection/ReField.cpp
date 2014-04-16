#include "Reflection/ReField.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReField );

void ReField::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Offset_", &ReField::Offset_ ),
		ReField( "Type_", &ReField::Type_ ),
		ReField( "Flags_", &ReField::Flags_ ),
		ReField( "KeyType_", &ReField::KeyType_ ),
		ReField( "ValueType_", &ReField::ValueType_ ),
		ReField( "KeyFlags_", &ReField::KeyFlags_ ),
		ReField( "ValueFlags_", &ReField::ValueFlags_ ),
	};
		
	ReRegisterClass< ReField, RePrimitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Field
ReField::ReField():
	Offset_( 0 ),
	Type_( nullptr ),
	Flags_( 0 ),
	KeyType_( nullptr ),
	ValueType_( nullptr ),
	KeyFlags_( 0 ),
	ValueFlags_( 0 )
{
	
}
	
//////////////////////////////////////////////////////////////////////////
// setType
void ReField::setType( const Type* Type )
{
	Type_ = Type;
}
	
//////////////////////////////////////////////////////////////////////////
// getType
const ReType* ReField::getType() const
{
	return Type_;
}
	
//////////////////////////////////////////////////////////////////////////
// setOffset
void ReField::setOffset( BcU32 Offset )
{
	Offset_ = Offset;
}
	
//////////////////////////////////////////////////////////////////////////
// getOffset
BcU32 ReField::getOffset() const
{
	return Offset_;
}
	
//////////////////////////////////////////////////////////////////////////
// setFlags
void ReField::setFlags( BcU32 Flags )
{
	Flags_ = Flags;
}
	
//////////////////////////////////////////////////////////////////////////
// getFlags
BcU32 ReField::getFlags() const
{
	return Flags_;
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
const ReType* ReField::getKeyType() const
{
	return KeyType_;
}

//////////////////////////////////////////////////////////////////////////
// getValueType
const ReType* ReField::getValueType() const
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
