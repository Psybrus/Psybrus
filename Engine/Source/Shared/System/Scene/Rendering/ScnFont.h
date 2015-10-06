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

#include <string>

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
	REFLECTION_DECLARE_DERIVED( ScnFont, CsResource );
	
	ScnFont();
	virtual ~ScnFont();

	void create() override;
	void destroy() override;
		
private:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
private:
	friend class ScnFontComponent;
	
	ScnFontHeader* pHeader_;
	ScnFontGlyphDesc* pGlyphDescs_;
	
	typedef std::map< BcU32, BcU32 > TCharCodeMap;
	typedef TCharCodeMap::iterator TCharCodeMapIterator;
	
	TCharCodeMap CharCodeMap_;
	ScnTexture* Texture_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontAlignment
enum class ScnFontAlignment : BcU32 
{
	NONE				= 0x00,
	LEFT				= 0x01,
	RIGHT				= 0x02,
	HCENTRE				= 0x04,
	TOP					= 0x10,
	BOTTOM				= 0x20,
	VCENTRE				= 0x40,

	//
	HORIZONTAL			= LEFT | RIGHT | HCENTRE,
	VERTICAL			= TOP | BOTTOM | VCENTRE,
};

inline ScnFontAlignment operator | ( ScnFontAlignment A, ScnFontAlignment B )
{
	return ScnFontAlignment( BcU32( A ) | BcU32( B ) );
}

inline ScnFontAlignment operator & ( ScnFontAlignment A, ScnFontAlignment B )
{
	return ScnFontAlignment( BcU32( A ) & BcU32( B ) );
}

//////////////////////////////////////////////////////////////////////////
// ScnFontUniformBlockData
struct ScnFontUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnFontUniformBlockData );
	ScnFontUniformBlockData(){};

	MaVec4d TextSettings_; /// x = smoothstep min, y = smoothstep max, z = ref (<)
	MaVec4d BorderSettings_; ///
	MaVec4d ShadowSettings_; ///
	RsColour TextColour_;
	RsColour BorderColour_;
	RsColour ShadowColour_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontDrawParams
class ScnFontDrawParams
{
public:
	REFLECTION_DECLARE_BASIC( ScnFontDrawParams );
public:
	ScnFontDrawParams();

	/**
	 * How the text should be aligned.
	 */
	ScnFontDrawParams& setAlignment( ScnFontAlignment Alignment );
	ScnFontAlignment getAlignment() const;

	/**
	 * Size of margin to put around text whilst aligning.
	 */
	ScnFontDrawParams& setMargin( BcF32 Margin );
	BcF32 getMargin() const;

	/**
	 * Do we wrap words when aligning?
	 */
	ScnFontDrawParams& setWrappingEnabled( BcBool Enabled );
	BcBool getWrappingEnabled() const;

	/**
	 * Layer to draw on.
	 */
	ScnFontDrawParams& setLayer( BcU32 Layer );
	BcU32 getLayer() const;

	/**
	 * Size of text.
	 */
	ScnFontDrawParams& setSize( BcF32 Size );
	BcF32 getSize() const;

	/**
	 * Do we wish to allow clipping?
	 */
	ScnFontDrawParams& setClippingEnabled( BcBool Enabled );
	BcBool getClippingEnabled() const;

	/**
	 * Clipping bounds.
	 * @param Bounds minx, miny, maxx, maxy
	 */
	ScnFontDrawParams& setClippingBounds( const MaVec4d& Bounds );
	const MaVec4d& getClippingBounds() const;

	/**
	 * Text colour.
	 */
	ScnFontDrawParams& setTextColour( const RsColour& TextColour );
	const RsColour& getTextColour() const;

	/**
	 * Border colour.
	 */
	ScnFontDrawParams& setBorderColour( const RsColour& BorderColour );
	const RsColour& getBorderColour() const;

	/**
	 * Shadow colour.
	 */
	ScnFontDrawParams& setShadowColour( const RsColour& ShadowColour );
	const RsColour& getShadowColour() const;

