/**************************************************************************
*
* File:		ScnFontImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnFontImport.h"
#include "System/Scene/Import/ScnTextureImport.h"
#include "System/Scene/Rendering/ScnFontFileData.h"
#include "Base/BcFile.h"
#include "Base/BcMath.h"
#include "Base/BcStream.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnFontImport )
	
void ScnFontImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnFontImport::Source_, bcRFF_IMPORTER ),
		new ReField( "NominalSize_", &ScnFontImport::NominalSize_, bcRFF_IMPORTER ),
		new ReField( "DistanceField_", &ScnFontImport::DistanceField_, bcRFF_IMPORTER ),
		new ReField( "Spread_", &ScnFontImport::Spread_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnFontImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnFontImport::ScnFontImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnFontImport::ScnFontImport( ReNoInit )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnFontImport::~ScnFontImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnFontImport::import(
	const Json::Value& )
{
	// Add root dependancy.
	CsResourceImporter::addDependency( Source_.c_str() );

	FT_Library	Library;
	FT_Face		Face;
	
	BcU32 OriginalNominalSize = NominalSize_;
	NominalSize_ = OriginalNominalSize * ( DistanceField_ ? 4 : 1 );
	BcU32 BorderSize = DistanceField_ ? Spread_ : 1;
	
	int Error;
	
	// Initialise free type.
	Error = FT_Init_FreeType( &Library );
	if( Error == 0 )
	{
		// Create new face.
		Error = FT_New_Face( Library,
							 Source_.c_str(),
							 0,
							 &Face );
		
		if( Error == 0 )
		{		
			// Set pixel size for font map.
			Error = FT_Set_Char_Size( Face,
									  0,
									  NominalSize_ * 64,
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
					typedef std::vector< ScnFontGlyphDesc > ScnFontGlyphDescList;
					ScnFontGlyphDescList GlyphDescList;
					
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
								FT_Render_Mode RenderMode = DistanceField_ ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL;
								FT_Glyph Glyph;
								GlyphError = FT_Get_Glyph( Face->glyph, &Glyph );
								
								if( GlyphError == 0 )
								{
									int ConvertError = FT_Glyph_To_Bitmap( &Glyph,
																		  RenderMode,
																		  0,
																		  0 );
									FT_BitmapGlyph BitmapGlyph = (FT_BitmapGlyph)(Glyph);
									
									ImgImageUPtr pImage = RenderMode == FT_RENDER_MODE_MONO ? 
										makeImageForGlyphMono( Glyph, BorderSize ) :
										makeImageForGlyphNormal( Glyph, BorderSize );
							
									BcF32 GlyphScale = DistanceField_ ? 0.25f : 1.0f;
									
									// Convert to distance field, and scale down 4x.
									if( DistanceField_ == BcTrue && pImage != nullptr )
									{									
										BcU32 Width = pImage->width();
										BcU32 Height = pImage->height();
										BcU32 WidthPot = BcPotNext( Width );
										BcU32 HeightPot = BcPotNext( Height );
										
										ImgColour FillColour = { 0, 0, 0, 0 };
										
										// Distance field.
										ImgImageUPtr pDistanceFieldImage = pImage->generateDistanceField( 128, (BcF32)BorderSize );
										
										// Power of 2 round up.
										ImgImageUPtr pPowerOfTwo = pDistanceFieldImage->canvasSize( WidthPot, HeightPot, &FillColour );
										
										// Scale down 4x.
										ImgImageUPtr pScale1_2 = pPowerOfTwo->resize( WidthPot >> 1, HeightPot >> 1 );
										ImgImageUPtr pScale1_4 = pScale1_2->resize( WidthPot >> 2, HeightPot >> 2 );			
										
										// Crop to final size.
										ImgImageUPtr pFinal = pScale1_4->cropByColour( FillColour, BcFalse );
																				
										// Assign final image.
										pImage = std::move( pFinal );
									}
									
									GlyphImageList.push_back( std::move( pImage ) );
										
									// Add glyph descriptor.
									ScnFontGlyphDesc GlyphDesc = 
									{
										0.0f, 0.0f, 0.0f, 0.0f, // UVs, fill in later.
										
										( (BcF32)BitmapGlyph->left + BorderSize ) * GlyphScale,
										( (BcF32)BitmapGlyph->top + BorderSize ) * GlyphScale,
										BitmapGlyph->bitmap.width * GlyphScale,
										BitmapGlyph->bitmap.rows * GlyphScale,
										( (BcF32)( Glyph->advance.x >> 16 ) + (BcF32)( Glyph->advance.x & 0xffff ) / 65536.0f ) * GlyphScale,
									
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
					ImgColour ClearColour = { 0, 0, 0, 0 };
					ImgImageUPtr pAtlasImage = ImgImage::generateAtlas( GlyphImageList, RectList, 512, 512, ClearColour );
					
					// Create texture.
					std::string FontTextureName = Name_ + "fonttextureatlas";
					std::string FontTextureFileName = getIntermediatePath() + std::string( "/" ) + FontTextureName + ".png";
					Img::save( FontTextureFileName.c_str(), pAtlasImage.get() );
					
					// Create texture importer..
					// NOTE: This is pretty horrible. The reason is
					//       that reflected objects use BcMemAlign, which
					//       is not used by global new.
					//       I found a bug with the std::thread and std::mutex
					//       code in VS2012 that uses its own custom allocator
					//       for new in debug builds. This annoyed me greatly.
					auto TextureImporter = CsResourceImporterUPtr(
						new ScnTextureImport( 
							FontTextureName, "ScnTexture",
							FontTextureFileName, RsTextureFormat::R8G8B8A8 ) );
					
					// Build data.
					BcStream HeaderStream;
					BcStream GlyphStream;
					
					ScnFontHeader Header;
					
					Header.NoofGlyphs_ = (BcU32)GlyphDescList.size();
					Header.TextureRef_ = CsResourceImporter::addImport( std::move( TextureImporter ) );
					Header.NominalSize_ = (BcF32)OriginalNominalSize;
					
					HeaderStream << Header;
					
					// Setup glyph desc UVs & serialise.
					for( BcU32 Idx = 0; Idx < GlyphDescList.size(); ++Idx )
					{
						ScnFontGlyphDesc& GlyphDesc = GlyphDescList[ Idx ];
						ImgRect& Rect = RectList[ Idx ];
								
						GlyphDesc.UA_ = BcF32( Rect.X_ ) / BcF32( pAtlasImage->width() );
						GlyphDesc.VA_ = BcF32( Rect.Y_ ) / BcF32( pAtlasImage->height() );
						GlyphDesc.UB_ = ( Rect.X_ + Rect.W_ ) / BcF32( pAtlasImage->width() );
						GlyphDesc.VB_ = ( Rect.Y_ + Rect.H_ ) / BcF32( pAtlasImage->height() );
						
						GlyphDesc.Width_ = BcF32( Rect.W_ );
						GlyphDesc.Height_ = BcF32( Rect.H_ );
							
						GlyphStream << GlyphDesc;
					}
						
					// Write out chunks.											
					CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
					CsResourceImporter::addChunk( BcHash( "glyphs" ), GlyphStream.pData(), GlyphStream.dataSize() );
						
					// Delete all images.
					GlyphImageList.clear();
					pAtlasImage = nullptr;
						
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
			BcPrintf( "ScnFont: Error loading font %s.\n", Source_.c_str() );
		}
	}
	else
	{
		BcPrintf( "ScnFont: Error initialising freetype2.\n" );
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// makeImageForGlyphMono
ImgImageUPtr ScnFontImport::makeImageForGlyphMono( struct FT_GlyphRec_* Glyph, BcU32 BorderSize )
{
	ImgImageUPtr pImage = nullptr;
	BcU32 DoubleBorderSize = BorderSize * 2;
	{	
		FT_BitmapGlyph Bitmap = (FT_BitmapGlyph)Glyph;
		if( Bitmap->bitmap.buffer != NULL )
		{
			pImage = ImgImageUPtr( new ImgImage() );
		
			BcU32 W = Bitmap->bitmap.width;
			BcU32 H = Bitmap->bitmap.rows;
			BcU32 Pitch = Bitmap->bitmap.pitch;
		
			ImgColour ClearColour = { 0, 0, 0, 0 };
			pImage->create( W + DoubleBorderSize, H + DoubleBorderSize, &ClearColour );
			
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
	}
	
	return pImage;
}

//////////////////////////////////////////////////////////////////////////
// makeImageForGlyphNormal
ImgImageUPtr ScnFontImport::makeImageForGlyphNormal( struct FT_GlyphRec_* Glyph, BcU32 BorderSize )
{
	ImgImageUPtr pImage = nullptr;
	BcU32 DoubleBorderSize = BorderSize * 2;
	{	
		const FT_BitmapGlyph& Bitmap = (FT_BitmapGlyph)Glyph;
		if( Bitmap->bitmap.buffer != NULL )
		{
			pImage = ImgImageUPtr( new ImgImage() );
		
			BcU32 W = Bitmap->bitmap.width;
			BcU32 H = Bitmap->bitmap.rows;
			BcU32 Pitch = Bitmap->bitmap.pitch;
		
			ImgColour ClearColour = { 0, 0, 0, 0 };
			pImage->create( W + DoubleBorderSize, H + DoubleBorderSize, &ClearColour );
			
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
	
	return pImage;
}
