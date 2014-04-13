/**************************************************************************
*
* File:		ImgImage.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#include "ImgImage.h"

#include "Base/BcVec2d.h"
#include "Base/BcVec3d.h"
#include "Base/BcVec4d.h"

#include "squish.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ImgImage::ImgImage()
{
	Width_ = 0;
	Height_ = 0;
	pPixelData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ImgImage::~ImgImage()
{
	delete [] pPixelData_;
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
ImgImage::ImgImage( const ImgImage& Original )
{
	Width_ = 0;
	Height_ = 0;
	pPixelData_ = NULL;

	//
	BcAssertMsg( pPixelData_ != NULL, "ImgImage: Pixel data is NULL." );
 
	create( Original.Width_, Original.Height_ );
	
	if( Original.pPixelData_ != NULL )
	{
		BcMemCopy( pPixelData_, Original.pPixelData_, sizeof( ImgColour ) * Width_ * Height_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// create
void ImgImage::create( BcU32 Width, BcU32 Height, const ImgColour* pFillColour )
{
	Width_ = Width;
	Height_ = Height;
	pPixelData_ = new ImgColour[ Width * Height ];
	
	if( pFillColour != NULL )
	{
		for( BcU32 i = 0; i < Width_; ++i )
		{
			for( BcU32 j = 0; j < Height_; ++j )
			{
				setPixel( i, j, *pFillColour );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setPixel
void ImgImage::setPixel( BcU32 X, BcU32 Y, const ImgColour& Colour )
{
	BcAssertMsg( pPixelData_ != NULL, "ImgImage: Pixel data is NULL." );
	if( X < Width_ && Y < Height_ )
	{
		BcU32 Index = X + ( Y * Width_ );
	
		pPixelData_[ Index ] = Colour;
	}
}

//////////////////////////////////////////////////////////////////////////
// getPixel
const ImgColour& ImgImage::getPixel( BcU32 X, BcU32 Y ) const
{
	BcAssertMsg( pPixelData_ != NULL, "ImgImage: Pixel data is NULL." );
	if( X < Width_ && Y < Height_ )
	{
		BcU32 Index = X + ( Y * Width_ );
		return pPixelData_[ Index ];
	}

	static ImgColour NullColour = { 0, 0, 0, 0 };
	return NullColour;
}

//////////////////////////////////////////////////////////////////////////
// blit
void ImgImage::blit( ImgImage* pImage, const ImgRect& SrcRect, const ImgRect& DstRect )
{
	for( BcU32 Y = 0; Y < SrcRect.H_; ++Y )
	{
		for( BcU32 X = 0; X < SrcRect.W_; ++X )
		{
			setPixel( DstRect.X_ + X, DstRect.Y_ + Y, pImage->getPixel( SrcRect.X_ + X, SrcRect.Y_ + Y ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// resize
ImgImage* ImgImage::resize( BcU32 Width, BcU32 Height )
{
	// Create image.
	ImgImage* pImage = new ImgImage();
	pImage->create( Width, Height );

	if( Width != ( Width_ >> 1 ) || Height != ( Height_ >> 1 ) )
	{
		BcF32 SrcW = BcF32( Width_ - 1 );
		BcF32 SrcH = BcF32( Height_ - 1 );

		BcBreakpoint; // This is broken!

		// Bilinear filtering implementation.
		for( BcU32 iX = 0; iX < Width; ++iX )
		{
			BcF32 iXF = BcF32( iX ) / BcF32( Width );
			BcF32 iSrcXF = SrcW * iXF;
			BcU32 iSrcX = BcU32( iSrcXF );
			BcF32 iLerpX = iSrcXF - BcF32( iSrcX );

			for( BcU32 iY = 0; iY < Height; ++iY )
			{
				BcF32 iYF = BcF32( iY ) / BcF32( Height );
				BcF32 iSrcYF = SrcW * iYF;
				BcU32 iSrcY = BcU32( iSrcYF );
				BcF32 iLerpY = iSrcYF - BcF32( iSrcY );

				const ImgColour& PixelA = getPixel( iSrcX, iSrcY );
				const ImgColour& PixelB = getPixel( iSrcX + 1, iSrcY );
				const ImgColour& PixelC = getPixel( iSrcX, iSrcY + 1 );
				const ImgColour& PixelD = getPixel( iSrcX + 1, iSrcY + 1 );

				ImgColour DstPixelT;
				ImgColour DstPixelB;
				ImgColour DstPixel;

				DstPixelT.R_ = BcU8( ( BcF32( PixelA.R_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelB.R_ ) * iLerpX ) );
				DstPixelT.G_ = BcU8( ( BcF32( PixelA.G_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelB.G_ ) * iLerpX ) );
				DstPixelT.B_ = BcU8( ( BcF32( PixelA.B_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelB.B_ ) * iLerpX ) );
				DstPixelT.A_ = BcU8( ( BcF32( PixelA.A_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelB.A_ ) * iLerpX ) );
				DstPixelB.R_ = BcU8( ( BcF32( PixelC.R_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelD.R_ ) * iLerpX ) );
				DstPixelB.G_ = BcU8( ( BcF32( PixelC.G_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelD.G_ ) * iLerpX ) );
				DstPixelB.B_ = BcU8( ( BcF32( PixelC.B_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelD.B_ ) * iLerpX ) );
				DstPixelB.A_ = BcU8( ( BcF32( PixelC.A_ ) * ( 1.0f - iLerpX ) ) + ( BcF32( PixelD.A_ ) * iLerpX ) );

				DstPixel.R_ = BcU8( ( BcF32( DstPixelT.R_ ) * ( 1.0f - iLerpY ) ) + ( BcF32( DstPixelB.R_ ) * iLerpY ) );
				DstPixel.G_ = BcU8( ( BcF32( DstPixelT.G_ ) * ( 1.0f - iLerpY ) ) + ( BcF32( DstPixelB.G_ ) * iLerpY ) );
				DstPixel.B_ = BcU8( ( BcF32( DstPixelT.B_ ) * ( 1.0f - iLerpY ) ) + ( BcF32( DstPixelB.B_ ) * iLerpY ) );
				DstPixel.A_ = BcU8( ( BcF32( DstPixelT.A_ ) * ( 1.0f - iLerpY ) ) + ( BcF32( DstPixelB.A_ ) * iLerpY ) );		

				pImage->setPixel( iX, iY, DstPixel );
			}
		}
	}
	else
	{
		// Better implementation for downscaling.
		for( BcU32 iX = 0; iX < Width; ++iX )
		{
			BcU32 iSrcX = iX << 1;
			for( BcU32 iY = 0; iY < Height; ++iY )
			{
				BcU32 iSrcY = iY << 1;

				const ImgColour& PixelA = getPixel( iSrcX, iSrcY );
				const ImgColour& PixelB = getPixel( iSrcX + 1, iSrcY );
				const ImgColour& PixelC = getPixel( iSrcX, iSrcY + 1 );
				const ImgColour& PixelD = getPixel( iSrcX + 1, iSrcY + 1 );

				ImgColour DstPixel;

				DstPixel.R_ = BcU8( ( BcU32( PixelA.R_ ) +
				                      BcU32( PixelB.R_ ) +
				                      BcU32( PixelC.R_ ) +
				                      BcU32( PixelD.R_ ) ) >> 2 );
				DstPixel.G_ = BcU8( ( BcU32( PixelA.G_ ) +
				                      BcU32( PixelB.G_ ) +
				                      BcU32( PixelC.G_ ) +
				                      BcU32( PixelD.G_ ) ) >> 2 );
				DstPixel.B_ = BcU8( ( BcU32( PixelA.B_ ) +
				                      BcU32( PixelB.B_ ) +
				                      BcU32( PixelC.B_ ) +
				                      BcU32( PixelD.B_ ) ) >> 2 );
				DstPixel.A_ = BcU8( ( BcU32( PixelA.A_ ) +
				                      BcU32( PixelB.A_ ) +
				                      BcU32( PixelC.A_ ) +
				                      BcU32( PixelD.A_ ) ) >> 2 );

				pImage->setPixel( iX, iY, DstPixel );
			}
		}
	}

	return pImage;
}

//////////////////////////////////////////////////////////////////////////
// canvasSize
ImgImage* ImgImage::canvasSize( BcU32 Width, BcU32 Height, const ImgColour* pFillColour )
{
	BcU32 CopyW = BcMin( width(), Width );
	BcU32 CopyH = BcMin( height(), Height );

	ImgImage* pImage = new ImgImage();

	pImage->create( Width, Height, pFillColour );

	for( BcU32 iY = 0; iY < CopyH; ++iY )
	{
		for( BcU32 iX = 0; iX < CopyW; ++iX )
		{
			pImage->setPixel( iX, iY, getPixel( iX, iY ) );
		}
	}
	
	return pImage;
}

//////////////////////////////////////////////////////////////////////////
// generateMipMaps
ImgImage* ImgImage::cropByColour( const ImgColour& Colour, BcBool PowerOfTwo )
{
	// Find bounds.
	BcU32 EndX = 0;
	BcU32 EndY = 0;
	
	for( BcU32 iY = 0; iY < Height_; ++iY )
	{
		for( BcU32 iX = 0; iX < Width_; ++iX )
		{
			const ImgColour& SrcColour = getPixel( iX, iY );
			
			if( SrcColour != Colour )
			{
				EndX = BcMax( EndX, iX );
				EndY = BcMax( EndY, iY );
			}
		}
	}
	
	// Return a new canvas.
	if( PowerOfTwo )
	{
		return canvasSize( BcPotNext( EndX ), BcPotNext( EndY ), &Colour );
	}
	else
	{
		return canvasSize( EndX, EndY, &Colour );
	}
}

//////////////////////////////////////////////////////////////////////////
// generateMipMaps
BcU32 ImgImage::generateMipMaps( BcU32 NoofLevels, ImgImage** ppOutImages )
{
	BcU32 LevelsCreated = 1;

	// Assign first as ourself.
	*ppOutImages = this;

	ImgImage* pPrevImage = *ppOutImages;
	++ppOutImages;

	// Generate smaller images.
	for( BcU32 i = 0; i < ( NoofLevels - 1 ); ++i )
	{
		BcU32 W = pPrevImage->width() >> 1;
		BcU32 H = pPrevImage->height() >> 1;

		// Bail if target is too small.
		if( W < 8 || H < 8 )
		{
			break;
		}

		// Perform resize.
		*ppOutImages = pPrevImage->resize( W, H );
		pPrevImage = *ppOutImages;
		++ppOutImages;
		++LevelsCreated;
	}

	return LevelsCreated;
}

//////////////////////////////////////////////////////////////////////////
// generateDistanceField
static const BcVec2d DistanceFieldInside = BcVec2d( 0.0f, 0.0f );
static const BcVec2d DistanceFieldOutside = BcVec2d( 1e6f, 1e6f );

ImgImage* ImgImage::generateDistanceField( BcU32 IntensityThreshold, BcF32 Spread )
{
	// Utility funcs.
	class DistanceField
	{
	public:
		class Grid
		{
		public:
			BcU32 Width_;
			BcU32 Height_;
			BcVec2d* pCells_;
			
		public:
			Grid( BcU32 W, BcU32 H )
			{
				Width_ = W;
				Height_ = H;
				pCells_ = new BcVec2d[ W * H ];
			}
			
			~Grid()
			{
				delete [] pCells_;
			}
			
			const BcVec2d& getCell( BcS32 X, BcS32 Y ) const
			{
				if( X < 0 || X >= (BcS32)Width_ || Y < 0 || Y >= (BcS32)Height_ )
				{
					return DistanceFieldOutside;
				}
				return pCells_[ X + Y * Width_ ];
			}
			
			void setCell( BcS32 X, BcS32 Y, const BcVec2d& Value )
			{
				if( X >= 0 && X < (BcS32)Width_ && Y >= 0 && Y < (BcS32)Height_ )
				{
					pCells_[ X + Y * Width_ ] = Value;
				}
			}
		};
		
		static void SetupGrids( ImgImage* pImage, BcU32 IntensityThreshold, Grid& GridA, Grid& GridB )
		{
			for( BcS32 Y = 0; Y < (BcS32)GridA.Height_; ++Y )
			{
				for( BcS32 X = 0; X < (BcS32)GridB.Width_; ++X )
				{
					const ImgColour& SubPixel = pImage->getPixel( X, Y );
					BcU32 Intensity = (BcU32)SubPixel.A_;
					BcBool PixelSet = Intensity > IntensityThreshold;
					GridA.setCell( X, Y, PixelSet ? DistanceFieldOutside : DistanceFieldInside );
					GridB.setCell( X, Y, PixelSet ? DistanceFieldInside : DistanceFieldOutside );
				}
			}			
		}
		
		static BcInline BcVec2d Compare( Grid& GridX, const BcVec2d& Cell, BcS32 X, BcS32 Y, BcS32 OffsetX, BcS32 OffsetY )
		{
			BcVec2d OtherCell = GridX.getCell( X + OffsetX, Y + OffsetY );
			OtherCell = BcVec2d( OtherCell.x() + OffsetX, OtherCell.y() + OffsetY );
			return ( OtherCell.magnitudeSquared() < Cell.magnitudeSquared() ) ? OtherCell : Cell;
		}
		
		static void Propagate( Grid& GridX )
		{
			const BcS32 Width = GridX.Width_;
			const BcS32 Height = GridX.Height_;
			for( BcS32 Y = 0; Y < Height; ++Y )
			{
				for( BcS32 X = 0; X < Width; ++X )
				{
					BcVec2d Cell = GridX.getCell( X, Y );
					Cell = Compare( GridX, Cell, X, Y, -1,  0 );
					Cell = Compare( GridX, Cell, X, Y,  0, -1 );
					Cell = Compare( GridX, Cell, X, Y, -1, -1 );
					Cell = Compare( GridX, Cell, X, Y,  1, -1 );
					GridX.setCell( X, Y, Cell );
				}
				
				for( BcS32 X = Width - 1; X > 0; --X )
				{
					BcVec2d Cell = GridX.getCell( X, Y );
					Cell = Compare( GridX, Cell, X, Y,  1,  0 );
					GridX.setCell( X, Y, Cell );
				}
			}

			for( BcS32 Y = Height - 1; Y > 0; --Y )
			{
				for( BcS32 X = Width - 1; X > 0; --X )
				{
					BcVec2d Cell = GridX.getCell( X, Y );
					Cell = Compare( GridX, Cell, X, Y,  1,  0 );
					Cell = Compare( GridX, Cell, X, Y,  0,  1 );
					Cell = Compare( GridX, Cell, X, Y,  1,  1 );
					Cell = Compare( GridX, Cell, X, Y, -1,  1 );
					GridX.setCell( X, Y, Cell );
				}
				
				for( BcS32 X = 0; X < Width; ++X )
				{
					BcVec2d Cell = GridX.getCell( X, Y );
					Cell = Compare( GridX, Cell, X, Y, -1,  0 );
					GridX.setCell( X, Y, Cell );
				}
			}
		}
		
		static void Subtract( Grid& SignedDistanceGrid, const Grid& GridA, const Grid& GridB )
		{
			const BcU32 Width = SignedDistanceGrid.Width_;
			const BcU32 Height = SignedDistanceGrid.Height_;

			for( BcS32 Y = 0; Y < (BcS32)Height; ++Y )
			{
				for( BcS32 X = 0; X < (BcS32)Width; ++X )
				{
					const BcVec2d& CellA = GridA.getCell( X, Y );
					const BcVec2d& CellB = GridB.getCell( X, Y );
					const BcF32 DistA = CellA.magnitude();
					const BcF32 DistB = CellB.magnitude();
					const BcF32 SignedDistance = DistA - DistB;
					SignedDistanceGrid.setCell( X, Y, BcVec2d( SignedDistance, 0.0f ) );
				}
			}
		}
		
		static void Normalise( ImgImage* pOutputImage, Grid& SignedDistanceGrid, BcF32 Spread )
		{
			const BcU32 Width = SignedDistanceGrid.Width_;
			const BcU32 Height = SignedDistanceGrid.Height_;
			BcF32 MinDistance = -Spread;
			BcF32 MaxDistance = Spread;
			
			for( BcS32 Y = 0; Y < (BcS32)Height; ++Y )
			{
				for( BcS32 X = 0; X < (BcS32)Width; ++X )
				{
					BcF32 Distance = SignedDistanceGrid.getCell( X, Y ).x();
					if( Distance < 0.0f )
					{
						Distance = -128.0f * ( Distance - MinDistance ) / MinDistance;
					}
					else
					{
						Distance = 128.0f + 128.0f * Distance / MaxDistance;
					}
					
					BcU32 DistanceInt = (BcU32)BcClamp( Distance, 0.0f, 255.0f );
					
					ImgColour Colour = { 255, 255, 255, DistanceInt };
					pOutputImage->setPixel( X, Y, Colour );
				}
			}
		}
	};
	
	// Create image.
	ImgImage* pImage = new ImgImage();
	pImage->create( width(), height() );

	// Allocate grids.
	DistanceField::Grid GridA( width(), height() );	
	DistanceField::Grid GridB( width(), height() );
	DistanceField::Grid SignedDistanceGrid( width(), height() );
	
	// Setup grids.
	DistanceField::SetupGrids( this, IntensityThreshold, GridA, GridB );
	
	// Propagate.
	DistanceField::Propagate( GridA );
	DistanceField::Propagate( GridB );
	
	// Subtract grids.
	DistanceField::Subtract( SignedDistanceGrid, GridA, GridB );
	
	// Normalise.
	DistanceField::Normalise( pImage, SignedDistanceGrid, Spread );	
	
	// Return image.
	return pImage;
}

//////////////////////////////////////////////////////////////////////////
// generateAtlas
struct ImgIndexedImage
{
	BcU32 Idx_;
	ImgImage* pImage_;
};

typedef std::vector< ImgIndexedImage > ImgIndexedImageList;
typedef ImgIndexedImageList::iterator ImgIndexedImageListIterator;

class ImgIndexedImageSortWidthCompare
{
public:
	bool operator()( const ImgIndexedImage& A, const ImgIndexedImage& B )
	{
		BcU32 ImageA = A.pImage_ ? A.pImage_->width() : 0;
		BcU32 ImageB = B.pImage_ ? B.pImage_->width() : 0;
		return ImageA > ImageB;
	}
};

class ImgIndexedImageSortHeightCompare
{
public:
	bool operator()( const ImgIndexedImage& A, const ImgIndexedImage& B )
	{
		BcU32 ImageA = A.pImage_ ? A.pImage_->height() : 0;
		BcU32 ImageB = B.pImage_ ? B.pImage_->height() : 0;
		return ImageA > ImageB;
	}
};

ImgImage* ImgImage::generateAtlas( ImgImageList& ImageList, ImgRectList& OutRectList, BcU32 Width, BcU32 Height, ImgColour& ClearColour )
{
	// Utility funcs.
	class Atlas
	{
	public:
		static ImgIndexedImageList SortImageList( ImgImageList& ImageList )
		{
			// Copy into indexed list.
			ImgIndexedImageList IndexedImageList;
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgIndexedImage IndexedImage =
				{
					Idx,
					ImageList[ Idx ]
				};
				
				IndexedImageList.push_back( IndexedImage );
			}
			
			std::sort( IndexedImageList.begin(), IndexedImageList.end(), ImgIndexedImageSortHeightCompare() );
			std::sort( IndexedImageList.begin(), IndexedImageList.end(), ImgIndexedImageSortWidthCompare() );
		
			return IndexedImageList;
		}
		
		static ImgRect FindMinimumBounds( const ImgImageList& ImageList, BcBool PowerOfTwo )
		{
			ImgRect Bounds = { 0, 0, 65536, 65536 };
			for( ImgImageListConstIterator Iter( ImageList.begin() ); Iter != ImageList.end(); ++Iter )
			{
				ImgImage* pImage = (*Iter);
				
				if( pImage != NULL )
				{
					Bounds.W_ = BcMin( Bounds.W_, pImage->width() );
					Bounds.H_ = BcMin( Bounds.H_, pImage->height() );
				}
			}
			
			if( PowerOfTwo )
			{
				Bounds.W_ = BcPotNext( Bounds.W_ );
				Bounds.H_ = BcPotNext( Bounds.H_ );
			}
			
			return Bounds;
		}
		
		static ImgRect FindMaximumBounds( const ImgImageList& ImageList, BcBool PowerOfTwo )
		{
			ImgRect Bounds = { 0, 0, 0, 0 };
			for( ImgImageListConstIterator Iter( ImageList.begin() ); Iter != ImageList.end(); ++Iter )
			{
				ImgImage* pImage = (*Iter);
				
				if( pImage != NULL )
				{			
					Bounds.W_ = BcMax( Bounds.W_, pImage->width() );
					Bounds.H_ = BcMax( Bounds.H_, pImage->height() );
				}
			}
			
			if( PowerOfTwo )
			{
				Bounds.W_ = BcPotNext( Bounds.W_ );
				Bounds.H_ = BcPotNext( Bounds.H_ );
			}
			
			return Bounds;
		}
		
		static BcBool IsRectEmpty( const ImgRectList& RectList, const ImgRect& Rect )
		{
			// Iterate over all rects, and if we aren't inside a single one the rect is free.
			for( ImgRectListConstIterator Iter( RectList.begin() ); Iter != RectList.end(); ++Iter )
			{
				const ImgRect& CurrRect = (*Iter);
							
				if( !Rect.isOutsideOf( CurrRect ) )
				{
					return BcFalse;
				}
			}
			
			return BcTrue;
		}
		
		static BcBool FindEmptyRect( const ImgRectList& RectList, const ImgRect& ImageBounds, ImgRect& Rect, BcU32 StepSize )
		{
			// Start from the edge of the image, and iterate along until we fine a free
			// space to place it.
			// NOTE: This will be hideously slow. It's quick and dirty for the mean time.
			if( Rect.W_ > ImageBounds.W_ ||
			    Rect.H_ > ImageBounds.H_ )
			{
				return BcFalse;
			}

			// If wider than high pack along the X, and visa versa.
			// This keeps helps the atlas to close off diagonally.
			if( Rect.W_ > Rect.H_ )
			{		
				for( BcU32 Y = 0; Y < ImageBounds.H_ - Rect.H_; Y += StepSize )
				{
					for( BcU32 X = 0; X < ImageBounds.W_ - Rect.W_; X += StepSize )
					{
						Rect.X_ = X;
						Rect.Y_ = Y;
					
						if( IsRectEmpty( RectList, Rect ) )
						{
							return BcTrue;
						}					
					}
				}
			}
			else
			{
				for( BcU32 X = 0; X < ImageBounds.W_ - Rect.W_; X += StepSize )
				{
					for( BcU32 Y = 0; Y < ImageBounds.H_ - Rect.H_; Y += StepSize )
					{
						Rect.X_ = X;
						Rect.Y_ = Y;
						
						if( IsRectEmpty( RectList, Rect ) )
						{
							return BcTrue;
						}					
					}
				}
			}
			return BcFalse;
		}
	};

	// Find maximum bounds for a single image in the atlas.
	ImgRect ImageBounds = Atlas::FindMaximumBounds( ImageList, BcTrue );

	// Calculate a step size to use.
	// TODO: Determine a better algorithm for this.
	//ImgRect MinStepSize = Atlas::FindMinimumBounds( ImageList, BcTrue );
	//BcU32 StepSize = BcMax( (BcU32)1, BcMin( MinStepSize.W_, MinStepSize.H_ ) / 4 );
	BcU32 StepSize = 4;
	
	// Sort image list into an indexed list.
	ImgIndexedImageList IndexedImageList = Atlas::SortImageList( ImageList );
	ImgRectList IndexedRectList;
	
	// Iterate over all images and attempt to create rects for them.
	BcU32 ExpandAxis = 0;
	for( ImgIndexedImageListIterator ImgIter( IndexedImageList.begin() ); ImgIter != IndexedImageList.end(); ++ImgIter )
	{
		const ImgImage* pImage = (*ImgIter).pImage_;
		ImgRect ImageRect = { 0, 0, pImage ? pImage->width() : 0, pImage ? pImage->height() : 0 };
		if( pImage != NULL )
		{
			while( Atlas::FindEmptyRect( IndexedRectList, ImageBounds, ImageRect, StepSize ) == BcFalse )
			{
				// Expand an axis.
				switch( ExpandAxis )
				{
					case 0:
						ImageBounds.W_ = BcPotNext( ImageBounds.W_ + 1 );
						break;
					case 1:
						ImageBounds.H_ = BcPotNext( ImageBounds.H_ + 1 );
						break;
				}
			
				// Swap axis to expand.
				ExpandAxis = 1 - ExpandAxis;
			}
		}
		
		// Add rect to out list.
		IndexedRectList.push_back( ImageRect );
	}
	
	// Create out rect list from indexed rect list.
	OutRectList = IndexedRectList;
	for( BcU32 Idx = 0; Idx < IndexedImageList.size(); ++Idx )
	{
		BcU32 OutIdx = IndexedImageList[ Idx ].Idx_;
		OutRectList[ OutIdx ] = IndexedRectList[ Idx ];
	}
	
	// Create image.
	ImgImage* pAtlasImage = new ImgImage();
	pAtlasImage->create( ImageBounds.W_, ImageBounds.H_, &ClearColour );

	// Blit all images into it using output rects.
	for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
	{
		ImgImage* pImage = ImageList[ Idx ];
		if( pImage != NULL )
		{
			const ImgRect& DstRect = OutRectList[ Idx ];
			ImgRect SrcRect = { 0, 0, pImage->width(), pImage->height() };

			pAtlasImage->blit( pImage, SrcRect, DstRect );
		}
	}
	
	return pAtlasImage;
}

//////////////////////////////////////////////////////////////////////////
// width
BcU32 ImgImage::width() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
BcU32 ImgImage::height() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// getImageData
const ImgColour* ImgImage::getImageData() const
{
	return pPixelData_;
}


//////////////////////////////////////////////////////////////////////////
// hasAlpha
BcBool ImgImage::hasAlpha( BcU8 Threshold ) const
{
	BcU32 NoofPixels = Width_ * Height_;

	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		ImgColour& Colour = pPixelData_[ Idx ];

		if( Colour.A_ < Threshold )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getImageData
BcBool ImgImage::encodeAs( ImgEncodeFormat Format, BcU8*& pOutData, BcU32& OutSize )
{
	BcAssert( pOutData == NULL );
	BcAssert( OutSize == 0 );

	switch( Format )
	{
	case imgEF_RGB8:
		return encodeAsRGB8( pOutData, OutSize );

	case imgEF_RGBA8:
		return encodeAsRGBA8( pOutData, OutSize );

	case imgEF_BGR8:
		return encodeAsBGR8( pOutData, OutSize );

	case imgEF_ABGR8:
		return encodeAsABGR8( pOutData, OutSize );

	case imgEF_I8:
		return encodeAsI8( pOutData, OutSize );

	case imgEF_DXT1:
	case imgEF_DXT3:
	case imgEF_DXT5:
		return encodeAsDXT( Format, pOutData, OutSize );
		
	default:
		return BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// encodeAsRGB8
BcBool ImgImage::encodeAsRGB8( BcU8*& pOutData, BcU32& OutSize )
{
	BcU32 NoofPixels = Width_ * Height_;
	OutSize = NoofPixels * 3;
	pOutData = new BcU8[ OutSize ];
	
	BcU8* pCurrByte = pOutData;
	ImgColour* pColour = pPixelData_;

#define WRITE_PIXEL \
		*pCurrByte++ = pColour->R_; \
		*pCurrByte++ = pColour->G_; \
		*pCurrByte++ = pColour->B_; \
		++pColour

#if 0
	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		WRITE_PIXEL;
	}
#else
	register BcS32 Count = (BcS32)NoofPixels;
	{
		register BcS32 N = ( Count + 7 ) / 8;

		switch( Count % 8 )
		{
		case 0: do {	WRITE_PIXEL;
		case 7:			WRITE_PIXEL;
		case 6:			WRITE_PIXEL;
		case 5:			WRITE_PIXEL;
		case 4:			WRITE_PIXEL;
		case 3:			WRITE_PIXEL;
		case 2:			WRITE_PIXEL;
		case 1:			WRITE_PIXEL;
				} while( --N > 0 );
		}
	}
#endif
#undef WRITE_PIXEL

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// encodeAsRGBA8
BcBool ImgImage::encodeAsRGBA8( BcU8*& pOutData, BcU32& OutSize )
{
	BcU32 NoofPixels = Width_ * Height_;
	OutSize = NoofPixels * 4;
	pOutData = new BcU8[ OutSize ];
	
	BcU8* pCurrByte = pOutData;
	ImgColour* pColour = pPixelData_;

	BcU32 TotalWritten = 0;

#define WRITE_PIXEL \
		*pCurrByte++ = pColour->R_; \
		*pCurrByte++ = pColour->G_; \
		*pCurrByte++ = pColour->B_; \
		*pCurrByte++ = pColour->A_; \
		++pColour; TotalWritten += 4

#if 1
	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		WRITE_PIXEL;
	}
#else
	register BcS32 Count = (BcS32)NoofPixels;
	{
		register BcS32 N = ( Count + 7 ) / 8;

		switch( Count % 8 )
		{
		case 0: do {	WRITE_PIXEL;
		case 7:			WRITE_PIXEL;
		case 6:			WRITE_PIXEL;
		case 5:			WRITE_PIXEL;
		case 4:			WRITE_PIXEL;
		case 3:			WRITE_PIXEL;
		case 2:			WRITE_PIXEL;
		case 1:			WRITE_PIXEL;
				} while( --N > 0 );
		}
	}
#endif
#undef WRITE_PIXEL

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// encodeAsBGR8
BcBool ImgImage::encodeAsBGR8( BcU8*& pOutData, BcU32& OutSize )
{
	BcU32 NoofPixels = Width_ * Height_;
	OutSize = NoofPixels * 3;
	pOutData = new BcU8[ OutSize ];
	
	BcU8* pCurrByte = pOutData;
	ImgColour* pColour = pPixelData_;

#define WRITE_PIXEL \
		*pCurrByte++ = pColour->B_; \
		*pCurrByte++ = pColour->G_; \
		*pCurrByte++ = pColour->R_; \
		++pColour

#if 0
	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		WRITE_PIXEL;
	}
#else
	register BcS32 Count = (BcS32)NoofPixels;
	{
		register BcS32 N = ( Count + 7 ) / 8;

		switch( Count % 8 )
		{
		case 0: do {	WRITE_PIXEL;
		case 7:			WRITE_PIXEL;
		case 6:			WRITE_PIXEL;
		case 5:			WRITE_PIXEL;
		case 4:			WRITE_PIXEL;
		case 3:			WRITE_PIXEL;
		case 2:			WRITE_PIXEL;
		case 1:			WRITE_PIXEL;
				} while( --N > 0 );
		}
	}
#endif
#undef WRITE_PIXEL

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// encodeAsABGR8
BcBool ImgImage::encodeAsABGR8( BcU8*& pOutData, BcU32& OutSize )
{
	BcU32 NoofPixels = Width_ * Height_;
	OutSize = NoofPixels * 4;
	pOutData = new BcU8[ OutSize ];
	
	BcU8* pCurrByte = pOutData;
	ImgColour* pColour = pPixelData_;

#define WRITE_PIXEL \
		*pCurrByte++ = pColour->A_; \
		*pCurrByte++ = pColour->B_; \
		*pCurrByte++ = pColour->G_; \
		*pCurrByte++ = pColour->R_; \
		++pColour

#if 0
	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		WRITE_PIXEL;
	}
#else
	register BcS32 Count = (BcS32)NoofPixels;
	{
		register BcS32 N = ( Count + 7 ) / 8;

		switch( Count % 8 )
		{
		case 0: do {	WRITE_PIXEL;
		case 7:			WRITE_PIXEL;
		case 6:			WRITE_PIXEL;
		case 5:			WRITE_PIXEL;
		case 4:			WRITE_PIXEL;
		case 3:			WRITE_PIXEL;
		case 2:			WRITE_PIXEL;
		case 1:			WRITE_PIXEL;
				} while( --N > 0 );
		}
	}
#endif
#undef WRITE_PIXEL

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// encodeAsI8
BcBool ImgImage::encodeAsI8( BcU8*& pOutData, BcU32& OutSize )
{
	BcU32 NoofPixels = Width_ * Height_;
	OutSize = NoofPixels;
	pOutData = new BcU8[ OutSize ];
	
	BcU8* pCurrByte = pOutData;
	ImgColour* pColour = pPixelData_;

#define WRITE_PIXEL \
		*pCurrByte++ = (BcU8)BcMax( ( (BcU32)pColour->R_ + (BcU32)pColour->B_ + (BcU32)pColour->G_ ) / 3, (BcU32)255 ); \
		++pColour

#if 0
	for( BcU32 Idx = 0; Idx < NoofPixels; ++Idx )
	{
		WRITE_PIXEL;
	}
#else
	register BcS32 Count = (BcS32)NoofPixels;
	{
		register BcS32 N = ( Count + 7 ) / 8;

		switch( Count % 8 )
		{
		case 0: do {	WRITE_PIXEL;
		case 7:			WRITE_PIXEL;
		case 6:			WRITE_PIXEL;
		case 5:			WRITE_PIXEL;
		case 4:			WRITE_PIXEL;
		case 3:			WRITE_PIXEL;
		case 2:			WRITE_PIXEL;
		case 1:			WRITE_PIXEL;
				} while( --N > 0 );
		}
	}
#endif
#undef WRITE_PIXEL

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// encodeAsDXT
BcBool ImgImage::encodeAsDXT( ImgEncodeFormat Format, BcU8*& pOutData, BcU32& OutSize )
{
	if( Format == imgEF_DXT1 ||
		Format == imgEF_DXT3 ||
		Format == imgEF_DXT5 )
	{
		// Check if its a power of two too.
		if( BcPot( Width_ ) && BcPot( Height_ ) && Width_ >= 4 && Height_ >= 4 )
		{
			BcU32 SquishFormat = 0;

			switch( Format )
			{
			case imgEF_DXT1:
				SquishFormat = squish::kDxt1 | squish::kColourIterativeClusterFit;
				break;
			case imgEF_DXT3:
				SquishFormat = squish::kDxt3 | squish::kColourIterativeClusterFit | squish::kWeightColourByAlpha;
				break;
			case imgEF_DXT5:
				SquishFormat = squish::kDxt5 | squish::kColourIterativeClusterFit | squish::kWeightColourByAlpha;
				break;
			}
			
			// Find out what space squish needs.
			OutSize = squish::GetStorageRequirements( Width_, Height_, SquishFormat );
			pOutData = new BcU8[ OutSize ];
			
			// Squish takes RGBA8, so no need to convert before passing in.
			squish::CompressImage( reinterpret_cast< squish::u8*>( pPixelData_ ), Width_, Height_, pOutData, SquishFormat );
			
			//
			return BcTrue;
		}
	}
	
	return BcFalse;
}
