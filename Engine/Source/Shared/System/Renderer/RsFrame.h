/**************************************************************************
*
* File:		RsFrame.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Frame object. All render commands are pushed onto here.
*		
*
*
* 
**************************************************************************/

#ifndef __RSFRAME_H__
#define __RSFRAME_H__

#include "Base/BcTypes.h"


//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsCore;
class RsMaterial;
class RsEffect;
class RoModel;

//////////////////////////////////////////////////////////////////////////
// Enums
enum eRsFramePrimitiveMode
{
	rsFPM_2D = 0,
	rsFPM_3D
};

//////////////////////////////////////////////////////////////////////////
/**	\class RsFrame
*	\brief Render System Frame
*
*	This object encapsulates all rendering tasks and resources for a frame.
*/
class RsFrame
{
public:
	RsFrame( 
		class RsContext* pContext, 
		BcU32 NoofNodes = 32 * 1024, 
		BcU32 NodeMem = 1024 * 1024 );
	~RsFrame();
	
	/**
	 *	Get context.
	 */
	class RsContext* getContext() const;

	/**
	 * Reset frame.
	 */
	void reset();

	/**
	 * Render frame.
	 */
	void render();
		
	/**
	 *	Add render node.\n
	 *	NON-JOB FUNCTION: Placeholder.
	 */
	void addRenderNode( class RsRenderNode* pInstance );
	
	/**
	 *	Allocate from instance memory.
	 */
	void* allocMem( BcSize Bytes );

public:
	/**
	 *	Allocate from instance memory.\n
	 *	NON-JOB FUNCTION: Called by specific frame implementation.
	 */
	template< typename _Ty >
	_Ty* alloc( BcU32 NoofObjects  = 1 );
	
	/**
	 *	Create new object from instance memory.
	 */
	template< typename _Ty >
	_Ty* newObject();

private:
	void			sortNodes();
	void			sortNodeRadix( 
		BcU32 Bits, 
		BcU32 NoofItems, 
		class RsRenderNode** pSrc, 
		class RsRenderNode** pDst );

private:
	class RsContext* pContext_;
	class RsRenderNode** ppNodeArray_;
	class RsRenderNode** ppNodeSortArray_;
	BcU32 NoofNodes_;
	BcU32 CurrNode_;

private:
	BcU8* pFrameMem_;
	BcU8* pCurrFrameMem_;
	BcU32 FrameBytes_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines

template< typename _Ty >
inline _Ty* RsFrame::alloc( BcU32 NoofObjects )
{
	return reinterpret_cast< _Ty* >( allocMem( sizeof( _Ty ) * NoofObjects ) );
}

template< typename _Ty >
inline _Ty* RsFrame::newObject()
{
	return new ( alloc< _Ty >() ) _Ty();
}

#endif
