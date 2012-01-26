/**************************************************************************
*
* File:		ScnFont.h
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

#include "CsResource.h"

#include "ScnMaterial.h"
#include "ScnComponent.h"
#include "ScnCanvasComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef CsResourceRef< class ScnFont > ScnFontRef;

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef CsResourceRef< class ScnFontComponent > ScnFontComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnFont
class ScnFont:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnFont );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	BcBool								createInstance( const std::string& Name, ScnFontComponentRef& FontComponent, ScnMaterialRef Material );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
private:
	friend class ScnFontComponent;
	
	struct THeader
	{
		BcU32							NoofGlyphs_;
		BcChar							TextureName_[ 64 ];
	};
	
	struct TGlyphDesc
	{	
		// Texture.
		BcReal							UA_;
		BcReal							VA_;
		BcReal							UB_;
		BcReal							VB_;
	
		// Positioning.
		BcReal							OffsetX_;
		BcReal							OffsetY_;
		BcReal							Width_;
		BcReal							Height_;
		BcReal							AdvanceX_;
		
		// CharCode
		BcU32							CharCode_;
	};	
	
	THeader*							pHeader_;
	TGlyphDesc*							pGlyphDescs_;
	
	typedef std::map< BcU32, BcU32 >	TCharCodeMap;
	typedef TCharCodeMap::iterator		TCharCodeMapIterator;
	
	TCharCodeMap						CharCodeMap_;
	ScnTextureRef						Texture_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontComponent
class ScnFontComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnFontComponent );
	
	void								initialise( ScnFontRef Parent, ScnMaterialRef Material );
	
	BcVec2d								draw( ScnCanvasComponentRef Canvas, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse );

	ScnMaterialComponentRef				getMaterialComponent();
	
	virtual BcBool						isReady();

public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	friend class ScnFont;

	ScnFontRef							Parent_;
	ScnMaterialComponentRef				MaterialComponent_;
};

#endif
