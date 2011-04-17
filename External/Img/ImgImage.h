/**************************************************************************
*
* File:		ImgImage.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#ifndef __IMGIMAGE_H__
#define __IMGIMAGE_H__

//////////////////////////////////////////////////////////////////////////
// Includes
#include "ImgTypes.h"
#include "ImgException.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class ImgImage;

//////////////////////////////////////////////////////////////////////////
// ImgImageList
typedef std::vector< ImgImage* > ImgImageList;
typedef ImgImageList::iterator ImgImageListIterator;
typedef ImgImageList::const_iterator ImgImageListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ImgImage
class ImgImage
{
public:
	ImgImage();
	~ImgImage();

	ImgImage( const ImgImage& Original );

	/**
	*	Create an image.
	*	@param Width Width
	*	@param Height Height
	*	@param Format Format
	*	@param pFillColour Fill colour.
	*/
	void					create( BcU32 Width, BcU32 Height, eImgFormat Format, const ImgColour* pFillColour = NULL );

	/**
	*	Set pixel.
	*	@param X X pos.
	*	@param Y Y pos.
	*	@param Colour Colour to set to.
	*/
	void					setPixel( BcU32 X, BcU32 Y, const ImgColour& Colour );

	/**
	*	Get pixel.
	*	@param X X pos.
	*	@param Y Y pos.
	*/
	const ImgColour&		getPixel( BcU32 X, BcU32 Y ) const;

	/**
	 * Blit.
	 */
	void					blit( ImgImage* pImage, const ImgRect& SrcRect, const ImgRect& DstRect );
	
	/**
	*	Set palette entry.
	*	@param Idx Index.
	*	@param Colour Colour.
	*/
	void					setPalette( BcU32 Idx, const ImgColour& Colour );

	/**
	*	Get palette entry.
	*	@param Idx Index.
	*/
	const ImgColour&		getPalette( BcU32 Idx ) const;

	/**
	*	Find closest colour.
	*	@param Colour to find.
	*/
	ImgColour				findColour( const ImgColour& Colour );

	/**
	*	Resize image.
	*/
	ImgImage*				resize( BcU32 Width, BcU32 Height );

	/**
	 * Canvas size. Original is aligned to left.
	 */
	ImgImage*				canvasSize( BcU32 Width, BcU32 Height, const ImgColour* pFillColour );
	
	/**
	 *	Crop by colour. Only crops from right and bottom!
	 */
	ImgImage*				cropByColour( const ImgColour& Colour, BcBool PowerOfTwo );

	/**
	*	Generate mipmaps.
	*	@return Number of levels generated.
	*/
	BcU32					generateMipMaps( BcU32 NoofLevels, ImgImage** ppOutImages );
	
	/**
	 *	Generate distance field.
	 *	@param IntensityThreshold Intensity threshold to differentiate between 0 & 1.
	 *	@param Spread Spread.
	 *
	 */
	ImgImage*				generateDistanceField( BcU32 IntensityThreshold, BcReal Spread );
	
	/**
	 *	Generate atlas.
	 *	@param ImageList List of images.
	 *	@param OutRectList List of rects (out).
	 *	@param Width Max Width.
	 *	@param Height Max Height.
	 *	@return Atlased image.
	 */	
	static ImgImage*		generateAtlas( ImgImageList& ImageList, ImgRectList& OutRectList, BcU32 Width, BcU32 Height );
	
	/**
	*	Get width.
	*/
	BcU32					width() const;

	/**
	*	Get height.
	*/
	BcU32					height() const;

	/**
	*	Get format.
	*/
	eImgFormat				format() const;
	
	/**
	*	Get image data.
	*/
	const ImgColour*		getImageData() const;

	/**
	*	Release image data.
	*/
	BcU8*					release( BcU32& DataSize );

private:
	BcU32					Width_;
	BcU32					Height_;
	eImgFormat				Format_;

	ImgColour*				pPixelData_;
	ImgColour*				pPaletteEntries_;
	BcU32					NoofPaletteEntries_;
};

#endif
