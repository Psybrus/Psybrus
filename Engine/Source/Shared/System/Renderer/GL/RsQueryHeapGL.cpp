#include "System/Renderer/GL/RsQueryHeapGL.h"
#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"
#include "System/Renderer/RsQueryHeap.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsQueryHeapGL::RsQueryHeapGL( RsQueryHeap* Parent, const RsOpenGLVersion& Version ):
	Parent_( Parent )
{
	Parent_->setHandle( this );
	const auto& QueryHeapDesc = Parent_->getDesc();
	auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );

#if !defined( RENDER_USE_GLES )

	Handles_.resize( QueryHeapDesc.NoofQueries_ );

	GL( GenQueries( Handles_.size(), Handles_.data() ) );
#endif

#if !defined( RENDER_USE_GLES ) && !PSY_PRODUCTION
	// Causes GL_INVALID_VALUE. Investigate later.
	if( 0 && GLEW_KHR_debug )
	{
		for( auto Handle : Handles_ )
		{
			GL( ObjectLabel( GL_QUERY, Handle, BcStrLength( Parent->getDebugName() ), Parent->getDebugName() ) );
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsQueryHeapGL::~RsQueryHeapGL()
{
	GL( DeleteQueries( Handles_.size(), Handles_.data() ) );
	Handles_.clear();
}
