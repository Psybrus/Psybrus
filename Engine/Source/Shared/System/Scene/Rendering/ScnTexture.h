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

	static ScnTexture* New( const RsTextureDesc& Desc );
	static ScnTexture* New1D( BcU32 Width, BcU32 Levels, RsTextureFormat Format );
	static ScnTexture* New2D( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format );
	static ScnTexture* New3D( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format );
	static ScnTexture* NewCube( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format );

	RsTexture* getTexture();
	
	BcU32 getWidth() const;
	BcU32 getHeight() const;
	
	virtual const ScnRect& getRect( BcU32 Idx ) const;
	virtual BcU32 noofRects() const;

	
protected:
	void create() override;
	void destroy() override;

	void recreate();
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

protected:
	RsTexture* pTexture_;
	
	ScnTextureHeader Header_;
	void* pTextureData_;

	BcU32 Width_;
	BcU32 Height_;
	BcU32 Depth_;
};

#endif
