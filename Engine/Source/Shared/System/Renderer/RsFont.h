/**************************************************************************
*
* File:		RsFont.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Font class
*		
*
*
* 
**************************************************************************/

#ifndef __RSFONT_H__
#define __RSFONT_H__

#include "RsTypes.h"
#include "RsVertex.h"

#include "RsTexture.h"
#include "RsMaterial.h"

#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsCore;
class RsCoreImplGL;
class RsCoreImplDX9;

//////////////////////////////////////////////////////////////////////////
// Font File Data
struct RsFontChar
{
	BcReal X_;
	BcReal Y_;
	BcReal Width_;
	BcReal Height_;
	BcReal XOff_;
	BcReal YOff_;
	BcReal XAdv_;
	BcU32 Page_;
};

struct RsFontCharIdx
{
	BcU16 Type_;
	BcU16 Char_;
};

struct RsFontFileData
{
	enum
	{
		MAX_CHARS = 256,
	};

	BcReal Width_;
	BcReal Height_;
	BcReal LineHeight_;
	BcU32 nPages_;
};

enum eFontCharType
{
	rsFCT_LETTER = 0,
	rsFCT_NEWLINE
};

//////////////////////////////////////////////////////////////////////////
// RsFontStyle
class RsFontStyle
{
public:
	enum eHAlign
	{
		HALIGN_LEFT		= 0x1,
		HALIGN_RIGHT	= 0x2,
		HALIGN_CENTRE	= 0x3
	};

public:
	RsFontStyle();
	~RsFontStyle();

	void layer( BcU32 SortOrder );
	void hAlign( eHAlign HAlignment );
	void colour( const RsColour& Colour );

	BcU32 layer() const;
	eHAlign hAlign() const;
	const RsColour& colour() const;

private:
	// Style
	BcU32 SortOrder_;
	eHAlign HAlignment_;
	RsColour Colour_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline RsFontStyle::RsFontStyle():
	SortOrder_( 0 ),
	HAlignment_( HALIGN_LEFT ),
	Colour_( RsColour( 1.0f, 1.0f, 1.0f, 1.0f ) )
{

}

inline RsFontStyle::~RsFontStyle()
{

}

inline void RsFontStyle::layer( BcU32 SortOrder )
{
	SortOrder_ = SortOrder;
}

inline void RsFontStyle::hAlign( RsFontStyle::eHAlign HAlignment )
{
	HAlignment_ = HAlignment;
}

inline void RsFontStyle::colour( const RsColour& Colour )
{
	Colour_ = Colour;
}

inline BcU32 RsFontStyle::layer() const
{
	return SortOrder_;
}

inline RsFontStyle::eHAlign RsFontStyle::hAlign() const
{
	return HAlignment_;
}

inline const RsColour& RsFontStyle::colour() const
{
	return Colour_;
}

//////////////////////////////////////////////////////////////////////////
// RsFont
class RsFont
{
public:
	//DECLARE_RESOURCETYPE( rt_RS_FONT );

public:
	enum
	{
		MAX_PRINTED_CHARS = 4096,
		MAX_MATERIALS = 8,
	};

public:
	RsFont();
	virtual ~RsFont();

	BcReal drawText( const RsFontStyle& Style, const BcVec2d& Position, const BcVec2d& Dimensions, const BcU32 SortOrder, const BcChar* pText, ... );
	
	BcReal textHeight( const BcVec2d& Dimensions, const BcChar* pText, ... );

protected:
	friend class CsArchive;
	friend class RsCoreImpl;
	friend class RsCoreImplGL;
	friend class RsCoreImplDX9;

	//
	BcReal internalDraw( const RsFontStyle& Style, const BcVec2d& Position, const BcVec2d& Dimensions, const BcU32 Layer );

	//
	void unpackText( const BcChar* pText, BcU32 Length, va_list Args );

	// Utility
	BcReal lineWidth( BcU32 iFirstChar );
	void breakUpLines( BcReal MaxWidth );

	// Overrides
	virtual void startDraw( const RsFontStyle& Style, BcU32 nChars ) = 0;
	virtual void makeCharacter( const RsFontChar& Character, const BcVec2d& Position ) = 0;
	virtual void submitCharacters( BcU32 Layer, CsResourceHandle< RsTexture > Texture ) = 0;

	RsFontFileData* pFileData_;
	
	std::map< BcU32, RsFontChar >	CharacterMap_;
	std::array< CsResourceHandle< RsMaterial >, MAX_MATERIALS > aMaterials_;
	std::array< RsFontCharIdx, MAX_PRINTED_CHARS > PrintedChars_;
		
	BcU32 nPrintedChars_;
};

#endif
