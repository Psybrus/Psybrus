/**************************************************************************
*
* File:		Rendering/ScnTexture.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNTEXTURE_H__
#define __SCNTEXTURE_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/Rendering/ScnTextureFileData.h"


//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef ReObjectRef< class ScnTexture > ScnTextureRef;
typedef std::vector< ScnTextureRef > ScnTextureList;
typedef ScnTextureList::iterator ScnTextureListIterator;
typedef ScnTextureList::const_iterator ScnTextureListConstIterator;
typedef std::map< BcName, ScnTextureRef > ScnTextureMap;
typedef ScnTextureMap::iterator ScnTextureMapIterator;
typedef ScnTextureMap::const_iterator ScnTextureMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnTexture
class ScnTexture:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnTexture, CsResource );
	
	ScnTexture();
	virtual ~ScnTexture();

	static ScnTexture* New( const RsTextureDesc& Desc, const char* DebugName );
	static ScnTexture* New1D( BcS32 Width, BcU32 Levels, RsTextureFormat Format, const char* DebugName );
	static ScnTexture* New2D( BcS32 Width, BcS32 Height, BcU32 Levels, RsTextureFormat Format, bool IsRT, bool IsDS, const char* DebugName );
	static ScnTexture* New3D( BcS32 Width, BcS32 Height, BcS32 Depth, BcU32 Levels, RsTextureFormat Format, const char* DebugName );
	static ScnTexture* NewCube( BcS32 Width, BcS32 Height, BcU32 Levels, RsTextureFormat Format, const char* DebugName );

	RsTexture* getTexture();
	
	BcU32 getWidth() const;
	BcU32 getHeight() const;
	BcU32 getDepth() const;
	BcU32 getInternalWidth() const;
	BcU32 getInternalHeight() const;
	BcU32 getInternalDepth() const;

	/**
	 * Is texture dependent on client size? I.e. will it change on resize?
	 */
	bool isClientDependent() const;
	
	virtual ScnRect getRect( BcU32 Idx ) const;
	virtual BcU32 noofRects() const;

	
protected:
	void create() override;
	void destroy() override;

	void recreate();
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

protected:
	RsTextureUPtr Texture_;
	
	ScnTextureHeader Header_;
	void* pTextureData_;

	// Texture dimensions.
	BcU32 Width_;
	BcU32 Height_;
	BcU32 Depth_;

	// Internal texture dimensions.
	BcU32 InternalWidth_;
	BcU32 InternalHeight_;
	BcU32 InternalDepth_;
};

#endif
