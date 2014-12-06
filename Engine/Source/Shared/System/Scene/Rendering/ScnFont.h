/**************************************************************************
*
* File:		Rendering/ScnFont.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnFont_H__
#define __ScnFont_H__

#include "System/Content/CsResource.h"

#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFontFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef ReObjectRef< class ScnFont > ScnFontRef;
typedef std::map< std::string, ScnFontRef > ScnFontMap;
typedef ScnFontMap::iterator ScnFontMapIterator;
typedef ScnFontMap::const_iterator ScnFontConstIterator;
typedef std::vector< ScnFontRef > ScnFontList;
typedef ScnFontList::iterator ScnFontListIterator;
typedef ScnFontList::const_iterator ScnFontListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnFontComponentRef
typedef ReObjectRef< class ScnFontComponent > ScnFontComponentRef;
typedef std::map< std::string, ScnFontComponentRef > ScnFontComponentMap;
typedef ScnFontComponentMap::iterator ScnFontComponentMapIterator;
typedef ScnFontComponentMap::const_iterator ScnFontComponentConstIterator;
typedef std::vector< ScnFontComponentRef > ScnFontComponentList;
typedef ScnFontComponentList::iterator ScnFontComponentListIterator;
typedef ScnFontComponentList::const_iterator ScnFontComponentListConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnFont
class ScnFont:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnFont, CsResource );
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
	BcBool								createInstance( const std::string& Name, ScnFontComponentRef& FontComponent, ScnMaterialRef Material );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
private:
	friend class ScnFontComponent;
	
	ScnFontHeader* pHeader_;
	ScnFontGlyphDesc* pGlyphDescs_;
	
	typedef std::map< BcU32, BcU32 >	TCharCodeMap;
	typedef TCharCodeMap::iterator		TCharCodeMapIterator;
	
	TCharCodeMap CharCodeMap_;
	ScnTexture* Texture_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontComponent
class ScnFontComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnFontComponent, ScnComponent );
	
	void								initialise();
	void								initialise( ScnFontRef Parent, ScnMaterialRef Material );
	void								initialise( const Json::Value& Object );

	void								setClipping( BcBool Enabled, MaVec2d Min = MaVec2d( 0.0f, 0.0f ), MaVec2d Max = MaVec2d( 0.0f, 0.0f ) );
	
	MaVec2d								draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse, BcU32 Layer = 16 ); // HACK.
	MaVec2d								drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcU32 Layer = 16 ); // HACK.

	MaVec2d								draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, BcF32 Size, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse, BcU32 Layer = 16 ); // HACK.

	void								setAlphaTestStepping( const MaVec2d& Stepping );

	ScnMaterialComponentRef				getMaterialComponent();

public:
	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	friend class ScnFont;

	ScnFontRef Parent_;
	ScnMaterialRef Material_;
	ScnMaterialComponentRef MaterialComponent_;

	BcBool ClippingEnabled_;
	MaVec2d ClipMin_;
	MaVec2d ClipMax_;

	RsBuffer* UniformBuffer_;
	ScnShaderAlphaTestUniformBlockData AlphaTestUniforms_;
};

#endif
