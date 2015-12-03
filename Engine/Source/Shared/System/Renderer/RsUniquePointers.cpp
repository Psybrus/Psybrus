#include "System/Renderer/RsUniquePointers.h"
#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Resource deletion.
void RsResourceDeleters::operator()( class RsBuffer* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsContext* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsFrameBuffer* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsGeometryBinding* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsProgram* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsProgramBinding* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsRenderState* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsSamplerState* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsShader* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsTexture* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}

void RsResourceDeleters::operator()( class RsVertexDeclaration* Resource )
{
	BcAssertMsg( RsCore::pImpl(), "Must be reset prior to renderer shutdown." )
	if( RsCore::pImpl() )
	{
		RsCore::pImpl()->destroyResource( Resource );
	}
}
