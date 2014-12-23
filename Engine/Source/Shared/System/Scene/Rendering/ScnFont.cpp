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

#include "System/Scene/Rendering/ScnFont.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnFontImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Internal utility.
namespace 
{
	/**
	 * Get glyph size.
	 */
	inline MaVec2d GetGlyphSize( 
		ScnFontHeader & Header, 
		ScnFontGlyphDesc & Glyph, 
		BcF32 SizeMultiplier )
	{
		return MaVec2d( Glyph.Width_, Glyph.Height_ ) * SizeMultiplier;
	}

	/**
	 * Get glyph offset.
	 */
	 inline MaVec2d GetGlyphOffset( 
	 	ScnFontHeader & Header, 
	 	ScnFontGlyphDesc & Glyph, 
	 	BcF32 SizeMultiplier )
	{
	 	return MaVec2d( 
	 		+ ( Glyph.OffsetX_ * SizeMultiplier ), 
			- ( Glyph.OffsetY_ * SizeMultiplier ) + ( Header.NominalSize_ * SizeMultiplier ) );
	}

	/**
	 * Add glyph vertices.
	 */
	inline BcU32 AddGlyphVertices( 
		ScnCanvasComponentVertex*& pVert,
		const MaVec2d& CornerMin,
		const MaVec2d& CornerMax,
		const MaVec2d& UV0,
		const MaVec2d& UV1,
		BcU32 Colour )
	{
		// Add triangle for character.
		pVert->X_ = CornerMin.x();
		pVert->Y_ = CornerMin.y();
		pVert->U_ = UV0.x();
		pVert->V_ = UV0.y();
		pVert->ABGR_ = Colour;
		++pVert;
		
		pVert->X_ = CornerMax.x();
		pVert->Y_ = CornerMin.y();
		pVert->U_ = UV1.x();
		pVert->V_ = UV0.y();
		pVert->ABGR_ = Colour;
		++pVert;
		
		pVert->X_ = CornerMin.x();
		pVert->Y_ = CornerMax.y();
		pVert->U_ = UV0.x();
		pVert->V_ = UV1.y();
		pVert->ABGR_ = Colour;
		++pVert;
		
		pVert->X_ = CornerMax.x();
		pVert->Y_ = CornerMin.y();
		pVert->U_ = UV1.x();
		pVert->V_ = UV0.y();
		pVert->ABGR_ = Colour;
		++pVert;
		
		pVert->X_ = CornerMax.x();
		pVert->Y_ = CornerMax.y();
		pVert->U_ = UV1.x();
		pVert->V_ = UV1.y();
		pVert->ABGR_ = Colour;
		++pVert;
		
		pVert->X_ = CornerMin.x();
		pVert->Y_ = CornerMax.y();
		pVert->U_ = UV0.x();
		pVert->V_ = UV1.y();
		pVert->ABGR_ = Colour;
		++pVert;

		return 6;
	}

	/**
	 * Clip corners and UVs.
	 * @return true if should draw, false if not.
	 */
	inline BcBool ClipGlyph(
		const MaVec4d& ClippingBounds,
		const MaVec2d& Size,
		MaVec2d& CornerMin,
		MaVec2d& CornerMax,
		MaVec2d& UV0,
		MaVec2d& UV1 )
	{
		if ( ( CornerMax.x() < ClippingBounds.x() ) || 
			 ( CornerMin.x() > ClippingBounds.z() ) || 
			 ( CornerMax.y() < ClippingBounds.y() ) || 
			 ( CornerMin.y() > ClippingBounds.w() ) )
		{
			return BcFalse;
		}

		MaVec2d TexSize = UV1 - UV0;

		if ( CornerMin.x() < ClippingBounds.x() )
		{
			UV0.x( UV0.x() - ( ( CornerMin.x() - ClippingBounds.x() ) / Size.x() ) * TexSize.x() );
			CornerMin.x( ClippingBounds.x() );
		}

		if ( CornerMax.x() > ClippingBounds.z() )
		{
			UV1.x( UV1.x() - ( ( CornerMax.x() - ClippingBounds.z() ) / Size.x() ) * TexSize.x() );
			CornerMax.x( ClippingBounds.z() );
		}

		if ( CornerMin.y() < ClippingBounds.y() )
		{
			UV0.y( UV0.y() - ( ( CornerMin.y() - ClippingBounds.y() ) / Size.y() ) * TexSize.y() );
			CornerMin.y( ClippingBounds.y() );
		}

		if ( CornerMax.y() > ClippingBounds.w() )
		{
			UV1.y( UV1.y() - ( ( CornerMax.y() - ClippingBounds.w() ) / Size.y() ) * TexSize.y() );
			CornerMax.y( ClippingBounds.w() );
		}

		return BcTrue;
	}

