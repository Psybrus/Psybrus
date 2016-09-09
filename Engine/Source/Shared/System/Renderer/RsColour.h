#pragma once

#include "Base/BcTypes.h"
#include "Math/MaVec4d.h"

//////////////////////////////////////////////////////////////////////////
// Datatypes
class RsColour:
	public MaVec4d
{
public:
	REFLECTION_DECLARE_BASIC( RsColour );

public:
	RsColour();
	RsColour( const MaVec4d& Vec );
	RsColour( BcU32 RGBA );
	RsColour( BcF32 R, BcF32 G, BcF32 B, BcF32 A );

	BcU32 asRGBA() const;

	inline BcF32 r() const { return x(); };
	inline BcF32 g() const { return y(); };
	inline BcF32 b() const { return z(); };
	inline BcF32 a() const { return w(); };

	inline void r( BcF32 V ){ x( V ); };
	inline void g( BcF32 V ){ y( V ); };
	inline void b( BcF32 V ){ z( V ); };
	inline void a( BcF32 V ){ w( V ); };

public:
	static RsColour FromHTMLColourCode( const BcChar* String );

	static const RsColour WHITE;
	static const RsColour BLACK;
	static const RsColour GRAY;
	static const RsColour RED;
	static const RsColour GREEN;
	static const RsColour BLUE;
	static const RsColour ORANGE;
	static const RsColour YELLOW;
	static const RsColour MAGENTA;
	static const RsColour PURPLE;
	static const RsColour CYAN;
};
