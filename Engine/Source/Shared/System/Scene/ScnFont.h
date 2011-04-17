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

#include "CsResourceRef.h"

#include "ScnMaterial.h"
#include "ScnCanvas.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef CsResourceRef< class ScnFont > ScnFontRef;

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef CsResourceRef< class ScnFontInstance > ScnFontInstanceRef;

//////////////////////////////////////////////////////////////////////////
// ScnFont
class ScnFont:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnFont );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	BcBool								createInstance( const std::string& Name, ScnFontInstanceRef& FontInstance, ScnMaterialRef Material );
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	friend class ScnFontInstance;
	
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
// ScnFontInstance
class ScnFontInstance:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnFontInstance );
	
	void								initialise( ScnFontRef Parent, ScnMaterialRef Material );
	
	void								draw( ScnCanvasRef Canvas, const std::string& String );
	
	virtual BcBool						isReady();
	
private:
	friend class ScnFont;

	ScnFontRef							Parent_;
	ScnMaterialInstanceRef				MaterialInstance_;
};

#endif
