/**************************************************************************
*
* File:		Img.cpp
* Author:	Neil Richardson 
* Ver/Date:	17/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Img.h"

#include "png.h"

//////////////////////////////////////////////////////////////////////////
// load
//static
ImgImage* Img::load( const BcChar* Filename )
{
	if( BcStrStr( Filename, ".png" ) )
	{
		return loadPNG( Filename );
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// load
//static
BcBool Img::save( const BcChar* Filename, ImgImage* pImage )
{
	if( BcStrStr( Filename, ".png" ) )
	{
		return savePNG( Filename, pImage );
	}
	
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// loadPNG
void PngRead( png_structp pPngRead, png_bytep pData, png_size_t Length )
{
	FILE* pFile = ( FILE* ) png_get_io_ptr( pPngRead );
	fread( pData, 1, Length, pFile );
}

//static
ImgImage* Img::loadPNG( const BcChar* Filename )
{
	// Open file.
	FILE* pFile = fopen( Filename, "rb" );
	
	if( pFile != NULL )
	{
		// Initial setup.
		png_structp pPngRead = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		BcAssertException( pPngRead != NULL, ImgException( "ImgLoader: Can't create read struct" ) );
		
		png_infop pPngInfo = png_create_info_struct( pPngRead );
		BcAssertException( pPngRead != NULL, ImgException( "ImgLoader: Can't create read struct" ) );
		
		png_infop pPngEndInfo = png_create_info_struct( pPngRead );
		BcAssertException( pPngRead != NULL, ImgException( "ImgLoader: Can't create read struct" ) );
		
		// Set error handling
		if ( setjmp( png_jmpbuf( pPngRead ) ) )
		{
			png_destroy_read_struct( &pPngRead, &pPngInfo, &pPngEndInfo );
			throw ImgException( "ImgLoader: Internal libpng error." );
		}
		
		BcChar Header[ 8 ];
		fread( &Header[0], 1, 8, pFile );
		
		//
		png_set_sig_bytes( pPngRead, 8 );
		png_set_read_fn( pPngRead, pFile, &PngRead );
		png_read_info( pPngRead, pPngInfo );
		
		// Get image info.
		png_uint_32 PngWidth;
		png_uint_32 PngHeight;
		int PngBitDepth;
		int PngColourType;
		int PngInterlaceMethod;
		int PngCompressionMethod;
		int PngFilterMethod;
		
		png_get_IHDR( pPngRead, pPngInfo, &PngWidth, &PngHeight, &PngBitDepth, &PngColourType, &PngInterlaceMethod, &PngCompressionMethod, &PngFilterMethod );
		
		// Read the palette if present
		png_colorp pPngPalette = NULL;
		int NoofPngPaletteEntries = 0;
		if ( png_get_valid( pPngRead, pPngInfo, PNG_INFO_PLTE ) )
		{
			png_get_PLTE( pPngRead, pPngInfo, &pPngPalette, &NoofPngPaletteEntries );
		}
		
		// Read the transparency data if present
		png_bytep pPngPaletteAlphaData = NULL;
		png_color_16p pPngAlphaData = NULL;
		int NoofPngAlphaEntries = 0;
		if ( png_get_valid( pPngRead, pPngInfo, PNG_INFO_tRNS ) )
		{
			png_get_tRNS( pPngRead, pPngInfo, &pPngPaletteAlphaData, &NoofPngAlphaEntries, &pPngAlphaData );
		}
		
		// Colour format.
		eImgFormat Format;
		
		switch( PngColourType )
		{
			case PNG_COLOR_TYPE_PALETTE:
				BcBreakpoint;
				break;
				
			case PNG_COLOR_TYPE_RGB_ALPHA:
				Format = imgFMT_RGBA;
				break;
				
			case PNG_COLOR_TYPE_GRAY:
			case PNG_COLOR_TYPE_GRAY_ALPHA:
			case PNG_COLOR_TYPE_RGB:
			case PNG_COLOR_MASK_PALETTE:
				Format = imgFMT_RGB;
				break;
		}
		
		// Make sure we get 8 bit out.
		if ( PngBitDepth == 16 )
		{
			png_set_strip_16( pPngRead );
		}
		if ( PngBitDepth < 8 )
		{
			png_set_packing( pPngRead );
		}
		
		// Update ready for read.
		png_read_update_info( pPngRead, pPngInfo );
		
		// TODO: Palette load in.
		
		// Image.
		png_bytepp pPngRowPointers = new png_bytep[ PngHeight ];
		
		BcU32 ImageRowSize = PngWidth;
		if ( ( Format == imgFMT_RGBA ) )
		{
			ImageRowSize *= 4;
		}
		else if ( Format == imgFMT_RGB )
		{
			ImageRowSize *= 3;
		}
		
		BcU32 ImageSize = ImageRowSize * PngHeight;
		png_bytep pPngImageData = new png_byte[ ImageSize ];
		
		for ( BcU32 iRow = 0; iRow < PngHeight; ++iRow )
		{
			pPngRowPointers[ iRow ] = pPngImageData + ( iRow * ImageRowSize );
		}
		
		png_read_image( pPngRead, pPngRowPointers );
		
		// Copy over.
		ImgImage* pImage = new ImgImage();
		pImage->create( PngWidth, PngHeight, Format );
		
		for( BcU32 iRow = 0; iRow < PngHeight; ++iRow )
		{
			BcU8* pPixel = pPngRowPointers[ iRow ];
			
			for( BcU32 iCol = 0; iCol < PngWidth; ++iCol )
			{
				ImgColour Pixel;
				
				switch( Format )
				{
					case imgFMT_RGB:
						Pixel.R_ = *pPixel++;
						Pixel.G_ = *pPixel++;
						Pixel.B_ = *pPixel++;
						Pixel.A_ = 255;
						break;
						
					case imgFMT_RGBA:
						Pixel.R_ = *pPixel++;
						Pixel.G_ = *pPixel++;
						Pixel.B_ = *pPixel++;
						Pixel.A_ = *pPixel++;
						break;
						
					case imgFMT_INDEXED:
						BcBreakpoint;
						break;
				}
				
				pImage->setPixel( iCol, iRow, Pixel );
			}
		}
		
		// Free png.
		png_destroy_read_struct( &pPngRead, &pPngInfo, &pPngEndInfo );
		
		// Close file.
		fclose( pFile );
		
		// Delete the PNG stuff.
		delete [] pPngImageData;
		delete [] pPngRowPointers;
		
		//
		return pImage;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// savePNG
void PngWrite( void* pPngWrite, void* pData, unsigned int Length )
{
	FILE* pFile = ( FILE* ) png_get_io_ptr( (png_structp) pPngWrite );
	fwrite( pData, 1, Length, pFile );
}

void PngWriteFlush( void* pPngWrite )
{
}

//static
BcBool Img::savePNG( const BcChar* Filename, ImgImage* pImage )
{
	png_structp pPngWrite;
	png_infop pPngInfo;

	// Open file.
	FILE* pFile = fopen( Filename, "wb+" );

	if( pFile == NULL )
	{
		return BcFalse;
	}
	
	// Initial setup.
	pPngWrite = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if ( pPngWrite == NULL )
	{
		return BcFalse;
	}
	
	pPngInfo = png_create_info_struct( pPngWrite );
	if ( pPngInfo == NULL )
	{
		png_destroy_write_struct( &pPngWrite, ( png_infopp ) NULL );
		return BcFalse;
	}
	
	if ( setjmp( png_jmpbuf( pPngWrite ) ) )
	{
		png_destroy_write_struct( &pPngWrite, &pPngInfo );
		return BcFalse;
	}
	
	png_rw_ptr pWrite = ( png_rw_ptr ) &PngWrite;
	png_flush_ptr pFlush = ( png_flush_ptr ) &PngWriteFlush;
	png_set_write_fn( pPngWrite, pFile, pWrite, pFlush );
	
	png_set_filter( pPngWrite, 0, PNG_FILTER_NONE );
	
	png_set_compression_level( pPngWrite, Z_DEFAULT_COMPRESSION );
	
	// Set up the header
	int PngColType = 0;
	int PngBitDepth = 0;
	switch( pImage->format() )
	{
		case imgFMT_INDEXED:
			PngColType = PNG_COLOR_TYPE_PALETTE;
			PngBitDepth = 8;
			BcBreakpoint; // TODO.
			break;
		case imgFMT_RGB:
			PngColType = PNG_COLOR_TYPE_RGB;
			PngBitDepth = 8;
			break;
		case imgFMT_RGBA:
			PngColType = PNG_COLOR_TYPE_RGB_ALPHA;
			PngBitDepth = 8;
			break;
	}
	
	png_set_IHDR( pPngWrite, pPngInfo,
				  pImage->width(),
				  pImage->height(),
				  PngBitDepth,
				  PngColType,
				  PNG_INTERLACE_NONE,
				  PNG_COMPRESSION_TYPE_DEFAULT,
				  PNG_FILTER_TYPE_DEFAULT );
	
	if ( pImage->format() == imgFMT_INDEXED )
	{
		png_color PngPalette[ 256 ];
		png_byte PngAlpha[ 256 ];
		
		for ( int iPal = 0; iPal < 256; ++iPal )
		{
			const ImgColour& Colour = pImage->getPalette( iPal );
			PngPalette[ iPal ].red = Colour.R_;
			PngPalette[ iPal ].green = Colour.G_;
			PngPalette[ iPal ].blue = Colour.B_;
			PngAlpha[ iPal ] = Colour.A_;
		}
		
		png_set_PLTE( pPngWrite, pPngInfo, &PngPalette[ 0 ], 256 );
		png_set_tRNS( pPngWrite, pPngInfo, &PngAlpha[ 0 ], 256, NULL );
	}
	
	// Write out.
	png_set_packing( pPngWrite );
	png_write_info( pPngWrite, pPngInfo );
	
	// Set up the row pointer from the original data
	png_bytepp ppRows = ( png_bytepp ) malloc( sizeof( png_bytep ) * pImage->height() );
	const ImgColour* pPixels = pImage->getImageData();
	for ( int iY = 0; iY < pImage->height(); ++iY )
	{
		ppRows[ iY ] = ( png_bytep ) ( pPixels + iY );
	}
	
	// Write the image
	png_write_image( pPngWrite, ppRows );
	
	// Tidy up
	png_write_end( pPngWrite, pPngInfo );
	png_destroy_write_struct( &pPngWrite, &pPngInfo );
	free( ppRows );
	
	fclose( pFile );
	
	return BcTrue;	
}

