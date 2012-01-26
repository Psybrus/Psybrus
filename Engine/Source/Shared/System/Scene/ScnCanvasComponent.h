/**************************************************************************
*
* File:		ScnCanvasComponent.h
* Author:	Neil Richardson 
* Ver/Date:	10/04/11	
* Description:
*		Used for 2D rendering.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnCanvasComponentComponent_H__
#define __ScnCanvasComponentComponent_H__

#include "RsCore.h"
#include "ScnRenderableComponent.h"

#include "ScnTypes.h"
#include "ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentRef
typedef CsResourceRef< class ScnCanvasComponent > ScnCanvasComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentVertex
struct ScnCanvasComponentVertex
{
	BcF32 X_, Y_, Z_;
	BcF32 U_, V_;
	BcU32 RGBA_;
};

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentPrimitiveSection
struct ScnCanvasComponentPrimitiveSection
{
	eRsPrimitiveType		Type_;
	BcU32					VertexIndex_;
	BcU32					NoofVertices_;
	BcU32					Layer_;
	ScnMaterialComponentRef	MaterialComponent_;
};

class ScnCanvasComponentPrimitiveSectionCompare
{
public:
	bool operator()( const ScnCanvasComponentPrimitiveSection& A, const ScnCanvasComponentPrimitiveSection& B )
	{
		return A.Layer_ < B.Layer_;
	}
};


//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponent
class ScnCanvasComponent:
	public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, ScnCanvasComponent );
	
	virtual void						initialise( BcU32 NoofVertices, ScnMaterialComponentRef DefaultMaterialComponent );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	/**
	 * Get material component.
	 */
	ScnMaterialComponentRef				getMaterialComponent();

	/**
	 * Set material component.
	 */
	void								setMaterialComponent( ScnMaterialComponentRef MaterialComponent );
	
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
	ScnCanvasComponentVertex*			allocVertices( BcU32 NoofVertices );
	
	/**
	 * Add raw primitive.<br/>
	 */
	void								addPrimitive( eRsPrimitiveType Type, ScnCanvasComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer = 0, BcBool UseMatrixStack = BcTrue );

	/**
	 * Draw line.
	 * @param PointA Point A
	 * @param PointB Point B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawLine( const BcVec2d& PointA, const BcVec2d& PointB, const RsColour& Colour, BcU32 Layer = 0 );
	void								drawLine3d( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour, BcU32 Layer = 0 );

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
	void								drawSpriteUp3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

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
	void								drawSpriteCenteredUp3D( const BcVec3d& Position, const BcVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Clear canvas.
	 */
	void								clear();
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	virtual void						render( RsFrame* pFrame, RsRenderSort Sort );


protected:
	BcForceInline BcU32					convertVertexPointerToIndex( ScnCanvasComponentVertex* pVertex )
	{
		// NOTE: Will probably warn due to converting a 64-bit pointer to 32-bit value, but
		//       it's actually ok because we should never have over 4GB worth of vertices!
		BcU32 ByteOffset = BcU32( ( (BcU8*)pVertex - (BcU8*)pVertices_ ) & 0xffffffff );
		return ByteOffset / sizeof( ScnCanvasComponentVertex );
	}
	
protected:
	struct TRenderResource
	{
		RsVertexBuffer*					pVertexBuffer_;
		RsPrimitive*					pPrimitive_;
		ScnCanvasComponentVertex*		pVertices_;
	};

	BcU32								CurrentRenderResource_;
	BcBool								HaveVertexBufferLock_;
	TRenderResource						RenderResources_[ 2 ];
	TRenderResource*					pRenderResource_;

	// Submission data.
	ScnCanvasComponentVertex*			pVertices_;
	ScnCanvasComponentVertex*			pVerticesEnd_;
	BcU32								NoofVertices_;
	BcU32								VertexIndex_;
	
	// Materials.
	ScnMaterialComponentRef				DefaultMaterialComponent_;
	ScnMaterialComponentRef				MaterialComponent_;
	ScnTextureRef						DiffuseTexture_;

	typedef std::vector< ScnCanvasComponentPrimitiveSection > TPrimitiveSectionList;
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
