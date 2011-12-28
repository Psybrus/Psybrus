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

#include "ScnFont.h"
#include "ScnEntity.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "Img.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// makeImageForGlyph
static ImgImage* makeImageForGlyph( FT_Glyph Glyph, FT_Render_Mode RenderMode, BcU32 BorderSize )
{
	ImgImage* pImage = NULL;
	BcU32 DoubleBorderSize = BorderSize * 2;
	{	
		FT_BitmapGlyph Bitmap = (FT_BitmapGlyph)Glyph;
		if( Bitmap->bitmap.buffer != NULL )
		{
			pImage = new ImgImage();
		
			BcU32 W = Bitmap->bitmap.width;
			BcU32 H = Bitmap->bitmap.rows;
			BcU32 Pitch = Bitmap->bitmap.pitch;
		
			ImgColour ClearColour = { 0, 0, 0, 0 };
			pImage->create( W + DoubleBorderSize, H + DoubleBorderSize, &ClearColour );
			
			if( RenderMode == FT_RENDER_MODE_MONO )
			{
				for( BcU32 iY = 0; iY < H; ++iY )
				{
					BcU8* pRow = Bitmap->bitmap.buffer + ( iY * Pitch );
					BcU8 Mask = 128;
					for( BcU32 iX = 0; iX < W; ++iX )
					{
						BcU32 BitSet = *pRow & Mask;
						Mask >>= 1;
						
						ImgColour Colour = 
						{
							255, 255, 255, BitSet ? 255 : 0
						};
						
						pImage->setPixel( iX + BorderSize, iY + BorderSize, Colour );
						
						//  Advance row.
						if( Mask == 0 )
						{
							Mask = 128;
							++pRow;
						}
					}
				}
			}
			else if( RenderMode == FT_RENDER_MODE_NORMAL )
			{
				for( BcU32 iY = 0; iY < H; ++iY )
				{
					BcU8* pRow = Bitmap->bitmap.buffer + ( iY * Pitch );
					for( BcU32 iX = 0; iX < W; ++iX )
					{
						BcU32 Pixel = *pRow;
						
						ImgColour Colour = 
						{
							255, 255, 255, Pixel
						};
						
						pImage->setPixel( iX + BorderSize, iY + BorderSize, Colour );
						
						++pRow;
					}
				}									
			}
		}
	}
	
	return pImage;
}

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnFont::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const std::string& FileName = Object[ "source" ].asString();
	
	// Add root dependancy.
	DependancyList.push_back( CsDependancy( FileName ) );

	FT_Library	Library;
	FT_Face		Face;
	
	BcBool DistanceField = Object[ "distancefield" ].asBool();
	BcU32 NominalSize = Object[ "nominalsize" ].asInt() * ( DistanceField ? 8 : 1 );
	BcU32 BorderSize = DistanceField ? Object[ "spread" ].asInt(): 1;
	
	int Error;
	
	// Initialise free type.
	Error = FT_Init_FreeType( &Library );
	if( Error == 0 )
	{
		// Create new face.
		Error = FT_New_Face( Library,
							 FileName.c_str(),
							 0,
							 &Face );
		
		if( Error == 0 )
		{		
			// Set pixel size for font map.
			Error = FT_Set_Char_Size( Face,
									  0,
									  NominalSize * 64,
									  72,
									  72 );
			
			if( Error == 0 )
			{
				/*
				// Set pixel sizes.
				Error = FT_Set_Pixel_Sizes( Face,
										    0,
										    NominalSize );
				
				 */
				if( Error == 0 )
				{
					// List of glyph descs.			
					typedef std::vector< TGlyphDesc > TGlyphDescList;
					TGlyphDescList GlyphDescList;
					
					// List of glyph images.
					ImgImageList GlyphImageList;
					
					// Keep a map of what glyph indices we have already (they can be duplicated)
					std::map< FT_UInt, BcBool > HaveGlyph;
					
					// Iterate over glyphs (A-Z for now.)
					for( BcU32 CharCode = 0; CharCode <= 255; ++CharCode )
					{
						FT_UInt GlyphIndex = FT_Get_Char_Index( Face, CharCode );
						
						if( CharCode == ' ' )
						{
							int a =0;
							++a;
						}
						if( HaveGlyph.find( GlyphIndex ) == HaveGlyph.end() )
						{
							HaveGlyph[ GlyphIndex ] = BcTrue;
							int GlyphError = FT_Load_Glyph( Face, GlyphIndex, 0 ); 
							if( GlyphError == 0 )
							{
								FT_Render_Mode RenderMode = DistanceField ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL;
								FT_Glyph Glyph;
								GlyphError = FT_Get_Glyph( Face->glyph, &Glyph );
								
								if( GlyphError == 0 )
								{
									int ConvertError = FT_Glyph_To_Bitmap( &Glyph,
																		  RenderMode,
																		  0,
																		  0 );
									FT_BitmapGlyph BitmapGlyph = (FT_BitmapGlyph)(Glyph);
									
									ImgImage* pImage = makeImageForGlyph( Glyph, RenderMode, BorderSize );
							
									BcReal GlyphScale = DistanceField ? 0.125f : 1.0f;
									
									// Convert to distance field, and scale down 4x.
									if( DistanceField == BcTrue && pImage != NULL )
									{									
										BcU32 Width = pImage->width();
										BcU32 Height = pImage->height();
										BcU32 WidthPot = BcPotNext( Width );
										BcU32 HeightPot = BcPotNext( Height );
										
										ImgColour FillColour = { 0, 0, 0, 0 };
										
										// Distance field.
										ImgImage* pDistanceFieldImage = pImage->generateDistanceField( 128, (BcReal)BorderSize );
										
										// Power of 2 round up.
										ImgImage* pPowerOfTwo = pDistanceFieldImage->canvasSize( WidthPot, HeightPot, &FillColour );
										
										// Scale down 8x.
										ImgImage* pScale1_2 = pPowerOfTwo->resize( WidthPot >> 1, HeightPot >> 1 );
										ImgImage* pScale1_4 = pScale1_2->resize( WidthPot >> 2, HeightPot >> 2 );			
										ImgImage* pScale1_8 = pScale1_4->resize( WidthPot >> 3, HeightPot >> 3 );			
										
										// Crop to final size.
										ImgImage* pFinal = pScale1_8->cropByColour( FillColour, BcFalse );
										
										// Clean up.
										delete pImage;
										delete pDistanceFieldImage;
										delete pPowerOfTwo;
										delete pScale1_2;
										delete pScale1_4;
										delete pScale1_8;
										
										// Assign final image.
										pImage = pFinal;
									}
									
									GlyphImageList.push_back( pImage );										
										
									// Add glyph descriptor.
									TGlyphDesc GlyphDesc = 
									{
										0.0f, 0.0f, 0.0f, 0.0f, // UVs, fill in later.
										
										( (BcReal)BitmapGlyph->left + BorderSize ) * GlyphScale,
										( (BcReal)BitmapGlyph->top + BorderSize ) * GlyphScale,
										BitmapGlyph->bitmap.width * GlyphScale,
										BitmapGlyph->bitmap.rows * GlyphScale,
										( (BcReal)( Glyph->advance.x >> 16 ) + (BcReal)( Glyph->advance.x & 0xffff ) / 65536.0f ) * GlyphScale,
									
										CharCode
									};
									
									GlyphDescList.push_back( GlyphDesc );
								}
								
								// Done with this glyph.
								FT_Done_Glyph( Glyph );
							}
						}
					}
					
					// 
					
					// Create an atlas of glyphs.
					ImgRectList RectList;
					ImgImage* pAtlasImage = ImgImage::generateAtlas( GlyphImageList, RectList, 1024, 1024 );
					
					// Create a texture.
					std::string FontTextureName = Object[ "name" ].asString() + "fonttextureatlas";
					std::string FontTextureFileName = std::string( "IntermediateContent/" ) + FontTextureName + ".png";
					Img::save( FontTextureFileName.c_str(), pAtlasImage );
										
					// Setup texture object, and import.
					Json::Value TextureObject;		
					TextureObject[ "ScnTexture" ][ "name" ] = FontTextureName;
					TextureObject[ "ScnTexture" ][ "source" ] = FontTextureFileName;		
					
					// NOTE: Need a better solution for this. Don't want to reimport this texture.
					CsDependancyList TextureDependancyList;
					
					// Attempt to import texture.
					ScnTextureRef TextureRef;
					if( CsCore::pImpl()->importObject( TextureObject, TextureRef, TextureDependancyList ) )
					{
						// Build data.
						BcStream HeaderStream;
						BcStream GlyphStream;
						
						THeader Header;
						
						Header.NoofGlyphs_ = GlyphDescList.size();
						BcStrCopyN( Header.TextureName_, FontTextureName.c_str(), sizeof( Header.TextureName_ ) );

						HeaderStream << Header;
						
						// Setup glyph desc UVs & serialise.
						for( BcU32 Idx = 0; Idx < GlyphDescList.size(); ++Idx )
						{
							TGlyphDesc& GlyphDesc = GlyphDescList[ Idx ];
							ImgRect& Rect = RectList[ Idx ];
								
							GlyphDesc.UA_ = BcReal( Rect.X_ ) / BcReal( pAtlasImage->width() );
							GlyphDesc.VA_ = BcReal( Rect.Y_ ) / BcReal( pAtlasImage->height() );
							GlyphDesc.UB_ = ( Rect.X_ + Rect.W_ ) / BcReal( pAtlasImage->width() );
							GlyphDesc.VB_ = ( Rect.Y_ + Rect.H_ ) / BcReal( pAtlasImage->height() );
						
							GlyphDesc.Width_ = BcReal( Rect.W_ );
							GlyphDesc.Height_ = BcReal( Rect.H_ );
							
							GlyphStream << GlyphDesc;
						}
						
						// Write out chunks.											
						pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
						pFile_->addChunk( BcHash( "glyphs" ), GlyphStream.pData(), GlyphStream.dataSize() );
						
						// Delete all images.
						for( BcU32 Idx = 0; Idx < GlyphImageList.size(); ++Idx )
						{
							delete GlyphImageList[ Idx ];
						}
						GlyphImageList.clear();
						delete pAtlasImage;

						
						return BcTrue;
					}
				}
				else
				{
					BcPrintf( "ScnFont: Error setting pixel sizes.\n" );
				}
			}
			else
			{
				BcPrintf( "ScnFont: Error setting char size.\n" );
			}
		}
		else
		{
			BcPrintf( "ScnFont: Error loading font %s.\n", FileName.c_str() );
		}
	}
	else
	{
		BcPrintf( "ScnFont: Error initialising freetype2.\n" );
	}
	
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFont );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnFont::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnFont" )
		.field( "source",					csPVT_FILE,			csPCT_VALUE )
		.field( "distancefield",			csPVT_BOOL,			csPCT_VALUE )
		.field( "nominalsize",				csPVT_UINT,			csPCT_VALUE )
		.field( "spread",					csPVT_UINT,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnFont::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnFont::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnFont::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnFont::isReady()
{
	return pHeader_ != NULL && pGlyphDescs_ != NULL && Texture_.isReady();
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcBool ScnFont::createInstance( const std::string& Name, ScnFontComponentRef& FontComponent, ScnMaterialRef Material )
{	
	return CsCore::pImpl()->createResource( Name, FontComponent, this, Material );
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnFont::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnFont::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		getChunk( 0 );
		return;
	}

	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		
		// Get glyph desc chunk.
		getChunk( ++ChunkIdx );
		
		// Request texture.
		CsCore::pImpl()->requestResource( pHeader_->TextureName_, Texture_ );
	}
	else if( pChunk->ID_ == BcHash( "glyphs" ) )
	{
		pGlyphDescs_ = (TGlyphDesc*)pData;
	
		// Create a char code map.
		for( BcU32 Idx = 0; Idx < pHeader_->NoofGlyphs_; ++Idx )
		{
			TGlyphDesc* pGlyph = &pGlyphDescs_[ Idx ];
			CharCodeMap_[ pGlyph->CharCode_ ] = Idx;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnFontComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnFontComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "ScnFontComponent" )
		//.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnFontComponent::initialise( ScnFontRef Parent, ScnMaterialRef Material )
{
	Parent_ = Parent; 
	if( Material->createComponent( getName().getValue(), MaterialComponent_, scnSPF_DEFAULT ) )
	{	
		BcU32 Parameter = MaterialComponent_->findParameter( "aDiffuseTex" );
		if( Parameter != BcErrorCode )
		{
			MaterialComponent_->setTexture( Parameter, Parent_->Texture_ );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcVec2d ScnFontComponent::draw( ScnCanvasComponentRef Canvas, const std::string& String, BcBool SizeRun )
{
	// Cached elements from parent.
	ScnFont::TCharCodeMap& CharCodeMap( Parent_->CharCodeMap_ );
	ScnFont::TGlyphDesc* pGlyphDescs = Parent_->pGlyphDescs_;
	
	// Allocate enough vertices for each character.
	ScnCanvasComponentVertex* pFirstVert = SizeRun ? NULL : Canvas->allocVertices( String.length() * 6 );
	ScnCanvasComponentVertex* pVert = pFirstVert;

	// Zero the buffer.
	if( pFirstVert != NULL )
	{
		BcMemZero( pFirstVert, String.length() * 6 * sizeof( ScnCanvasComponentVertex ) );
	}
	
	BcU32 NoofVertices = 0;
	
	BcReal AdvanceX = 0.0f;
	BcReal AdvanceY = 0.0f;
		
	BcU32 RGBA = 0xffffffff;

	BcVec2d MinSize( 1e16f, 1e16f );
	BcVec2d MaxSize( -1e16f, -1e16f );
	
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
				AdvanceY += 16.0f; // TODO: Take from header.
			}
			
			// Find glyph.
			ScnFont::TCharCodeMapIterator Iter = CharCodeMap.find( CharCode );
			
			if( Iter != CharCodeMap.end() )
			{
				ScnFont::TGlyphDesc* pGlyph = &pGlyphDescs[ (*Iter).second ];
				
				const BcReal X1 = AdvanceX + pGlyph->OffsetX_;
				const BcReal Y1 = AdvanceY - pGlyph->OffsetY_;
				const BcReal X2 = X1 + pGlyph->Width_;
				const BcReal Y2 = Y1 + pGlyph->Height_;

				MinSize.x( BcMin( MinSize.x(), X1 ) );
				MinSize.y( BcMin( MinSize.y(), Y1 ) );
				MaxSize.x( BcMax( MaxSize.x(), X1 ) );
				MaxSize.y( BcMax( MaxSize.y(), Y1 ) );
				
				MinSize.x( BcMin( MinSize.x(), X2 ) );
				MinSize.y( BcMin( MinSize.y(), Y2 ) );
				MaxSize.x( BcMax( MaxSize.x(), X2 ) );
				MaxSize.y( BcMax( MaxSize.y(), Y2 ) );
				if( SizeRun == BcFalse )
				{
					// Add triangle for character.
					pVert->X_ = X1;
					pVert->Y_ = Y1;
					pVert->U_ = pGlyph->UA_;
					pVert->V_ = pGlyph->VA_;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = X2;
					pVert->Y_ = Y1;
					pVert->U_ = pGlyph->UB_;
					pVert->V_ = pGlyph->VA_;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = X1;
					pVert->Y_ = Y2;
					pVert->U_ = pGlyph->UA_;
					pVert->V_ = pGlyph->VB_;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = X2;
					pVert->Y_ = Y1;
					pVert->U_ = pGlyph->UB_;
					pVert->V_ = pGlyph->VA_;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = X2;
					pVert->Y_ = Y2;
					pVert->U_ = pGlyph->UB_;
					pVert->V_ = pGlyph->VB_;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = X1;
					pVert->Y_ = Y2;
					pVert->U_ = pGlyph->UA_;
					pVert->V_ = pGlyph->VB_;
					pVert->RGBA_ = RGBA;
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
			Canvas->addPrimitive( rsPT_TRIANGLELIST, pFirstVert, NoofVertices, 0 );
		}
	}
	else
	{
		BcPrintf( "ScnFontComponent: Out of vertices!\n" );
	}

	return MaxSize - MinSize;
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
BcBool ScnFontComponent::isReady()
{
	return Parent_->isReady() && MaterialComponent_.isReady();
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
void ScnFontComponent::update( BcReal Tick )
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

	//
	ScnComponent::onDetach( Parent );
}
