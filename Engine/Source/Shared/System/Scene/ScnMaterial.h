/**************************************************************************
*
* File:		ScnMaterial.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMATERIAL_H__
#define __SCNMATERIAL_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnShader.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialRef
typedef CsResourceRef< class ScnMaterial > ScnMaterialRef;
typedef std::map< std::string, ScnMaterialRef > ScnMaterialMap;
typedef ScnMaterialMap::iterator ScnMaterialMapIterator;
typedef ScnMaterialMap::const_iterator ScnMaterialConstIterator;
typedef std::vector< ScnMaterialRef > ScnMaterialList;
typedef ScnMaterialList::iterator ScnMaterialListIterator;
typedef ScnMaterialList::const_iterator ScnMaterialListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnMaterialComponentRef
typedef CsResourceRef< class ScnMaterialComponent > ScnMaterialComponentRef;
typedef std::map< std::string, ScnMaterialComponentRef > ScnMaterialComponentMap;
typedef ScnMaterialComponentMap::iterator ScnMaterialComponentMapIterator;
typedef ScnMaterialComponentMap::const_iterator ScnMaterialComponentConstIterator;
typedef std::vector< ScnMaterialRef > ScnMaterialComponentList;
typedef ScnMaterialComponentList::iterator ScnMaterialComponentListIterator;
typedef ScnMaterialComponentList::const_iterator ScnMaterialComponentListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnMaterial
class ScnMaterial:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnMaterial );
	
#ifdef PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	ScnTextureRef						getTexture( BcName Name );


private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
private:
	friend class ScnMaterialComponent;
	
	struct THeader
	{
		BcU32							ShaderName_;
		BcU32							NoofTextures_;
	};

	struct TTextureHeader
	{
		BcU32							SamplerName_;
		BcU32							TextureName_;
		BcU32							TextureType_;
	};
	
	THeader*							pHeader_;
	
	ScnShaderRef						Shader_;
	ScnTextureMap						TextureMap_;

	// TODO: Should be handled by the state block.
	BcU32*								pStateBuffer_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialComponent
class ScnMaterialComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnMaterialComponent );
	
	void								initialise( ScnMaterialRef Parent, BcU32 PermutationFlags );
	void								initialise( const Json::Value& Object );
	void								destroy();
	
	BcU32								findParameter( const BcName& ParameterName );	
	void								setParameter( BcU32 Parameter, BcS32 Value );
	void								setParameter( BcU32 Parameter, BcBool Value );
	void								setParameter( BcU32 Parameter, BcReal Value );
	void								setParameter( BcU32 Parameter, const BcVec2d& Value );
	void								setParameter( BcU32 Parameter, const BcVec3d& Value );
	void								setParameter( BcU32 Parameter, const BcVec4d& Value );
	void								setParameter( BcU32 Parameter, const BcMat3d& Value );
	void								setParameter( BcU32 Parameter, const BcMat4d& Value );
	void								setTexture( BcU32 Parameter, ScnTextureRef Texture );

	// Common scene parameters.
	void								setClipTransform( const BcMat4d& Transform );
	void								setViewTransform( const BcMat4d& Transform );
	void								setWorldTransform( const BcMat4d& Transform );
	
	void								setState( eRsRenderState State, BcU32 Value );
	
	ScnTextureRef						getTexture( BcU32 Parameter );
	ScnMaterialRef						getMaterial();
	
	void								bind( RsFrame* pFrame, RsRenderSort& Sort );

	virtual BcBool						isReady();

public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );


private:
	friend class ScnMaterial;
			
	struct TParameterBinding
	{
		eRsShaderParameterType			Type_;
		BcU32							Offset_;
	};
	
	typedef std::vector< TParameterBinding > TParameterBindingList;
	typedef TParameterBindingList::iterator TParameterBindingListIterator;
	
	struct TTextureBinding
	{
		BcU32							Parameter_;
		ScnTextureRef					Texture_;
	};
	
	typedef std::vector< TTextureBinding > TTextureBindingList;
	typedef TTextureBindingList::iterator TTextureBindingListIterator;
	
	ScnMaterialRef						Parent_;
	RsProgram*							pProgram_;

	TParameterBindingList				ParameterBindingList_;
	TTextureBindingList					TextureBindingList_;

	BcU32								ParameterBufferSize_;
	BcU8*								pParameterBuffer_;
	
	// TODO: Should be handled by the state block.
	BcU32*								pStateBuffer_;

	// Common scene parameters.
	BcU32								ClipTransformParameter_;
	BcU32								ViewTransformParameter_;
	BcU32								InverseViewTransformParameter_;
	BcU32								WorldTransformParameter_;
};

#endif