	/**
	 * Length UTF-8. TODO, actually implement later. Pass through for now.
	 * @param Char Char array pointer.
	 * @return Number of UTF-8 chars total.
	 */
	inline BcU32 LengthUTF8( const char* Char )
	{
		return strlen( Char );
	}

	/**
	 * Decode UTF-8. TODO, actually implement later. Pass through for now.
	 * @param Char Char array pointer reference. Will return on next char.
	 * @param RemainingChars Number of chars remaining.
	 * @return Decoded chracter.
	 */
	inline BcU32 DecodeUTF8( const char*& Char, int& RemainingChars )
	{
		BcU32 OutChar = 0;
	 	if( RemainingChars > 0 )
	 	{
		 	--RemainingChars;
		 	OutChar = static_cast< BcU32 >( *Char++ );
		}
		return OutChar;
	}
}

//////////////////////////////////////////////////////////////////////////
// Font draw params.
REFLECTION_DEFINE_BASIC( ScnFontDrawParams );

void ScnFontDrawParams::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Alignment_", &ScnFontDrawParams::Alignment_ ),
		new ReField( "Layer_", &ScnFontDrawParams::Layer_ ),
		new ReField( "Size_", &ScnFontDrawParams::Size_ ),
		new ReField( "ClippingEnabled_", &ScnFontDrawParams::ClippingEnabled_ ),
		new ReField( "ClippingBounds_", &ScnFontDrawParams::ClippingBounds_ ),
		new ReField( "Colour_", &ScnFontDrawParams::Colour_ ),
		new ReField( "AlphaTestSettings_", &ScnFontDrawParams::AlphaTestSettings_ ),
	};
	
	ReRegisterClass< ScnFontDrawParams >( Fields );


	ReEnumConstant* ScnFontAlignmentEnumConstants[] = 
	{
		new ReEnumConstant( "LEFT", ScnFontAlignment::LEFT ),
		new ReEnumConstant( "RIGHT", ScnFontAlignment::LEFT ),
		new ReEnumConstant( "VCENTRE", ScnFontAlignment::VCENTRE ),
		new ReEnumConstant( "TOP", ScnFontAlignment::TOP ),
		new ReEnumConstant( "BOTTOM", ScnFontAlignment::BOTTOM ),
		new ReEnumConstant( "HCENTRE", ScnFontAlignment::HCENTRE ),
	};
	ReRegisterEnum< ScnFontAlignment >( ScnFontAlignmentEnumConstants );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnFontDrawParams::ScnFontDrawParams():
	Alignment_( ScnFontAlignment::LEFT | ScnFontAlignment::TOP ),
	Layer_( 0 ),
	Size_( 1.0f ),
	ClippingEnabled_( BcFalse ),
	ClippingBounds_( 0.0f, 0.0f, 0.0f, 0.0f ),
	Colour_( RsColour::BLACK ),
	AlphaTestSettings_( 0.4f, 0.5f, 0.0f, 0.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// setAlignment
ScnFontDrawParams& ScnFontDrawParams::setAlignment( ScnFontAlignment Alignment )
{
	Alignment_ = Alignment;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getAlignment
ScnFontAlignment ScnFontDrawParams::getAlignment() const
{
	return Alignment_;
}

//////////////////////////////////////////////////////////////////////////
// setLayer
ScnFontDrawParams& ScnFontDrawParams::setLayer( BcU32 Layer )
{
	Layer_ = Layer;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getLayer
BcU32 ScnFontDrawParams::getLayer() const
{
	return Layer_;
}

//////////////////////////////////////////////////////////////////////////
// setSize
ScnFontDrawParams& ScnFontDrawParams::setSize( BcF32 Size )
{
	Size_ = Size;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getSize
BcF32 ScnFontDrawParams::getSize() const
{
	return Size_;
}

//////////////////////////////////////////////////////////////////////////
// setClippingEnabled
ScnFontDrawParams& ScnFontDrawParams::setClippingEnabled( BcBool Enabled )
{
	ClippingEnabled_ = Enabled;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getClippingEnabled
BcBool ScnFontDrawParams::getClippingEnabled() const
{
	return ClippingEnabled_;
}

//////////////////////////////////////////////////////////////////////////
// setClippingBounds
ScnFontDrawParams& ScnFontDrawParams::setClippingBounds( const MaVec4d& Bounds )
{
	ClippingBounds_ = Bounds;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getClippingBounds
const MaVec4d& ScnFontDrawParams::getClippingBounds() const
{
	return ClippingBounds_;	
}

//////////////////////////////////////////////////////////////////////////
// setColour
ScnFontDrawParams& ScnFontDrawParams::setColour( const RsColour& Colour )
{
	Colour_ = Colour;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getColour
const RsColour& ScnFontDrawParams::getColour() const
{
	return Colour_;
}

//////////////////////////////////////////////////////////////////////////
// setAlphaTestSettings
ScnFontDrawParams& ScnFontDrawParams::setAlphaTestSettings( const MaVec4d& Settings )
{
	AlphaTestSettings_ = Settings;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getAlphaTestSettings
const MaVec4d& ScnFontDrawParams::getAlphaTestSettings() const
{
	return AlphaTestSettings_;
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFont );

void ScnFont::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnFont::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "pGlyphDescs_", &ScnFont::pGlyphDescs_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "CharCodeMap_", &ScnFont::CharCodeMap_ ),
		new ReField( "Texture_", &ScnFont::Texture_, bcRFF_SHALLOW_COPY ),
	};
	
	auto& Class = ReRegisterClass< ScnFont, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnFontImport::StaticGetClass(), 0 ) );
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
	Texture_ = ScnTextureRef( getPackage()->getCrossRefResource( pHeader_->TextureRef_ ) );

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
		new ReField( "Parent_", &ScnFontComponent::Parent_, bcRFF_SHALLOW_COPY ),
		new ReField( "Material_", &ScnFontComponent::Material_, bcRFF_SHALLOW_COPY ),
		new ReField( "MaterialComponent_", &ScnFontComponent::MaterialComponent_, bcRFF_TRANSIENT ),
		new ReField( "ClippingEnabled_", &ScnFontComponent::ClippingEnabled_ ),
		new ReField( "ClipMin_", &ScnFontComponent::ClipMin_ ),
		new ReField( "ClipMax_", &ScnFontComponent::ClipMax_ ),
		new ReField( "AlphaTestUniforms_", &ScnFontComponent::AlphaTestUniforms_ ),
	};
		
	ReRegisterClass< ScnFontComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise()
{
	Super::initialise();

	Parent_ = nullptr;
	Material_ = nullptr;
	MaterialComponent_ = nullptr;
	ClippingEnabled_ = BcFalse;

	// Null uniform buffer.
	UniformBuffer_ = nullptr;

	// Setup default alpha test params.
	AlphaTestUniforms_.AlphaTestParams_ = MaVec4d( 0.4f, 0.5f, 0.5f, 0.0f );

	// Disable clipping.
	setClipping( BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise( ScnFontRef Parent, ScnMaterialRef Material )
{
	initialise();

	Parent_ = Parent; 
	Material_ = Material;

}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise( const Json::Value& Object )
{
	ScnFontRef FontRef;
	ScnMaterialRef MaterialRef;
	FontRef = getPackage()->getCrossRefResource( Object[ "font" ].asUInt() );
	MaterialRef = getPackage()->getCrossRefResource( Object[ "material" ].asUInt() );
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
// draw
MaVec2d ScnFontComponent::draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun, BcU32 Layer )
{
	return draw( Canvas, Position, Parent_->pHeader_->NominalSize_, String, Colour, SizeRun, Layer );
}

//////////////////////////////////////////////////////////////////////////
// drawCentered
MaVec2d ScnFontComponent::drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcU32 Layer )
{
	MaVec2d Size = draw( Canvas, Position, String, Colour, BcTrue, Layer );
	return draw( Canvas, Position - Size * 0.5f, String, Colour, BcFalse, Layer );
}

//////////////////////////////////////////////////////////////////////////
// draw
MaVec2d ScnFontComponent::draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, BcF32 Size, const std::string& String, RsColour Colour, BcBool SizeRun, BcU32 Layer )
{
	// Cached elements from parent.
	ScnFontHeader* pHeader = Parent_->pHeader_;
	ScnFontGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );
	
	BcF32 SizeMultiplier = Size / pHeader->NominalSize_;

	// Allocate enough vertices for each character.
	ScnCanvasComponentVertex* pFirstVert = SizeRun ? nullptr : Canvas->allocVertices( String.length() * 6 );
	ScnCanvasComponentVertex* pVert = pFirstVert;

	// Zero the buffer.
	if( pFirstVert != nullptr )
	{
		BcMemZero( pFirstVert, String.length() * 6 * sizeof( ScnCanvasComponentVertex ) );
	}
	
	BcU32 NoofVertices = 0;
	
	BcF32 AdvanceX = 0.0f;
	BcF32 AdvanceY = 0.0f;
	
	BcU32 ABGR = Colour.asABGR();

	MaVec2d MinSize( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
	MaVec2d MaxSize( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );
	
	BcBool FirstCharacterOnLine = BcTrue;

	if( pFirstVert != nullptr || SizeRun == BcTrue )
	{
		int RemainingChars = String.length();
		const char* StringChar = String.c_str();

		while( RemainingChars > 0 )
		{
			BcU32 CharCode = DecodeUTF8( StringChar, RemainingChars );
			
			// Handle special characters.
			if( CharCode == '\n' )
			{
				AdvanceX = 0.0f;
				AdvanceY += pHeader->NominalSize_ * SizeMultiplier;
				FirstCharacterOnLine = BcTrue;
			}
			
			// Find glyph.
			ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
			
			if( Iter != CharCodeMap.end() )
			{
				ScnFontGlyphDesc& Glyph = pGlyphDescs[ (*Iter).second ];

				// Bring first character back to the left so it sits on the cursor.
				if( FirstCharacterOnLine )
				{
					AdvanceX -= Glyph.OffsetX_;
					//AdvanceY -= pGlyph->OffsetY_ + pHeader->NominalSize_;
					FirstCharacterOnLine = BcFalse;
				}
				
				// Calculate size and UVs.
				MaVec2d Size = GetGlyphSize( *pHeader, Glyph, SizeMultiplier );
				MaVec2d CornerMin( MaVec2d( AdvanceX, AdvanceY ) + GetGlyphOffset( *pHeader, Glyph, SizeMultiplier ) );
				MaVec2d CornerMax( CornerMin + Size );
				MaVec2d UV0( Glyph.UA_, Glyph.VA_ );
				MaVec2d UV1( Glyph.UB_, Glyph.VB_ );
				
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
						if( !ClipGlyph(
							MaVec4d( ClipMin_.x(), ClipMin_.y(), ClipMax_.x(), ClipMax_.y() ),
							Size,
							CornerMin,
							CornerMax,
							UV0,
							UV1 ) )
						{
							// Advance.
							AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;

							// Next character.
							continue;
						}
					}

					NoofVertices += AddGlyphVertices( 
						pVert, CornerMin, CornerMax, UV0, UV1, ABGR );
				}
								
				// Advance.
				AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;
			}
		}
		
		// Update min + max sizes.
		MinSize += Position;
		MaxSize += Position;

		// Add primitive to canvas.
		if( NoofVertices > 0 )
		{
			// Update position of all vertices.
			for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
			{
				auto& Vertex = pFirstVert[ Idx ];
				Vertex.X_ += Position.x();
				Vertex.Y_ += Position.y();
			}

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
MaVec2d ScnFontComponent::drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, BcF32 Size, const std::string& String, RsColour Colour, BcU32 Layer )
{
	MaVec2d FontSize = draw( Canvas, Position, Size, String, Colour, BcTrue, Layer );
	return draw( Canvas, Position - FontSize * 0.5f, Size, String, Colour, BcFalse, Layer );
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
// drawText
MaVec2d ScnFontComponent::drawText( 
	ScnCanvasComponentRef Canvas, 
	const ScnFontDrawParams& DrawParams,
	const MaVec2d& Position,
	const MaVec2d& Bounds,
	const std::wstring& Text )
{
	// Cached elements from parent.
	ScnFontHeader* pHeader = Parent_->pHeader_;
	ScnFontGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );
	
	BcU32 TextLength = Text.length();
	BcBool SizeRun = Canvas.isValid() == BcFalse;
	BcF32 SizeMultiplier = DrawParams.getSize() / pHeader->NominalSize_;

	// Allocate enough vertices for each character.
	ScnCanvasComponentVertex* pFirstVert = SizeRun ? nullptr : Canvas->allocVertices( TextLength * 6 );
	ScnCanvasComponentVertex* pVert = pFirstVert;

	// Zero the buffer.
	if( pFirstVert != nullptr )
	{
		BcMemZero( pFirstVert, TextLength * 6 * sizeof( ScnCanvasComponentVertex ) );
	}
	
	BcU32 NoofVertices = 0;
	
	BcF32 AdvanceX = 0.0f;
	BcF32 AdvanceY = 0.0f;
	
	BcU32 ABGR = DrawParams.getColour().asABGR();

	MaVec2d MinSize( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
	MaVec2d MaxSize( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );
	
	BcBool FirstCharacterOnLine = BcTrue;

	if( pFirstVert != nullptr || SizeRun == BcTrue )
	{
		for( BcU32 Idx = 0; Idx < TextLength; ++Idx )
		{
			BcU32 CharCode = Text[ Idx ];
			
			// Handle special characters.
			if( CharCode == '\n' )
			{
				AdvanceX = 0.0f;
				AdvanceY += pHeader->NominalSize_ * SizeMultiplier;
				FirstCharacterOnLine = BcTrue;
			}
			
			// Find glyph.
			ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
			
			if( Iter != CharCodeMap.end() )
			{
				ScnFontGlyphDesc& Glyph = pGlyphDescs[ (*Iter).second ];

				// Bring first character back to the left so it sits on the cursor.
				if( FirstCharacterOnLine )
				{
					AdvanceX -= Glyph.OffsetX_;
					//AdvanceY -= pGlyph->OffsetY_ + pHeader->NominalSize_;
					FirstCharacterOnLine = BcFalse;
				}
				
				// Calculate size and UVs.
				MaVec2d Size = GetGlyphSize( *pHeader, Glyph, SizeMultiplier );
				MaVec2d CornerMin( MaVec2d( AdvanceX, AdvanceY ) + GetGlyphOffset( *pHeader, Glyph, SizeMultiplier ) );
				MaVec2d CornerMax( CornerMin + Size );
				MaVec2d UV0( Glyph.UA_, Glyph.VA_ );
				MaVec2d UV1( Glyph.UB_, Glyph.VB_ );
				
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
						if( !ClipGlyph(
							MaVec4d( ClipMin_.x(), ClipMin_.y(), ClipMax_.x(), ClipMax_.y() ),
							Size,
							CornerMin,
							CornerMax,
							UV0,
							UV1 ) )
						{
							// Advance.
							AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;

							// Next character.
							continue;
						}
					}

					NoofVertices += AddGlyphVertices( 
						pVert, CornerMin, CornerMax, UV0, UV1, ABGR );
				}
								
				// Advance.
				AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;
			}
		}
		
		// Update min + max sizes.
		MinSize += Position;
		MaxSize += Position;

		// Add primitive to canvas.
		if( NoofVertices > 0 )
		{
			// Update position of all vertices.
			for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
			{
				auto& Vertex = pFirstVert[ Idx ];
				Vertex.X_ += Position.x();
				Vertex.Y_ += Position.y();
			}

			Canvas->setMaterialComponent( MaterialComponent_ );
			Canvas->addPrimitive( RsTopologyType::TRIANGLE_LIST, pFirstVert, NoofVertices, DrawParams.getLayer() );
		}
	}
	else
	{
		BcPrintf( "ScnFontComponent: Out of vertices!\n" );
	}

	return MaxSize - MinSize;
}

//////////////////////////////////////////////////////////////////////////
// measureText
MaVec2d ScnFontComponent::measureText( 
	const ScnFontDrawParams& DrawParams,
	const std::wstring& Text )
{
	// Cached elements from parent.
	ScnFontHeader* pHeader = Parent_->pHeader_;
	ScnFontGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );

	const BcU32 TextLength = Text.length();
	const BcF32 SizeMultiplier = DrawParams.getSize() / pHeader->NominalSize_;

	BcF32 AdvanceX = 0.0f;
	BcF32 AdvanceY = 0.0f;

	MaVec2d MinSize( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
	MaVec2d MaxSize( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );
	
	BcBool FirstCharacterOnLine = BcTrue;

	for( BcU32 Idx = 0; Idx < TextLength; ++Idx )
	{
		BcU32 CharCode = Text[ Idx ];
		
		// Handle special characters.
		if( CharCode == '\n' )
		{
			AdvanceX = 0.0f;
			AdvanceY += pHeader->NominalSize_ * SizeMultiplier;
			FirstCharacterOnLine = BcTrue;
		}
		
		// Find glyph.
		ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
		
		if( Iter != CharCodeMap.end() )
		{
			ScnFontGlyphDesc& Glyph = pGlyphDescs[ (*Iter).second ];

			// Bring first character back to the left so it sits on the cursor.
			if( FirstCharacterOnLine )
			{
				AdvanceX -= Glyph.OffsetX_;
				//AdvanceY -= pGlyph->OffsetY_ + pHeader->NominalSize_;
				FirstCharacterOnLine = BcFalse;
			}
			
			// Calculate size and UVs.
			MaVec2d Size = GetGlyphSize( *pHeader, Glyph, SizeMultiplier );
			MaVec2d CornerMin( MaVec2d( AdvanceX, AdvanceY ) + GetGlyphOffset( *pHeader, Glyph, SizeMultiplier ) );
			MaVec2d CornerMax( CornerMin + Size );
			
			// Pre-clipping size.
			MinSize.x( BcMin( MinSize.x(), CornerMin.x() ) );
			MinSize.y( BcMin( MinSize.y(), CornerMin.y() ) );
			MaxSize.x( BcMax( MaxSize.x(), CornerMin.x() ) );
			MaxSize.y( BcMax( MaxSize.y(), CornerMin.y() ) );
			MinSize.x( BcMin( MinSize.x(), CornerMax.x() ) );
			MinSize.y( BcMin( MinSize.y(), CornerMax.y() ) );
			MaxSize.x( BcMax( MaxSize.x(), CornerMax.x() ) );
			MaxSize.y( BcMax( MaxSize.y(), CornerMax.y() ) );
							
			// Advance.
			AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;
		}
	}

	return MaxSize - MinSize;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnFontComponent::update( BcF32 Tick )
{
	ScnComponent::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnFontComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Attach material to our parent.
	ScnMaterialComponentRef MaterialComponent;
	if( CsCore::pImpl()->createResource( 
		BcName::INVALID, getPackage(), MaterialComponent, Material_, 
		ScnShaderPermutationFlags::RENDER_FORWARD |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D ) )
	{	
		MaterialComponent_ = MaterialComponent;
		BcU32 Sampler = MaterialComponent_->findTextureSlot( "aDiffuseTex" );
		if( Sampler != BcErrorCode )
		{ 
			MaterialComponent_->setTexture( Sampler, Parent_->Texture_ );
		}
	}
	Parent->attach( MaterialComponent_ );

	UniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( ScnShaderAlphaTestUniformBlockData ) ) );
	auto UniformBlock = MaterialComponent_->findUniformBlock( "ScnShaderAlphaTestUniformBlockData" );
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

	MaterialComponent_ = nullptr;
	
	RsCore::pImpl()->destroyResource( UniformBuffer_ );

	//
	ScnComponent::onDetach( Parent );
}
