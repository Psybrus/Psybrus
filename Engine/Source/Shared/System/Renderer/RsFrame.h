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
#include "System/Renderer/RsRenderNode.h"

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
		BcSize NodeMem = 1024 * 1024 );
	~RsFrame();

	/**
	 *	Get context.
	 */
	class RsContext* getContext() const;

	/**
	 *	Get back buffer width.
	 */
	BcU32 getBackBufferWidth() const;
	
	/**
	 *	Get back buffer height.
	 */
	BcU32 getBackBufferHeight() const;
	
	/**
	 * Reset frame.
	 */
	void reset();

	/**
	 * Render frame.
	 */
	void render();
			
	/**
	 *	Allocate from instance memory.
	 */
	void* allocMem( BcSize Bytes );

public:
	/**
	 * Queue render node.
	 * @param Sort Sorting value for render node.
	 * @param Callable Callable/lambda object to use in command.
	 * @pre Callable is trivially copyable.
	 * @pre Callable is trivially destructible..
	 */
	template < typename _CallableType >
	void queueRenderNode( RsRenderSort Sort, _CallableType&& Callable );

	/**
	 *	Allocate from instance memory.
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
	BcU32 Width_;
	BcU32 Height_;
	class RsRenderNode** ppNodeArray_;
	class RsRenderNode** ppNodeSortArray_;
	BcU32 NoofNodes_;
	BcU32 CurrNode_;

private:
	BcU8* pFrameMem_;
	BcU8* pCurrFrameMem_;
	BcSize FrameBytes_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines
template < typename _CallableType >
inline void RsFrame::queueRenderNode( RsRenderSort Sort, _CallableType&& Callable )
{
	static_assert( std::is_trivially_destructible< _CallableType >::value, "Must be trivially destructible" );

	BcAssertMsg( CurrNode_ < NoofNodes_, "Out of render nodes." );

	// Grab call function.
	auto CallFunc = &_CallableType::operator();

	// Grab aligned render node size.
	const auto RenderNodeSize = sizeof( RsRenderNode );

	// Allocate node data.
	BcU8* NodeBytes = reinterpret_cast< BcU8* >( allocMem( RenderNodeSize + sizeof( _CallableType ) ) );

	// Construct render node.
	auto RenderNode = new ( NodeBytes ) RsRenderNode( 
		Sort, reinterpret_cast< RsRenderNode::Callback >( CallFunc ) );

	// Placement construct a new callable moving into it.
	_CallableType* TargetCallable = reinterpret_cast< _CallableType* >( NodeBytes + RenderNodeSize );
	new ( TargetCallable ) _CallableType( std::move( Callable ) );

	// Add to node array.
	ppNodeArray_[ CurrNode_++ ] = RenderNode;
}

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
