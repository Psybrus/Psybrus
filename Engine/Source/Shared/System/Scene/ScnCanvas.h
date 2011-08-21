/**************************************************************************
*
* File:		ScnCanvas.h
* Author:	Neil Richardson 
* Ver/Date:	10/04/11	
* Description:
*		Used for 2D rendering.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnCanvas_H__
#define __ScnCanvas_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnTypes.h"
#include "ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnCanvasRef
typedef CsResourceRef< class ScnCanvas > ScnCanvasRef;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasVertex
struct ScnCanvasVertex
{
	BcF32 X_, Y_, Z_;
	BcF32 NX_, NY_, NZ_;
	BcF32 TX_, TY_, TZ_;
	BcF32 U_, V_;
	BcU32 RGBA_;
};

//////////////////////////////////////////////////////////////////////////
// ScnCanvasPrimitiveSection
struct ScnCanvasPrimitiveSection
{
	eRsPrimitiveType		Type_;
	BcU32					VertexIndex_;
	BcU32					NoofVertices_;
	BcU32					Layer_;
	ScnMaterialInstanceRef	MaterialInstance_;
};

class ScnCanvasPrimitiveSectionCompare
{
public:
	bool operator()( const ScnCanvasPrimitiveSection& A, const ScnCanvasPrimitiveSection& B )
	{
		return A.Layer_ < B.Layer_;
	}
};


//////////////////////////////////////////////////////////////////////////
// ScnCanvas
class ScnCanvas:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnCanvas );
	
	virtual void						initialise( BcU32 NoofVertices, ScnMaterialInstanceRef DefaultMaterialInstance );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	/**
	 * Set material instance.
	 */
	void								setMaterialInstance( ScnMaterialInstanceRef MaterialInstance );
	
	/**
	 * Push matrix.
	 * @param Matrix Matrix.
	 */
	void								pushMatrix( const BcMat4d& Matrix );
	
	/*
	 * Pop matrix.
	 */
	void								popMatrix();
	
	/*
	 * Get current matrix.
	 */
	BcMat4d								getMatrix() const;

	/**
	 * Allocate some vertices to use.<br/>
	 * Safe to allocate 0 if you don't know how many vertices you need initially,
	 * and to allocate the total number at the end. Provided you don't overrun the buffer!
	 * @param NoofVertices Number of vertices to allocate.
	 */
	ScnCanvasVertex*					allocVertices( BcU32 NoofVertices );
	
	/**
	 * Add raw primitive.<br/>
	 */
	void								addPrimitive( eRsPrimitiveType Type, ScnCanvasVertex* pVertices, BcU32 NoofVertices, BcU32 Layer = 0, BcBool UseMatrixStack = BcTrue );

	/**
	 * Draw line.
	 * @param PointA Point A
	 * @param PointB Point B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer = 0 );
	
	/**
	 * Draw lines.
	 * @param pPoints Pointer to points.
	 * @param NoofLines Number of lines.
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawLines( const BcVec2d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw box.
	 * @param CornerA Corner A
	 * @param CornerB Corner B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawBox( const BcVec2d& CornerA, const BcVec2d& CornerB, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite.
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void								drawSprite( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite in 3D
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void								drawSprite3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite centered.
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void								drawSpriteCentered( const BcVec2d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite centered in 3D.
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void								drawSpriteCentered3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Clear canvas.
	 */
	void								clear();
	
	/**
	 * Render canvas.<br/>
	 * Can be called multiple times on different, or the same, frames.
	 * @param pFrame Frame to render with.
	 * @param Sort Sort value to use.
	 */
	void								render( RsFrame* pFrame, RsRenderSort Sort );
	
protected:
	BcForceInline BcU32					convertVertexPointerToIndex( ScnCanvasVertex* pVertex )
	{
		// NOTE: Will probably warn due to converting a 64-bit pointer to 32-bit value, but
		//       it's actually ok because we should never have over 4GB worth of vertices!
		BcU32 ByteOffset = BcU32( (BcU8*)pVertex - (BcU8*)pVertices_ );
		return ByteOffset / sizeof( ScnCanvasVertex );
	}
	
protected:
	RsVertexBuffer*						pVertexBuffer_;
	RsPrimitive*						pPrimitive_;
	BcBool								HaveVertexBufferLock_;

	// Submission data.
	ScnCanvasVertex*					pVertices_;
	ScnCanvasVertex*					pVerticesEnd_;
	BcU32								NoofVertices_;
	BcU32								VertexIndex_;
	
	// Materials.
	ScnMaterialInstanceRef				DefaultMaterialInstance_;
	ScnMaterialInstanceRef				MaterialInstance_;
	ScnTextureRef						DiffuseTexture_;

	typedef std::vector< ScnCanvasPrimitiveSection > TPrimitiveSectionList;
	typedef TPrimitiveSectionList::iterator TPrimitiveSectionListIterator;
	
	TPrimitiveSectionList				PrimitiveSectionList_;
	BcU32								LastPrimitiveSection_;

	// Matrix stack.
	typedef std::vector< BcMat4d > TMatrixStack;
	typedef TMatrixStack::iterator TMatrixStackIterator;
	
	TMatrixStack						MatrixStack_;
	BcBool								IsIdentity_;
	
	
};

#endif
