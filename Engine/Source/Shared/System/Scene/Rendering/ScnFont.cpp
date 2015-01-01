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
			- ( Glyph.OffsetY_ * SizeMultiplier ) );
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
	 * Position vertices.
	 * @param Vertices Pointer to vertices.
	 * @param NoofVertices Number of vertices to position.
	 * @param Position Position to add to vertices.
	 */
	inline void PositionVertices( 
		ScnCanvasComponentVertex* Vertices,
		BcU32 NoofVertices,
		const MaVec2d& Position )
	{
		// Update position of all vertices.
		for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
		{
			Vertices->X_ += Position.x();
			Vertices->Y_ += Position.y();
			++Vertices;
		}
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

	/**
	 * Is whitespace?
	 */
	inline BcBool IsWhitespace( BcU32 CharCode )
	{
		return CharCode == ' ' || CharCode == '\t' || CharCode == '\n';
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
		new ReField( "WrappingEnabled_", &ScnFontDrawParams::WrappingEnabled_ ),
		new ReField( "Layer_", &ScnFontDrawParams::Layer_ ),
		new ReField( "Size_", &ScnFontDrawParams::Size_ ),
		new ReField( "ClippingEnabled_", &ScnFontDrawParams::ClippingEnabled_ ),
		new ReField( "ClippingBounds_", &ScnFontDrawParams::ClippingBounds_ ),
		new ReField( "TextColour_", &ScnFontDrawParams::TextColour_ ),
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
	AlignmentBorder_( 0.0f ),
	WrappingEnabled_( BcFalse ),
	Layer_( 0 ),
	Size_( 1.0f ),
	ClippingEnabled_( BcFalse ),
	ClippingBounds_( 0.0f, 0.0f, 0.0f, 0.0f ),
	TextColour_( RsColour::BLACK ),
	AlphaTestSettings_( 0.4f, 0.5f, 0.0f, 0.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// setWrappingEnabled
ScnFontDrawParams& ScnFontDrawParams::setWrappingEnabled( BcBool Enabled )
{
	WrappingEnabled_ = Enabled;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getWrappingEnabled
BcBool ScnFontDrawParams::getWrappingEnabled() const
{
	return WrappingEnabled_;
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
// setAlignmentBorder
ScnFontDrawParams& ScnFontDrawParams::setAlignmentBorder( BcF32 AlignmentBorder )
{
	AlignmentBorder_ = AlignmentBorder;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getAlignmentBorder
BcF32 ScnFontDrawParams::getAlignmentBorder() const
{
	return AlignmentBorder_;
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
// setTextColour
ScnFontDrawParams& ScnFontDrawParams::setTextColour( const RsColour& TextColour )
{
	TextColour_ = TextColour;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// getTextColour
const RsColour& ScnFontDrawParams::getTextColour() const
{
	return TextColour_;
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
// ScnFontUniformBlockData
REFLECTION_DEFINE_BASIC( ScnFontUniformBlockData );

void ScnFontUniformBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "FontParams_", &ScnFontUniformBlockData::FontParams_ ),
		new ReField( "TextColour_", &ScnFontUniformBlockData::TextColour_ ),
		new ReField( "BorderColour_", &ScnFontUniformBlockData::BorderColour_ ),
		new ReField( "ShadowColour_", &ScnFontUniformBlockData::ShadowColour_ ),
	};
	
	ReRegisterClass< ScnFontUniformBlockData >( Fields );
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
		new ReField( "FontUniformData_", &ScnFontComponent::FontUniformData_ ),
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
	FontUniformData_.FontParams_ = MaVec4d( 0.4f, 0.5f, 0.5f, 0.0f );
	FontUniformData_.TextColour_ = RsColour::BLACK;
	FontUniformData_.BorderColour_ = RsColour::BLACK;
	FontUniformData_.ShadowColour_ = RsColour::BLACK;

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

	ScnFontUniformBlockData FontUniformData = FontUniformData_;

	// Add custom render command to canvas to update the uniform buffer correctly.
	Canvas->setMaterialComponent( MaterialComponent_ );
	Canvas->addCustomRender(
		[ this, FontUniformData ]( RsContext* Context )
		{
			Context->updateBuffer( 
				UniformBuffer_,
				0, sizeof( FontUniformData ),
				RsResourceUpdateFlags::NONE,
				[ & ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					BcMemCopy( Lock.Buffer_, &FontUniformData, sizeof( FontUniformData ) );
				} );
		},
		Layer );

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
			PositionVertices( pFirstVert, NoofVertices, Position );

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
	FontUniformData_.FontParams_ = MaVec4d( Stepping.x(), Stepping.y(), 0.0f, 0.0f );
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
	const MaVec2d& TargetSize,
	const std::wstring& Text )
{
	// Grab values from draw params and check validity.
	const BcU32 ABGR = DrawParams.getTextColour().asABGR();
	const ScnFontAlignment Alignment = DrawParams.getAlignment();
	const BcBool WrappingEnabled = DrawParams.getWrappingEnabled();
	const BcF32 AlignmentBorder = DrawParams.getAlignmentBorder();

	BcAssertMsg( ( Alignment & ScnFontAlignment::HORIZONTAL ) != ScnFontAlignment::NONE, 
		"Missing horizontal alignment flags." );
	BcAssertMsg( ( Alignment & ScnFontAlignment::VERTICAL ) != ScnFontAlignment::NONE, 
		"Missing vertical alignment flags." );
	BcAssertMsg( DrawParams.getSize() > 0.0f,
		"Font size must be greater than 0.0" );

	// TODO: Set in render thread.
	setAlphaTestStepping( 
		MaVec2d( DrawParams.getAlphaTestSettings().x(), DrawParams.getAlphaTestSettings().y() ) );

	ScnFontUniformBlockData FontUniformData = FontUniformData_;

	// Add custom render command to canvas to update the uniform buffer correctly.
	Canvas->setMaterialComponent( MaterialComponent_ );
	Canvas->addCustomRender(
		[ this, FontUniformData ]( RsContext* Context)
		{
			Context->updateBuffer( 
				UniformBuffer_,
				0, sizeof( FontUniformData ),
				RsResourceUpdateFlags::NONE,
				[ & ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					BcMemCopy( Lock.Buffer_, &FontUniformData, sizeof( FontUniformData ) );
				} );
		},
		DrawParams.getLayer() );

	// Cached elements from parent.
	ScnFontHeader* pHeader = Parent_->pHeader_;
	ScnFontGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );
	
	BcU32 TextLength = Text.length();
	BcF32 SizeMultiplier = DrawParams.getSize() / pHeader->NominalSize_;

	// Allocate enough vertices for each character.
	ScnCanvasComponentVertex* pFirstVert = Canvas->allocVertices( TextLength * 6 );
	ScnCanvasComponentVertex* pFirstVertOnLine = pFirstVert;
	ScnCanvasComponentVertex* pVert = pFirstVert;
	BcU32 NoofVerticesOnLine = 0;

	// Zero the buffer.
	if( pFirstVert != nullptr )
	{
		BcMemZero( pFirstVert, TextLength * 6 * sizeof( ScnCanvasComponentVertex ) );
	}
	
	BcU32 NoofVertices = 0;
	
	BcF32 AdvanceX = 0.0f;
	BcF32 AdvanceY = 0.0f;
	
	MaVec2d MinSize( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
	MaVec2d MaxSize( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );

	MaVec2d MinLineSize( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
	MaVec2d MaxLineSize( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );

	auto TerminateLineFunc = [ & ]()
	{
		// Update text min/max sizes.
		MinSize.y( BcMin( MinSize.y(), MinLineSize.y() ) );
		MinSize.x( BcMin( MinSize.x(), MinLineSize.x() ) );
		MaxSize.x( BcMax( MaxSize.x(), MaxLineSize.x() ) );
		MaxSize.y( BcMax( MaxSize.y(), MaxLineSize.y() ) );
		MinSize.x( BcMin( MinSize.x(), MinLineSize.x() ) );
		MinSize.y( BcMin( MinSize.y(), MinLineSize.y() ) );
		MaxSize.x( BcMax( MaxSize.x(), MaxLineSize.x() ) );
		MaxSize.y( BcMax( MaxSize.y(), MaxLineSize.y() ) );

		MaVec2d Offset = MaVec2d( AlignmentBorder - MinSize.x(), 0.0f );
		MaVec2d LineSize = ( MaxLineSize - MinLineSize ) + MaVec2d( AlignmentBorder * 2.0f, 0.0f );

		// Position along the x axis using appropriate alignment.
		if( ( Alignment & ScnFontAlignment::LEFT ) != ScnFontAlignment::NONE )
		{
			PositionVertices( pFirstVertOnLine, NoofVerticesOnLine, 
				MaVec2d( Position.x(), 0.0f ) +
				Offset );
		}
		else if( ( Alignment & ScnFontAlignment::RIGHT ) != ScnFontAlignment::NONE )
		{
			PositionVertices( pFirstVertOnLine, NoofVerticesOnLine, 
				MaVec2d( ( TargetSize.x() + Position.x() - LineSize.x() ), 0.0f ) + 
				Offset );
		}
		else if( ( Alignment & ScnFontAlignment::HCENTRE ) != ScnFontAlignment::NONE )
		{
			PositionVertices( pFirstVertOnLine, NoofVerticesOnLine, 
				MaVec2d( ( ( TargetSize.x() * 0.5f ) + Position.x() - LineSize.x() * 0.5f ), 0.0f ) +
				Offset );
		}

		// Reset line.
		AdvanceX = 0.0f;
		AdvanceY += pHeader->NominalSize_ * SizeMultiplier;
		pFirstVertOnLine = pVert;
		NoofVerticesOnLine = 0;

		MinLineSize = MaVec2d( std::numeric_limits< BcF32 >::max(), std::numeric_limits< BcF32 >::max() );
		MaxLineSize = MaVec2d( std::numeric_limits< BcF32 >::min(), std::numeric_limits< BcF32 >::min() );
	};
	
	if( pFirstVert != nullptr )
	{
		BcU32 LastWhitespaceIdx = -1;

		// Iterate and include null terminator.
		for( BcU32 Idx = 0; Idx < TextLength + 1; ++Idx )
		{
			BcU32 CharCode = Text[ Idx ];
			
			// Handle special characters.
			if( CharCode == '\n' ||
				CharCode == '\0' )
			{
				TerminateLineFunc();

				// Null term? Bail.
				if( CharCode == '\0' )
				{
					break;
				}
			}

			// Grab index of last whitespace character.
			if( IsWhitespace( CharCode ) )
			{
				LastWhitespaceIdx = Idx;
			}

			// Find glyph.
			ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
			
			if( Iter != CharCodeMap.end() )
			{
				ScnFontGlyphDesc& Glyph = pGlyphDescs[ (*Iter).second ];
				
				// Calculate size and UVs.
				MaVec2d Size = GetGlyphSize( *pHeader, Glyph, SizeMultiplier );
				MaVec2d CornerMin( MaVec2d( AdvanceX, AdvanceY ) + GetGlyphOffset( *pHeader, Glyph, SizeMultiplier ) );
				MaVec2d CornerMax( CornerMin + Size );
				MaVec2d GlyphMin( CornerMin + MaVec2d( pHeader->BorderSize_, pHeader->BorderSize_ ) * SizeMultiplier );
				MaVec2d GlyphMax( CornerMax - MaVec2d( pHeader->BorderSize_, pHeader->BorderSize_ ) * SizeMultiplier );
				MaVec2d UV0( Glyph.UA_, Glyph.VA_ );
				MaVec2d UV1( Glyph.UB_, Glyph.VB_ );

				// Pre-clipping size.
				MinLineSize.x( BcMin( MinLineSize.x(), GlyphMin.x() ) );
				MinLineSize.y( BcMin( MinLineSize.y(), GlyphMin.y() ) );
				MaxLineSize.x( BcMax( MaxLineSize.x(), GlyphMin.x() ) );
				MaxLineSize.y( BcMax( MaxLineSize.y(), GlyphMin.y() ) );
				MinLineSize.x( BcMin( MinLineSize.x(), GlyphMax.x() ) );
				MinLineSize.y( BcMin( MinLineSize.y(), GlyphMax.y() ) );
				MaxLineSize.x( BcMax( MaxLineSize.x(), GlyphMax.x() ) );
				MaxLineSize.y( BcMax( MaxLineSize.y(), GlyphMax.y() ) );

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

				BcU32 NoofVerticesForGlyph = AddGlyphVertices( 
					pVert, CornerMin, CornerMax, UV0, UV1, ABGR );
				
				NoofVertices += NoofVerticesForGlyph;
				NoofVerticesOnLine += NoofVerticesForGlyph;

				// Handle wrapping.
				if( WrappingEnabled )
				{
					// If the character spills over, terminate this line.
					if( TargetSize.x() > 0.0f && CornerMax.x() > TargetSize.x() )
					{
						// Back track the vertices.
						BcU32 NoofCharsSkipped = Idx - LastWhitespaceIdx;
						BcU32 NoofVertsSkipped = NoofCharsSkipped * NoofVerticesForGlyph;
						pVert -= NoofVertsSkipped;
						NoofVertices -= NoofVertsSkipped;
						NoofVerticesOnLine -= NoofVertsSkipped;

						// Skip all leading whitespace.
						Idx = LastWhitespaceIdx;
						while( Idx < Text.length() && IsWhitespace( Text[ Idx ] ) )
						{
							LastWhitespaceIdx = Idx++;
						}
						Idx = LastWhitespaceIdx;

						TerminateLineFunc();
						continue;
					}
				}

				// Advance.
				AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;
			}
		}
		
		// Add primitive to canvas.
		if( NoofVertices > 0 )
		{
			MaVec2d FinalSize = ( MaxSize - MinSize ) + MaVec2d( 0.0f, AlignmentBorder * 2.0f );
			MaVec2d Offset = MaVec2d( 0.0f, AlignmentBorder - MinSize.y() );

			// Position along the y axis using appropriate alignment.
			if( ( Alignment & ScnFontAlignment::TOP ) != ScnFontAlignment::NONE )
			{
				PositionVertices( pFirstVert, NoofVertices, 
					MaVec2d( 0.0f, Position.y() ) +
					Offset );
			}
			else if( ( Alignment & ScnFontAlignment::BOTTOM ) != ScnFontAlignment::NONE )
			{
				PositionVertices( pFirstVert, NoofVertices, 
					MaVec2d( 0.0f, ( TargetSize.y() + Position.y() - FinalSize.y() ) ) +
					Offset );
			}
			else if( ( Alignment & ScnFontAlignment::VCENTRE ) != ScnFontAlignment::NONE )
			{
				PositionVertices( pFirstVert, NoofVertices, 
					MaVec2d( 0.0f, ( TargetSize.y() * 0.5f ) + Position.y() - ( FinalSize.y() * 0.5f ) ) +
					Offset );
			}

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
	
	for( BcU32 Idx = 0; Idx < TextLength; ++Idx )
	{
		BcU32 CharCode = Text[ Idx ];
		
		// Handle special characters.
		if( CharCode == '\n' )
		{
			AdvanceX = 0.0f;
			AdvanceY += pHeader->NominalSize_ * SizeMultiplier;
		}
		
		// Find glyph.
		ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
		
		if( Iter != CharCodeMap.end() )
		{
			ScnFontGlyphDesc& Glyph = pGlyphDescs[ (*Iter).second ];
			
			// Calculate size and UVs.
			MaVec2d Size = GetGlyphSize( *pHeader, Glyph, SizeMultiplier );
			MaVec2d CornerMin( MaVec2d( AdvanceX, AdvanceY ) + GetGlyphOffset( *pHeader, Glyph, SizeMultiplier ) );
			MaVec2d CornerMax( CornerMin + Size );
			MaVec2d GlyphMin( CornerMin + MaVec2d( pHeader->BorderSize_, pHeader->BorderSize_ ) * SizeMultiplier );
			MaVec2d GlyphMax( CornerMax - MaVec2d( pHeader->BorderSize_, pHeader->BorderSize_ ) * SizeMultiplier );

			// Pre-clipping size.
			MinSize.x( BcMin( MinSize.x(), GlyphMin.x() ) );
			MinSize.y( BcMin( MinSize.y(), GlyphMin.y() ) );
			MaxSize.x( BcMax( MaxSize.x(), GlyphMin.x() ) );
			MaxSize.y( BcMax( MaxSize.y(), GlyphMin.y() ) );
			MinSize.x( BcMin( MinSize.x(), GlyphMax.x() ) );
			MinSize.y( BcMin( MinSize.y(), GlyphMax.y() ) );
			MaxSize.x( BcMax( MaxSize.x(), GlyphMax.x() ) );
			MaxSize.y( BcMax( MaxSize.y(), GlyphMax.y() ) );

			// Advance.
			AdvanceX += Glyph.AdvanceX_ * SizeMultiplier;
		}
	}

	return ( MaxSize - MinSize ) + 
		MaVec2d( DrawParams.getAlignmentBorder(), DrawParams.getAlignmentBorder() ) * 2.0f;
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
			sizeof( FontUniformData_ ) ) );
	auto UniformBlock = MaterialComponent_->findUniformBlock( "ScnFontUniformBlockData" );
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
