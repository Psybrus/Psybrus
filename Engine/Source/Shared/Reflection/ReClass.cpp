#include "Reflection/ReClass.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReClass );

void ReClass::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Super_",		&ReClass::Super_ ),
		new ReField( "Fields_",		&ReClass::Fields_ ),
	};
		
	ReRegisterClass< ReClass, ReType >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReClass::ReClass():
	Super_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReClass::ReClass( BcName Name ):
	Super_( nullptr )
{
	setName( Name );
}
	
//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ReClass::~ReClass()
{
	for( ReField* Field : Fields_ )
	{
		delete Field;
	}
	Fields_.clear();
}

//////////////////////////////////////////////////////////////////////////
// setSuper
void ReClass::setSuper( const ReClass* Super )
{
	Super_ = Super;
}
	
//////////////////////////////////////////////////////////////////////////
// getSuper
const ReClass* ReClass::getSuper() const
{
	return Super_;
}
	
//////////////////////////////////////////////////////////////////////////
// hasBaseClass
BcBool ReClass::hasBaseClass( const ReClass* pClass ) const
{
	if( this == pClass )
	{
		return true;
	}
	else
	{
		const ReClass* pSuperClass = getSuper();
		if( pSuperClass != nullptr )
		{
			return pSuperClass->hasBaseClass( pClass );
		}
	}
	return false;
}
	
//////////////////////////////////////////////////////////////////////////
// setFields
void ReClass::setFields( ReFieldVector&& Fields )
{
	BcAssertMsg( Fields_.size() == 0, "Fields already set." );
	Fields_ = std::move( Fields );
}
	
//////////////////////////////////////////////////////////////////////////
// getField
const ReField* ReClass::getField( size_t Idx ) const
{
	return Fields_[ Idx ];
}
	
//////////////////////////////////////////////////////////////////////////
// getNoofFields
size_t ReClass::getNoofFields() const
{
	return Fields_.size();
}

//////////////////////////////////////////////////////////////////////////
// getFields
const ReFieldVector& ReClass::getFields() const
{
	return Fields_;
}

//////////////////////////////////////////////////////////////////////////
// validate
BcBool ReClass::validate() const
{
	BcBool RetVal = BcTrue;
	const ReClass* OtherClass = Super_;
	while( OtherClass != nullptr )
	{
		const auto& FieldsB = OtherClass->getFields();

		for( const auto& FieldA : Fields_ )
		{
			for( const auto& FieldB : FieldsB )
			{
				if( FieldA->getName() == FieldB->getName() )
				{
					BcPrintf( "ERROR: ReClass \"%s\" has field \"%s\" in its super \"%s\"\n",
						(*getName()).c_str(), 
						(*FieldA->getName()).c_str(),
						(*OtherClass->getName()).c_str() );
					RetVal = BcFalse;
				}
			}
		}

		OtherClass = OtherClass->getSuper();
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ReClass::destroy( void* pData ) const
{
	BcAssertMsg( Serialiser_, "No serialiser for class \"%s\"", (*getName()).c_str() );
	Serialiser_->destroy( pData );
}
