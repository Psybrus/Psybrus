/**************************************************************************
*
* File:		ScnFont.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnFont.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnFontImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFont );

void ScnFont::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "CharCodeMap_",	&ScnFont::CharCodeMap_ ),
		new ReField( "Texture_",		&ScnFont::Texture_ ),
	};
		
	auto& Class = ReRegisterClass< ScnFont, Super >( Fields );

#ifdef PSY_SERVER
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( ScnFontImport::StaticGetClass() ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnFont::initialise()
{
	pHeader_ = nullptr;
	pGlyphDescs_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnFont::create()
{
	// Request texture.
	Texture_ = getPackage()->getPackageCrossRef( pHeader_->TextureRef_ );

	// Create a char code map.
	for( BcU32 Idx = 0; Idx < pHeader_->NoofGlyphs_; ++Idx )
	{
		ScnFontGlyphDesc* pGlyph = &pGlyphDescs_[ Idx ];
		CharCodeMap_[ pGlyph->CharCode_ ] = Idx;
	}

	// Mark as ready for use.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnFont::destroy()
{
	//Texture_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcBool ScnFont::createInstance( const std::string& Name, ScnFontComponentRef& FontComponent, ScnMaterialRef Material )
{	
	return CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), FontComponent, this, Material );
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnFont::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnFont::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnFontHeader*)pData;
		
		// Get glyph desc chunk.
		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "glyphs" ) )
	{
		pGlyphDescs_ = (ScnFontGlyphDesc*)pData;

		markCreate(); // All data loaded, time to create.
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFontComponent );

void ScnFontComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Parent_",					&ScnFontComponent::Parent_ ),
		new ReField( "MaterialComponent_",		&ScnFontComponent::MaterialComponent_ ),
		new ReField( "ClippingEnabled_",		&ScnFontComponent::ClippingEnabled_ ),
		new ReField( "ClipMin_",				&ScnFontComponent::ClipMin_ ),
		new ReField( "ClipMax_",				&ScnFontComponent::ClipMax_ ),
	};
		
	ReRegisterClass< ScnFontComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise( ScnFontRef Parent, ScnMaterialRef Material )
{
	Super::initialise();

	Parent_ = Parent; 
	if( CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponent_, Material, 
		ScnShaderPermutationFlags::RENDER_FORWARD |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D ) )
	{	
		BcU32 Sampler = MaterialComponent_->findTextureSlot( "aDiffuseTex" );
		if( Sampler != BcErrorCode )
		{ 
			MaterialComponent_->setTexture( Sampler, Parent_->Texture_ );
		}
	}
	
	// Null uniform buffer.
	UniformBuffer_ = nullptr;

	// Disable clipping.
	setClipping( BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise( const Json::Value& Object )
{
	ScnFontRef FontRef;
	ScnMaterialRef MaterialRef;
	FontRef = getPackage()->getPackageCrossRef( Object[ "font" ].asUInt() );
	MaterialRef = getPackage()->getPackageCrossRef( Object[ "material" ].asUInt() );
	initialise( FontRef, MaterialRef );
}

//////////////////////////////////////////////////////////////////////////
// setClipping
void ScnFontComponent::setClipping( BcBool Enabled, MaVec2d Min, MaVec2d Max )
{
	ClippingEnabled_ = Enabled;
	ClipMin_ = Min;
	ClipMax_ = Max;
}

//////////////////////////////////////////////////////////////////////////
// isReady
MaVec2d ScnFontComponent::draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun, BcU32 Layer )
{
	// Cached elements from parent.
	ScnFontHeader* pHeader = Parent_->pHeader_;
	ScnFontGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );
	
	// Allocate enough vertices for each character.
	ScnCanvasComponentVertex* pFirstVert = SizeRun ? NULL : Canvas->allocVertices( String.length() * 6 );
	ScnCanvasComponentVertex* pVert = pFirstVert;

	// Zero the buffer.
	if( pFirstVert != NULL )
	{
		BcMemZero( pFirstVert, String.length() * 6 * sizeof( ScnCanvasComponentVertex ) );
	}
	
	BcU32 NoofVertices = 0;
	
	BcF32 AdvanceX = 0.0f;
	BcF32 AdvanceY = 0.0f;
		
	BcU32 ABGR = Colour.asABGR();

	MaVec2d MinSize( Position );
	MaVec2d MaxSize( Position );
	
	BcBool FirstCharacterOnLine = BcTrue;

	// TODO: UTF-8 support.
	if( pFirstVert != NULL || SizeRun == BcTrue )
	{
		for( BcU32 CharIdx = 0; CharIdx < String.length(); ++CharIdx )
		{
			BcU32 CharCode = String[ CharIdx ];
			
			// Handle special characters.
			if( CharCode == '\n' )
			{
				AdvanceX = 0.0f;
				AdvanceY += pHeader->NominalSize_;
				FirstCharacterOnLine = BcTrue;
			}
			
			// Find glyph.
			ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
			
			if( Iter != CharCodeMap.end() )
			{
				ScnFontGlyphDesc* pGlyph = &pGlyphDescs[ (*Iter).second ];

				// Bring first character back to the left so it sits on the cursor.
				if( FirstCharacterOnLine )
				{
					AdvanceX -= pGlyph->OffsetX_;
					//AdvanceY -= pGlyph->OffsetY_ + pHeader->NominalSize_;
					FirstCharacterOnLine = BcFalse;
				}
				
				// Calculate size and UVs.
				MaVec2d Size( MaVec2d( pGlyph->Width_, pGlyph->Height_ ) );
				MaVec2d CornerMin( Position + MaVec2d( AdvanceX + pGlyph->OffsetX_, AdvanceY - pGlyph->OffsetY_ + pHeader->NominalSize_ ) );
				MaVec2d CornerMax( CornerMin + Size );
				BcF32 U0 = pGlyph->UA_;
				BcF32 V0 = pGlyph->VA_;
				BcF32 U1 = pGlyph->UB_;
				BcF32 V1 = pGlyph->VB_;
				
				// Pre-clipping size.
				MinSize.x( BcMin( MinSize.x(), CornerMin.x() ) );
				MinSize.y( BcMin( MinSize.y(), CornerMin.y() ) );
				MaxSize.x( BcMax( MaxSize.x(), CornerMin.x() ) );
				MaxSize.y( BcMax( MaxSize.y(), CornerMin.y() ) );
				MinSize.x( BcMin( MinSize.x(), CornerMax.x() ) );
				MinSize.y( BcMin( MinSize.y(), CornerMax.y() ) );
				MaxSize.x( BcMax( MaxSize.x(), CornerMax.x() ) );
				MaxSize.y( BcMax( MaxSize.y(), CornerMax.y() ) );

				// Draw if not a size run.
				if( SizeRun == BcFalse )
				{
					if ( ClippingEnabled_ )
					{
						if ( ( CornerMax.x() < ClipMin_.x() ) || ( CornerMin.x() > ClipMax_.x() ) || ( CornerMax.y() < ClipMin_.y() ) || ( CornerMin.y() > ClipMax_.y() ) )
						{
							// Advance.
							AdvanceX += pGlyph->AdvanceX_;

							// Next character.
							continue;
						}

						BcF32 TexWidth = U1 - U0;
						BcF32 TexHeight = V1 - V0;

						if ( CornerMin.x() < ClipMin_.x() )
						{
							U0 -= ( ( CornerMin.x() - ClipMin_.x() ) / Size.x() ) * TexWidth;
							CornerMin.x( ClipMin_.x() );
						}

						if ( CornerMax.x() > ClipMax_.x() )
						{
							U1 -= ( ( CornerMax.x() - ClipMax_.x() ) / Size.x() ) * TexWidth;
							CornerMax.x( ClipMax_.x() );
						}

						if ( CornerMin.y() < ClipMin_.y() )
						{
							V0 -= ( ( CornerMin.y() - ClipMin_.y() ) / Size.y() ) * TexHeight;
							CornerMin.y( ClipMin_.y() );
						}

						if ( CornerMax.y() > ClipMax_.y() )
						{
							V1 -= ( ( CornerMax.y() - ClipMax_.y() ) / Size.y() ) * TexHeight;
							CornerMax.y( ClipMax_.y() );
						}
					}

					// Add triangle for character.
					pVert->X_ = CornerMin.x();
					pVert->Y_ = CornerMin.y();
					pVert->U_ = U0;
					pVert->V_ = V0;
					pVert->ABGR_ = ABGR;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMin.y();
					pVert->U_ = U1;
					pVert->V_ = V0;
					pVert->ABGR_ = ABGR;
					++pVert;
					
					pVert->X_ = CornerMin.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U0;
					pVert->V_ = V1;
					pVert->ABGR_ = ABGR;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMin.y();
					pVert->U_ = U1;
					pVert->V_ = V0;
					pVert->ABGR_ = ABGR;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U1;
					pVert->V_ = V1;
					pVert->ABGR_ = ABGR;
					++pVert;
					
					pVert->X_ = CornerMin.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U0;
					pVert->V_ = V1;
					pVert->ABGR_ = ABGR;
					++pVert;

					// Add 2 triangles worth of vertices.
					NoofVertices += 6;
				}
								
				// Advance.
				AdvanceX += pGlyph->AdvanceX_;
			}
		}
		
		// Add primitive to canvas.
		if( NoofVertices > 0 )
		{
			Canvas->setMaterialComponent( MaterialComponent_ );
			Canvas->addPrimitive( RsTopologyType::TRIANGLE_LIST, pFirstVert, NoofVertices, Layer );
		}
	}
	else
	{
		BcPrintf( "ScnFontComponent: Out of vertices!\n" );
	}

	return MaxSize - MinSize;
}

//////////////////////////////////////////////////////////////////////////
// drawCentered
MaVec2d ScnFontComponent::drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcU32 Layer )
{
	MaVec2d Size = draw( Canvas, Position, String, Colour, BcTrue, Layer );
	return draw( Canvas, Position - Size * 0.5f, String, Colour, BcFalse, Layer );
}

//////////////////////////////////////////////////////////////////////////
// setAlphaTestStepping
void ScnFontComponent::setAlphaTestStepping( const MaVec2d& Stepping )
{
	AlphaTestUniforms_.AlphaTestParams_ = MaVec4d( Stepping.x(), Stepping.y(), 0.0f, 0.0f );

	RsCore::pImpl()->updateBuffer( 
		UniformBuffer_,
		0, sizeof( AlphaTestUniforms_ ),
		RsResourceUpdateFlags::ASYNC,
		[ & ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			BcMemCopy( Lock.Buffer_, &AlphaTestUniforms_, sizeof( AlphaTestUniforms_ ) );
		} );
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnFontComponent::getMaterialComponent()
{
	return MaterialComponent_;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
void ScnFontComponent::update( BcF32 Tick )
{
	ScnComponent::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
void ScnFontComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Attach material to our parent.
	Parent->attach( MaterialComponent_ );

	UniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( ScnShaderAlphaTestUniformBlockData ) ) );
	auto UniformBlock = MaterialComponent_->findUniformBlock( "AlphaTestUniformBlock" );
	if( UniformBlock != BcErrorCode )
	{
		MaterialComponent_->setUniformBlock( UniformBlock, UniformBuffer_ );
	}

	//
	ScnComponent::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
void ScnFontComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Detach material from our parent.
	Parent->detach( MaterialComponent_ );

	MaterialComponent_ = NULL;
	
	RsCore::pImpl()->destroyResource( UniformBuffer_ );

	//
	ScnComponent::onDetach( Parent );
}
