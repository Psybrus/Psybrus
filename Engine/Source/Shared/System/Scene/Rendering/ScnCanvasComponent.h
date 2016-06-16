/**************************************************************************
*
* File:		Rendering/ScnCanvasComponent.h
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

#include "System/Renderer/RsCore.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentRef
typedef ReObjectRef< class ScnCanvasComponent > ScnCanvasComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentVertex
struct ScnCanvasComponentVertex
{
	BcF32 X_, Y_, Z_, W_;
	BcF32 U_, V_;
	BcU32 ABGR_;
};

typedef std::function< void( class RsContext* ) > ScnCanvasRenderFunc;

//////////////////////////////////////////////////////////////////////////
// ScnCanvasComponentPrimitiveSection
struct ScnCanvasComponentPrimitiveSection
{
	RsTopologyType Type_;
	BcU32 VertexIndex_;
	BcU32 NoofVertices_;
	BcU32 Layer_;
	ScnMaterialComponentRef	MaterialComponent_;
	ScnCanvasRenderFunc RenderFunc_;
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
/** @class ScnCanvasComponent
 *
 * Format in package:
 {
   "type" : "ScnCanvasComponent",
   // Number of vertices to allocate for canvas.
   "noofvertices" : int
   // Clear canvas automatically at the beginning of each frame.
   "clear" : true|false,
   // If true, will use coords you specify w/o multiplying by client size.
   "absolutecoords" : true|false,
   // Left coordinate in canvas.
   "left" : float,
   // Right coordinate in canvas.
   "right" : float,
   // Top coordinate in canvas.
   "top" : float,
   // Bottom coordinate in canvas.
   "bottom" : float	
 }
 */
class ScnCanvasComponent:
	public ScnRenderableComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnCanvasComponent, ScnRenderableComponent );
	
	ScnCanvasComponent();
	ScnCanvasComponent( BcU32 NoofVertices );
	virtual ~ScnCanvasComponent();

	MaAABB getAABB() const override;

	/**
	 * Set material component.
	 */
	void setMaterialComponent( ScnMaterialComponentRef MaterialComponent );
	
	/**
	 * Get material component.
	 */
	ScnMaterialComponentRef getMaterialComponent();

	/**
	 * Push matrix.
	 * @param Matrix Matrix.
	 */
	void pushMatrix( const MaMat4d& Matrix );
	
	/*
	 * Pop matrix.
	 */
	MaMat4d popMatrix();
	
	/*
	 * Set current matrix.
	 */
	void setMatrix( const MaMat4d& Matrix );

	/*
	 * Get current matrix.
	 */
	MaMat4d getMatrix() const;

	/**
	 * Set view matrix.
	 * Only used when automatically set to clear.
	 */
	void setViewMatrix( const MaMat4d& View );

	/**
	 * Get rect for current material component.
	 */
	const ScnRect& getRect( BcU32 Idx ) const;

	/**
	 * Allocate some vertices to use.<br/>
	 * Safe to allocate 0 if you don't know how many vertices you need initially,
	 * and to allocate the total number at the end. Provided you don't overrun the buffer!
	 * @param NoofVertices Number of vertices to allocate.
	 */
	ScnCanvasComponentVertex* allocVertices( BcSize NoofVertices );

	/**
	 * Add custom render.<br/>
	 */
	void addCustomRender( 
		ScnCanvasRenderFunc CustomRenderFunc,
		BcU32 Layer = 0 );
	
	/**
	 * Add raw primitive.<br/>
	 */
	void addPrimitive( 
		RsTopologyType Type, 
		ScnCanvasComponentVertex* pVertices, 
		BcU32 NoofVertices, 
		BcU32 Layer = 0,
		BcBool UseMatrixStack = BcTrue );

	/**
	 * Draw line.
	 * @param PointA Point A
	 * @param PointB Point B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void drawLine( const MaVec2d& PointA, const MaVec2d& PointB, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw lines.
	 * @param pPoints Pointer to points.
	 * @param NoofLines Number of lines.
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void drawLines( const MaVec2d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw line box.
	 * @param CornerA Corner A
	 * @param CornerB Corner B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void drawLineBox( const MaVec2d& CornerA, const MaVec2d& CornerB, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw line box centered.
	 * @param CornerA Corner A
	 * @param CornerB Corner B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void drawLineBoxCentered( const MaVec2d& Position, const MaVec2d& Size, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw box.
	 * @param CornerA Corner A
	 * @param CornerB Corner B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void drawBox( const MaVec2d& CornerA, const MaVec2d& CornerB, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite.
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void drawSprite( const MaVec2d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw sprite centered.
	 * @param Position Position.
	 * @param Size Size.
	 * @param TextureIdx Texture Index.
	 * @param Colour Colour.
	 * @param Layer Layer.
	 */
	void drawSpriteCentered( const MaVec2d& Position, const MaVec2d& Size, BcU32 TextureIdx, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Clear canvas.
	 */
	void clear();
	
public:
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	void render( ScnRenderContext& RenderContext ) override;

	static void clearAll( const ScnComponentList& Components );
	static void endAll( const ScnComponentList& Components );

protected:
	BcForceInline BcU32 convertVertexPointerToIndex( ScnCanvasComponentVertex* pVertex )
	{
		// NOTE: Will probably warn due to converting a 64-bit pointer to 32-bit value, but
		//       it's actually ok because we should never have over 4GB worth of vertices!
		BcU32 ByteOffset = BcU32( ( (BcU8*)pVertex - (BcU8*)pVertices_ ) & 0xffffffff );
		return ByteOffset / sizeof( ScnCanvasComponentVertex );
	}
	
protected:
	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr VertexBuffer_;
	RsGeometryBindingUPtr GeometryBinding_;

	BcBool HaveVertexBufferLock_;

	// Submission data.
	ScnCanvasComponentVertex* pWorkingVertices_;
	ScnCanvasComponentVertex* pVertices_;
	ScnCanvasComponentVertex* pVerticesEnd_;
	BcSize NoofVertices_;
	BcSize VertexIndex_;
	SysFence UploadFence_;
	
	// Materials.
	ScnMaterialComponentRef MaterialComponent_;
	ScnTextureRef DiffuseTexture_;

	typedef std::vector< ScnCanvasComponentPrimitiveSection > TPrimitiveSectionList;
	typedef TPrimitiveSectionList::iterator TPrimitiveSectionListIterator;
	
	TPrimitiveSectionList PrimitiveSectionList_;
	BcU32 LastPrimitiveSection_;

	// Matrix stack.
	typedef std::vector< MaMat4d > TMatrixStack;
	typedef TMatrixStack::iterator TMatrixStackIterator;
	
	TMatrixStack MatrixStack_;
	BcBool IsIdentity_;

	// Automatic clear and setup.
	BcBool Clear_;
	BcBool AbsoluteCoords_ = BcFalse;
	BcF32 Left_;
	BcF32 Right_;
	BcF32 Top_;
	BcF32 Bottom_;
	MaMat4d ViewMatrix_;
};

#endif
