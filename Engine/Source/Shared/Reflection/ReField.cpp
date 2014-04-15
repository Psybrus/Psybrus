#include "Reflection/ReField.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( Field );

void Field::StaticRegisterClass()
{
	static const Field Fields[] = 
	{
		Field( "Offset_", &Field::Offset_ ),
		Field( "Type_", &Field::Type_ ),
		Field( "Flags_", &Field::Flags_ ),
		Field( "KeyType_", &Field::KeyType_ ),
		Field( "ValueType_", &Field::ValueType_ ),
		Field( "KeyFlags_", &Field::KeyFlags_ ),
		Field( "ValueFlags_", &Field::ValueFlags_ ),
	};
		
	RegisterClass< Field, Primitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Field
Field::Field():
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
void Field::setType( const Type* Type )
{
	Type_ = Type;
}
	
//////////////////////////////////////////////////////////////////////////
// getType
const Type* Field::getType() const
{
	return Type_;
}
	
//////////////////////////////////////////////////////////////////////////
// setOffset
void Field::setOffset( BcU32 Offset )
{
	Offset_ = Offset;
}
	
//////////////////////////////////////////////////////////////////////////
// getOffset
BcU32 Field::getOffset() const
{
	return Offset_;
}
	
//////////////////////////////////////////////////////////////////////////
// setFlags
void Field::setFlags( BcU32 Flags )
{
	Flags_ = Flags;
}
	
//////////////////////////////////////////////////////////////////////////
// getFlags
BcU32 Field::getFlags() const
{
	return Flags_;
}

//////////////////////////////////////////////////////////////////////////
// isContainer
bool Field::isContainer() const
{
	return ContainerAccessor_ != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// isPointerType
bool Field::isPointerType() const
{
	return ( getFlags() & bcRFF_ANY_POINTER_TYPE ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// getKeyType
const Type* Field::getKeyType() const
{
	return KeyType_;
}

//////////////////////////////////////////////////////////////////////////
// getValueType
const Type* Field::getValueType() const
{
	return ValueType_;
}

//////////////////////////////////////////////////////////////////////////
// getKeyFlags
BcU32 Field::getKeyFlags() const
{
	return KeyFlags_;
}

//////////////////////////////////////////////////////////////////////////
// getValueFlags
BcU32 Field::getValueFlags() const
{
	return ValueFlags_;
}

//////////////////////////////////////////////////////////////////////////
// newWriteIterator
ContainerAccessor::WriteIterator* Field::newWriteIterator( void* pContainerData ) const
{
	if( ContainerAccessor_ != nullptr )
	{
		return ContainerAccessor_->newWriteIterator( pContainerData );
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// newReadIterator
ContainerAccessor::ReadIterator* Field::newReadIterator( void* pContainerData ) const
{
	if( ContainerAccessor_ != nullptr )
	{
		return ContainerAccessor_->newReadIterator( pContainerData );
	}
	return nullptr;
}
