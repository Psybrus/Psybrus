/**************************************************************************
*
* File:		ScnComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "Serialisation/SeJsonReader.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnComponentImport.h"
#endif

#include "System/Scene/Animation/ScnAnimationComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnComponent );

void ScnComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ComponentFlags_", &ScnComponent::ComponentFlags_, bcRFF_TRANSIENT ),
		new ReField( "ParentEntity_", &ScnComponent::ParentEntity_, bcRFF_SHALLOW_COPY | bcRFF_CONST ),
	};
	
	auto& Class = ReRegisterClass< ScnComponent, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnComponentImport::StaticGetClass(), 1, 100 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponent::ScnComponent():
	ComponentFlags_( 0 ),
	ParentEntity_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponent::ScnComponent( ReNoInit ):
	ComponentFlags_( 0 ),
	ParentEntity_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnComponent::~ScnComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// postInitialise
void ScnComponent::postInitialise()
{
	// TODO: Move this into initialise when we move initialise to constructors.
	CsResource::markCreate();
	CsResource::markReady();
}

//////////////////////////////////////////////////////////////////////////
// visitHierarchy
//virtual
void ScnComponent::visitHierarchy( 
		ScnComponentVisitType VisitType, 
		ScnEntity* Parent,
		const ScnComponentVisitFunc& Func )
{
	Func( this, Parent );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( !isFlagSet( scnCF_ATTACHED ), "Attempting to attach component when it's already attached!" );
	BcAssertMsg( isFlagSet( scnCF_PENDING_ATTACH ), "ScnComponent: Not pending attach!" );

	clearFlag( scnCF_PENDING_ATTACH );
	setFlag( scnCF_ATTACHED );

	ParentEntity_ = Parent;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnComponent::onDetach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( ParentEntity_ == Parent, "Attempting to detach component from an entity it isn't attached to!" );
	BcAssertMsg( isFlagSet( scnCF_ATTACHED ), "ScnComponent: Not attached!" );
	BcAssertMsg( isFlagSet( scnCF_PENDING_DETACH ), "ScnComponent: Not pending detach!" );

	clearFlag( scnCF_PENDING_DETACH );
	clearFlag( scnCF_ATTACHED );
	setFlag( scnCF_PENDING_DESTROY );
	ParentEntity_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// setFlag
void ScnComponent::setFlag( ScnComponentFlags Flag )
{
	ComponentFlags_ = ComponentFlags_ | Flag;
}

//////////////////////////////////////////////////////////////////////////
// clearFlag
void ScnComponent::clearFlag( ScnComponentFlags Flag )
{
	ComponentFlags_ = ComponentFlags_ & ~Flag;
}

//////////////////////////////////////////////////////////////////////////
// isFlagSet
BcBool ScnComponent::isFlagSet( ScnComponentFlags Flag ) const
{
	return ( ComponentFlags_ & Flag ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnComponent::isAttached() const
{
	return isFlagSet( scnCF_ATTACHED ) && !isFlagSet( scnCF_PENDING_DETACH );
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnComponent::isAttached( ScnEntityWeakRef Parent ) const
{
	return isAttached() && ParentEntity_ == Parent;
}

//////////////////////////////////////////////////////////////////////////
// setParentEntity
void ScnComponent::setParentEntity( ScnEntityWeakRef Entity )
{
	BcAssertMsg( !isFlagSet( scnCF_ATTACHED ), "ScnComponent: Already attached, can't reassign parent entity."  );
	BcAssertMsg( !isFlagSet( scnCF_PENDING_ATTACH ), "ScnComponent: Currently attaching, can't reassign parent entity."  );
	BcAssertMsg( !isFlagSet( scnCF_PENDING_DETACH ), "ScnComponent: Currently detaching, can't reassign parent entity."  );
	BcAssertMsg( !isFlagSet( scnCF_PENDING_DESTROY ), "ScnComponent: Currently being destroyed, can't reassign parent entity."  );
	BcAssertMsg( ParentEntity_ == nullptr, "ScnComponent: Already have a parent, can't reassign parent entity."  );
	ParentEntity_ = Entity;
}

//////////////////////////////////////////////////////////////////////////
// getParentEntity
ScnEntity* ScnComponent::getParentEntity()
{
	return ParentEntity_;
}

const ScnEntity* ScnComponent::getParentEntity() const
{
	return ParentEntity_;
}

//////////////////////////////////////////////////////////////////////////
// getFullName
std::string ScnComponent::getFullName()
{
	std::string FullName;

	ScnEntityWeakRef Parent( getParentEntity() );

	if( Parent.isValid() )
	{
		FullName += Parent->getFullName() + ".";
	}

	FullName += (*getName());

	return FullName;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnComponent::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnComponent::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "object" ) )
	{
		auto JsonObject = reinterpret_cast< const BcChar* >( pData );

		// Initialise json object.
		Json::Value Root;
		Json::Reader Reader;
		if( Reader.parse( JsonObject, Root ) )
		{
			// Serialise from json.
			// TODO: Serialise from binary.
			CsSerialiserPackageObjectCodec ObjectCodec( getPackage(), bcRFF_IMPORTER, bcRFF_NONE, bcRFF_IMPORTER );
			SeJsonReader Reader( &ObjectCodec );
			Reader.setRootValue( Root );
			Reader.serialise( this, getClass() );
			
			// Now reinitialise.
			initialise();
		}
		else
		{
			BcBreakpoint;
		}

		CsResource::markCreate();
		CsResource::markReady();
	}
}


//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponent* ScnComponent::getComponent( size_t Idx, const ReClass* Class )
{
	BcAssert( ParentEntity_ != nullptr );
	return ParentEntity_->getComponent( Idx, Class );
}

//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponent* ScnComponent::getComponent( BcName Name, const ReClass* Class )
{
	BcAssert( ParentEntity_ != nullptr );
	return ParentEntity_->getComponent( Name, Class );
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnComponent::getComponentAnyParent( size_t Idx, const ReClass* Class )
{
	BcAssert( ParentEntity_ != nullptr );
	return ParentEntity_->getComponentAnyParent( Idx, Class );
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnComponent::getComponentAnyParent( BcName Name, const ReClass* Class )
{
	BcAssert( ParentEntity_ != nullptr );
	return ParentEntity_->getComponentAnyParent( Name, Class );
}
