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

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#include "Import/Img/Img.h"

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
BcBool ScnFont::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();
	
	// Add root dependancy.
	Importer.addDependency( FileName.c_str() );

	FT_Library	Library;
	FT_Face		Face;
	
	BcU32 OriginalNominalSize = Object[ "nominalsize" ].asInt();
	BcBool DistanceField = Object[ "distancefield" ].asBool();
	BcU32 NominalSize = OriginalNominalSize * ( DistanceField ? 4 : 1 );
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
							
									BcReal GlyphScale = DistanceField ? 0.25f : 1.0f;
									
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
										
										// Scale down 4x.
										ImgImage* pScale1_2 = pPowerOfTwo->resize( WidthPot >> 1, HeightPot >> 1 );
										ImgImage* pScale1_4 = pScale1_2->resize( WidthPot >> 2, HeightPot >> 2 );			
										
										// Crop to final size.
										ImgImage* pFinal = pScale1_4->cropByColour( FillColour, BcFalse );
										
										// Clean up.
										delete pImage;
										delete pDistanceFieldImage;
										delete pPowerOfTwo;
										delete pScale1_2;
										delete pScale1_4;
										
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
					
					// Setup texture object for import.
					Json::Value TextureObject;		
					TextureObject[ "name" ] = FontTextureName;
					TextureObject[ "type" ] = "ScnTexture";
					TextureObject[ "source" ] = FontTextureFileName;		
					
					// Add the texture to imports.
					Importer.addImport( TextureObject );

					// Build data.
					BcStream HeaderStream;
					BcStream GlyphStream;
					
					THeader Header;
					
					Header.NoofGlyphs_ = GlyphDescList.size();
					Header.TextureName_ = Importer.addString( FontTextureName.c_str() );
					Header.NominalSize_ = (BcReal)OriginalNominalSize;
					
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
					Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
					Importer.addChunk( BcHash( "glyphs" ), GlyphStream.pData(), GlyphStream.dataSize() );
						
					// Delete all images.
					for( BcU32 Idx = 0; Idx < GlyphImageList.size(); ++Idx )
					{
						delete GlyphImageList[ Idx ];
					}
					GlyphImageList.clear();
					delete pAtlasImage;

						
					return BcTrue;
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
	return CsCore::pImpl()->createResource( BcName::INVALID, FontComponent, this, Material );
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
		pHeader_ = (THeader*)pData;
		
		// Get glyph desc chunk.
		requestChunk( ++ChunkIdx );
		
		// Request texture.
		CsCore::pImpl()->requestResource( /* WIP */ getPackageName(), getString( pHeader_->TextureName_ ), Texture_ );
	}
	else if( ChunkID == BcHash( "glyphs" ) )
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
// initialise
void ScnFontComponent::initialise( ScnFontRef Parent, ScnMaterialRef Material )
{
	Parent_ = Parent; 
	if( CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent_, Material, scnSPF_DEFAULT ) )
	{	
		BcU32 Parameter = MaterialComponent_->findParameter( "aDiffuseTex" );
		if( Parameter != BcErrorCode )
		{
			MaterialComponent_->setTexture( Parameter, Parent_->Texture_ );
		}
	}

	// Disable clipping.
	setClipping( BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// setClipping
void ScnFontComponent::setClipping( BcBool Enabled, BcVec2d Min, BcVec2d Max )
{
	ClippingEnabled_ = Enabled;
	ClipMin_ = Min;
	ClipMax_ = Max;
}

//////////////////////////////////////////////////////////////////////////
// isReady
BcVec2d ScnFontComponent::draw( ScnCanvasComponentRef Canvas, const BcVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun )
{
	// Cached elements from parent.
	ScnFont::THeader* pHeader = Parent_->pHeader_;
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
		
	BcU32 RGBA = Colour.asABGR();

	BcVec2d MinSize( Position );
	BcVec2d MaxSize( Position );
	
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
				ScnFont::TGlyphDesc* pGlyph = &pGlyphDescs[ (*Iter).second ];

				// Bring first character back to the left so it sits on the cursor.
				if( FirstCharacterOnLine )
				{
					AdvanceX -= pGlyph->OffsetX_;
					//AdvanceY -= pGlyph->OffsetY_ + pHeader->NominalSize_;
					FirstCharacterOnLine = BcFalse;
				}
				
				// Calculate size and UVs.
				BcVec2d Size( BcVec2d( pGlyph->Width_, pGlyph->Height_ ) );
				BcVec2d CornerMin( Position + BcVec2d( AdvanceX + pGlyph->OffsetX_, AdvanceY - pGlyph->OffsetY_ + pHeader->NominalSize_ ) );
				BcVec2d CornerMax( CornerMin + Size );
				BcReal U0 = pGlyph->UA_;
				BcReal V0 = pGlyph->VA_;
				BcReal U1 = pGlyph->UB_;
				BcReal V1 = pGlyph->VB_;
				
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

						BcReal TexWidth = U1 - U0;
						BcReal TexHeight = V1 - V0;

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
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMin.y();
					pVert->U_ = U1;
					pVert->V_ = V0;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = CornerMin.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U0;
					pVert->V_ = V1;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMin.y();
					pVert->U_ = U1;
					pVert->V_ = V0;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = CornerMax.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U1;
					pVert->V_ = V1;
					pVert->RGBA_ = RGBA;
					++pVert;
					
					pVert->X_ = CornerMin.x();
					pVert->Y_ = CornerMax.y();
					pVert->U_ = U0;
					pVert->V_ = V1;
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
