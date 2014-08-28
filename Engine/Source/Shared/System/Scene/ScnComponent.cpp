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
#include "System/Scene/ScnEntity.h"
#include "System/Content/CsCore.h"
#include "System/Renderer/RsCore.h"

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnComponentImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// ScnComponentAttribute
REFLECTION_DEFINE_DERIVED( ScnComponentAttribute );

void ScnComponentAttribute::StaticRegisterClass()
{
	ReRegisterClass< ScnComponentAttribute, Super >();
}

ScnComponentAttribute::ScnComponentAttribute( BcS32 UpdatePriority ):
	UpdatePriority_( UpdatePriority )
{
}

int ScnComponentAttribute::getUpdatePriority() const
{
	return UpdatePriority_;
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnComponent );

void ScnComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Flags_", &ScnComponent::Flags_, bcRFF_TRANSIENT ),
		new ReField( "ParentEntity_", &ScnComponent::ParentEntity_, bcRFF_SHALLOW_COPY ),
		new ReField( "pJsonObject_", &ScnComponent::pJsonObject_, bcRFF_SHALLOW_COPY )
	};
		
	auto& Class = ReRegisterClass< ScnComponent, Super >( Fields );

#ifdef PSY_SERVER
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( ScnComponentImport::StaticGetClass() ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnComponent::initialise()
{
	Super::initialise();

	Flags_ = 0;
	ParentEntity_ = nullptr;
	pJsonObject_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnComponent::initialise( const Json::Value& Object )
{
	BcBreakpoint; // Should never enter here.
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnComponent::create()
{
	CsResource::markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void ScnComponent::preUpdate( BcF32 Tick )
{
	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnComponent::update( BcF32 Tick )
{
	
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual
void ScnComponent::postUpdate( BcF32 Tick )
{
	
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

	markCreate();
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

	ParentEntity_ = NULL;

	markDestroy();
}

//////////////////////////////////////////////////////////////////////////
// setFlag
void ScnComponent::setFlag( ScnComponentFlags Flag )
{
	Flags_ = Flags_ | Flag;
}

//////////////////////////////////////////////////////////////////////////
// clearFlag
void ScnComponent::clearFlag( ScnComponentFlags Flag )
{
	Flags_ = Flags_ & ~Flag;
}

//////////////////////////////////////////////////////////////////////////
// isFlagSet
BcBool ScnComponent::isFlagSet( ScnComponentFlags Flag ) const
{
	return ( Flags_ & Flag ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnComponent::isAttached() const
{
	return isFlagSet( scnCF_ATTACHED );
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
		pJsonObject_ = reinterpret_cast< const BcChar* >( pData );

		// Initialise json object.
	    Json::Value Root;
	    Json::Reader Reader;
	    if( Reader.parse( pJsonObject_, Root ) )
		{
			initialise( Root );
		}
		else
		{
			BcBreakpoint;
		}

		// Advance to ready stage, we don't need to do anything in the create,
		// nor do we want to.
		CsResource::markCreate();
		CsResource::markReady();
	}
}
