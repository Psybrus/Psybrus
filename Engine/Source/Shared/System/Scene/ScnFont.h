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

#include "System/Content/CsResource.h"

#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnCanvasComponent.h"
#include "System/Scene/ScnFontFileData.h"

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
	DECLARE_RESOURCE( CsResource, ScnFont );
	
#ifdef PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
	BcBool								createInstance( const std::string& Name, ScnFontComponentRef& FontComponent, ScnMaterialRef Material );
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
private:
	friend class ScnFontComponent;
	
	ScnFontHeader*						pHeader_;
	ScnFontGlyphDesc*					pGlyphDescs_;
	
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
	void								initialise( const Json::Value& Object );

	void								setClipping( BcBool Enabled, BcVec2d Min = BcVec2d( 0.0f, 0.0f ), BcVec2d Max = BcVec2d( 0.0f, 0.0f ) );
	
	BcVec2d								draw( ScnCanvasComponentRef Canvas, const BcVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse, BcU32 Layer = 16 ); // HACK.
	BcVec2d								drawCentered( ScnCanvasComponentRef Canvas, const BcVec2d& Position, const std::string& String, RsColour Colour, BcU32 Layer = 16 ); // HACK.

	void								setAlphaTestStepping( const BcVec2d& Stepping );

	ScnMaterialComponentRef				getMaterialComponent();

public:
	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	friend class ScnFont;

	ScnFontRef							Parent_;
	ScnMaterialComponentRef				MaterialComponent_;

	BcBool								ClippingEnabled_;
	BcVec2d								ClipMin_;
	BcVec2d								ClipMax_;
};

#endif
