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

#include "ScnComponent.h"
#include "RsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnComponent::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	/*
	const std::string& FileName = Object[ "source" ].asString();

	// Add root dependancy.
	DependancyList.push_back( FileName );

	// Load texture from file and create the data for export.
	ImgImage* pImage = Img::load( FileName.c_str() );
	
	if( pImage != NULL )
	{
		BcStream HeaderStream;
		BcStream BodyStream( BcFalse, 1024, ( pImage->width() * pImage->height() * 4 ) );
		
		// TODO: Use parameters to pick a format.
		THeader Header = { pImage->width(), pImage->height(), 1, rsTF_RGBA8 };
		HeaderStream << Header;
		
		// Write body.				
		for( BcU32 Y = 0; Y < pImage->height(); ++Y )
		{
			for( BcU32 X = 0; X < pImage->width(); ++X )
			{
				ImgColour Colour = pImage->getPixel( X, Y );
				
				BodyStream << Colour.R_;
				BodyStream << Colour.G_;
				BodyStream << Colour.B_;
				BodyStream << Colour.A_;
			}
		}
		
		// Delete image.
		delete pImage;
		
		// Add chunks and finish up.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );
				
		//
		return BcTrue;
	}
	*/
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnComponent" )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnComponent::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
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
// isReady
//virtual
BcBool ScnComponent::isReady()
{
	return pHeader_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnComponent::update( BcReal Tick )
{
	
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( Parent_.isValid() == BcFalse, "Attempting to attach component when it's already attached!" );

	Parent_ = Parent;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnComponent::onDetach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( Parent_.isValid() == BcTrue, "Attempting to detach component that is already detached!" );
	BcAssertMsg( Parent_ == Parent, "Attempting to detach component from an entity it isn't attached to!" );

	Parent_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnComponent::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnComponent::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
	}
}
