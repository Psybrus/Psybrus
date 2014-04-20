/**************************************************************************
*
* File:		ScnDebugRenderComponent.h
* Author:	Neil Richardson 
* Ver/Date:	10/04/11	
* Description:
*		Used for 2D rendering.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnDebugRenderComponentComponent_H__
#define __ScnDebugRenderComponentComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Scene/ScnRenderableComponent.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// ScnDebugRenderComponentRef
typedef ReObjectRef< class ScnDebugRenderComponent > ScnDebugRenderComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnDebugRenderComponentVertex
struct ScnDebugRenderComponentVertex
{
	BcF32 X_, Y_, Z_;
	BcU32 ABGR_;
};

//////////////////////////////////////////////////////////////////////////
// ScnDebugRenderComponentPrimitiveSection
struct ScnDebugRenderComponentPrimitiveSection
{
	eRsPrimitiveType		Type_;
	BcU32					VertexIndex_;
	BcU32					NoofVertices_;
	BcU32					Layer_;
	ScnMaterialComponentRef	MaterialComponent_;
};

class ScnDebugRenderComponentPrimitiveSectionCompare
{
public:
	bool operator()( const ScnDebugRenderComponentPrimitiveSection& A, const ScnDebugRenderComponentPrimitiveSection& B )
	{
		return A.Layer_ < B.Layer_;
	}
};

//////////////////////////////////////////////////////////////////////////
// ScnDebugRenderComponent
class ScnDebugRenderComponent:
	public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, ScnDebugRenderComponent );

	static ScnDebugRenderComponent*		pImpl();
	
	virtual void						initialise( BcU32 NoofVertices );
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();
	virtual BcAABB						getAABB() const;
	
	/**
	 * Allocate some vertices to use.<br/>
	 * Safe to allocate 0 if you don't know how many vertices you need initially,
	 * and to allocate the total number at the end. Provided you don't overrun the buffer!
	 * @param NoofVertices Number of vertices to allocate.
	 */
	ScnDebugRenderComponentVertex*		allocVertices( BcU32 NoofVertices );
	
	/**
	 * Add raw primitive.<br/>
	 */
	void								addPrimitive( eRsPrimitiveType Type, ScnDebugRenderComponentVertex* pVertices, BcU32 NoofVertices, BcU32 Layer = 0, BcBool UseMatrixStack = BcTrue );

	/**
	 * Draw line.
	 * @param PointA Point A
	 * @param PointB Point B
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour, BcU32 Layer = 0 );
	
	/**
	 * Draw lines.
	 * @param pPoints Pointer to points.
	 * @param NoofLines Number of lines.
	 * @param Colour Colour
	 * @param Layer Layer
	 */
	void								drawLines( const BcVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Draw grid.
	 * @params Position Position of centre of grid.
	 * @params Size Size of grid. Only use 2 of the axis!
	 * @param StepSize Step size to draw at.
	 * @param SubDivideMultiple Multiple to use to subdivide grid.
	 * @param Layer Layer
	 */
	void								drawGrid( const BcVec3d& Position, const BcVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer = 0 );

	/**
	 * Draw ellipsoid.
	 * @params Position Position of ellipsoid.
	 * @params Size Size of ellipsioid.
	 * @params Colour Colour to draw it.
	 * @param Layer Layer
	 */
	void								drawEllipsoid( const BcVec3d& Position, const BcVec3d& Size, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	* Draw AABB.
	 * @params AABB AABB to draw.
	 * @params Colour Colour to draw it.
	 * @param Layer Layer
	 */
	void								drawAABB( const BcAABB& AABB, const RsColour& Colour, BcU32 Layer = 0 );

	/**
	 * Clear
	 */
	void								clear();

public:
	virtual void						preUpdate( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );


protected:
	BcForceInline BcU32					convertVertexPointerToIndex( ScnDebugRenderComponentVertex* pVertex )
	{
		// NOTE: Will probably warn due to converting a 64-bit pointer to 32-bit value, but
		//       it's actually ok because we should never have over 4GB worth of vertices!
		BcU32 ByteOffset = BcU32( ( (BcU8*)pVertex - (BcU8*)pVertices_ ) & 0xffffffff );
		return ByteOffset / sizeof( ScnDebugRenderComponentVertex );
	}
	
protected:
	static ScnDebugRenderComponent*		pImpl_;

	struct TRenderResource
	{
		RsVertexBuffer*					pVertexBuffer_;
		RsPrimitive*					pPrimitive_;
		ScnDebugRenderComponentVertex*	pVertices_;
	};

	BcU32								CurrentRenderResource_;
	BcBool								HaveVertexBufferLock_;
	TRenderResource						RenderResources_[ 2 ];
	TRenderResource*					pRenderResource_;

	// Submission data.
	ScnDebugRenderComponentVertex*		pVertices_;
	ScnDebugRenderComponentVertex*		pVerticesEnd_;
	BcU32								NoofVertices_;
	BcU32								VertexIndex_;
	
	// Materials.
	ScnMaterialComponentRef				MaterialComponent_;

	typedef std::vector< ScnDebugRenderComponentPrimitiveSection > TPrimitiveSectionList;
	typedef TPrimitiveSectionList::iterator TPrimitiveSectionListIterator;
	
	TPrimitiveSectionList				PrimitiveSectionList_;
	BcU32								LastPrimitiveSection_;
};

#endif