	/**
	 * Text settings.
	 * @param Settings (Min, Max, <>, <>)
	 */
	ScnFontDrawParams& setTextSettings( const MaVec4d& Settings );
	const MaVec4d& getTextSettings() const;

	/**
	 * Border settings.
	 * @param Settings (Min, Max, <>, <>)
	 */
	ScnFontDrawParams& setBorderSettings( const MaVec4d& Settings );
	const MaVec4d& getBorderSettings() const;

	/**
	 * Shadow settings.
	 * @param Settings (Xoff, Yoff, <>, <>)
	 */
	ScnFontDrawParams& setShadowSettings( const MaVec4d& Settings );
	const MaVec4d& getShadowSettings() const;

private:
	ScnFontAlignment Alignment_;
	BcF32 Margin_;
	BcBool WrappingEnabled_;
	BcU32 Layer_;
	BcF32 Size_;
	BcBool ClippingEnabled_;
	MaVec4d ClippingBounds_;
	RsColour TextColour_;
	RsColour BorderColour_;
	RsColour ShadowColour_;
	MaVec4d TextSettings_;
	MaVec4d BorderSettings_;
	MaVec4d ShadowSettings_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontComponent
class ScnFontComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnFontComponent, ScnComponent );

	ScnFontComponent();
	ScnFontComponent( ScnFontRef Parent, ScnMaterialRef Material );
	virtual ~ScnFontComponent();
	
	// Old interface.
	void setClipping( BcBool Enabled, MaVec2d Min = MaVec2d( 0.0f, 0.0f ), MaVec2d Max = MaVec2d( 0.0f, 0.0f ) );
	MaVec2d draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse, BcU32 Layer = 16 ); // HACK.
	MaVec2d drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, const std::string& String, RsColour Colour, BcU32 Layer = 16 ); // HACK.
	MaVec2d draw( ScnCanvasComponentRef Canvas, const MaVec2d& Position, BcF32 Size, const std::string& String, RsColour Colour, BcBool SizeRun = BcFalse, BcU32 Layer = 16 ); // HACK.
	MaVec2d drawCentered( ScnCanvasComponentRef Canvas, const MaVec2d& Position, BcF32 Size, const std::string& String, RsColour Colour, BcU32 Layer = 16 ); // HACK.
	void setAlphaTestStepping( const MaVec2d& Stepping );
	ScnMaterialComponentRef getMaterialComponent();

	// New interfaces.

	/**
	 * Draw text to canvas.
	 * @param Canvas Canvas to render in to. Can be null.
	 * @param DrawParam Draw parameters.
	 * @param Position Position to render to.
	 * @param TargetSize Target text size.
	 * @param Text Text to draw.
	 * @return Final size of text.
	 */
	MaVec2d drawText( 
		ScnCanvasComponentRef Canvas, 
		const ScnFontDrawParams& DrawParams,
		const MaVec2d& Position,
		const MaVec2d& TargetSize,
		const std::string& Text );

	/**
	 * Draw text to canvas.
	 * @param Canvas Canvas to render in to. Can be null.
	 * @param DrawParam Draw parameters.
	 * @param Position Position to render to.
	 * @param TargetSize Target text size.
	 * @param Text Text to draw.
	 * @return Final size of text.
	 */
	MaVec2d drawText( 
		ScnCanvasComponentRef Canvas, 
		const ScnFontDrawParams& DrawParams,
		const MaVec2d& Position,
		const MaVec2d& TargetSize,
		const std::wstring& Text );

	/**
	 * Measure text.
	 */
	MaVec2d measureText( 
		const ScnFontDrawParams& DrawParams,
		const std::wstring& Text );


public:
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	friend class ScnFont;

	ScnFontRef Font_;
	ScnMaterialRef Material_;
	ScnMaterialComponentRef MaterialComponent_;

	bool ClippingEnabled_;
	MaVec2d ClipMin_;
	MaVec2d ClipMax_;

	RsBuffer* UniformBuffer_;
	ScnFontUniformBlockData FontUniformData_;
	SysFence UploadFence_;
};

#endif
