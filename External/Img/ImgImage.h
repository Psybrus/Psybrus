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
	void					create( BcU32 Width, BcU32 Height, const ImgColour* pFillColour = NULL );

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
	*	Get image data.
	*/
	const ImgColour*		getImageData() const;

	/**
	*	Has image got alpha?
	*	@param Threshold How low the value has to be to consider it having alpha.
	*/
	BcBool					hasAlpha( BcU8 Threshold ) const;

	/**
	 * Encode image as specified format.
	 */
	BcBool					encodeAs( ImgEncodeFormat Format, BcU8*& pOutData, BcU32& OutSize );

private:
	BcBool					encodeAsRGB8( BcU8*& pOutData, BcU32& OutSize );
	BcBool					encodeAsRGBA8( BcU8*& pOutData, BcU32& OutSize );
	BcBool					encodeAsBGR8( BcU8*& pOutData, BcU32& OutSize );
	BcBool					encodeAsABGR8( BcU8*& pOutData, BcU32& OutSize );
	BcBool					encodeAsI8( BcU8*& pOutData, BcU32& OutSize );
	BcBool					encodeAsDXT( ImgEncodeFormat Format, BcU8*& pOutData, BcU32& OutSize );

private:
	BcU32					Width_;
	BcU32					Height_;
	ImgColour*				pPixelData_;
};

#endif
