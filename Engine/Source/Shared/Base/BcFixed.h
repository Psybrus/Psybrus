/*
Copyright (c) 2006 Henry Strickland & Ryan Seto

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

(* http://www.opensource.org/licenses/mit-license.php *)
*/

// Neilo: Removed doubles, renamed class to BcFixed, replaced float with BcF32.
// Neilo: Templated to allow controlled number of bits and storage size.

#ifndef __BCFIXED_H__
#define __BCFIXED_H__

#include "Base/BcTypes.h"
#include "Base/BcMath.h"

template< typename _Ty = BcS32, int _BP = 16 >
class BcFixed
{
public:
	const static int BP = _BP;  // how many low bits are right of Binary Point
	const static int BP2 = BP*2;  // how many low bits are right of Binary Point
	const static int BPhalf = BP/2;  // how many low bits are right of Binary Point
	const static int IB = ( sizeof( _Ty ) * 8 ) - BP;  // integer bits

	static BcF32 STEP() { return 1.0f / (1<<BP); }  // smallest step we can represent

	// for private construction via guts
	enum FixedRaw { RAW };
	BcFixed(FixedRaw, _Ty guts) : g(guts) {}

private:
	_Ty	g; // the guts

public:
	BcFixed() : g(0) {}
	BcFixed(const BcFixed& a) : g( a.g ) {}
	BcFixed(BcF32 a) : g( _Ty(a / (BcF32)STEP()) ) {}
	BcFixed(int a) : g( a << BP ) {}
	BcFixed(long a) : g( a << BP ) {}
	BcFixed& operator =(const BcFixed& a) { g= a.g; return *this; }
	BcFixed& operator =(BcF32 a) { g= BcFixed(a).g; return *this; }
	BcFixed& operator =(int a) { g= BcFixed(a).g; return *this; }
	BcFixed& operator =(long a) { g= BcFixed(a).g; return *this; }

	operator BcF32() const { return g * (BcF32)STEP(); }
	operator int() const { return g>>BP; }
	operator long() const { return g>>BP; }

	BcF32 asReal() const { return g * (BcF32)STEP(); }

	int getGuts() const
	{
		return g;
	};

	BcFixed operator +() const { return BcFixed(RAW,g); }
	BcFixed operator -() const { return BcFixed(RAW,-g); }

	BcFixed operator +(const BcFixed& a) const { return BcFixed(RAW, g + a.g); }
	BcFixed operator -(const BcFixed& a) const { return BcFixed(RAW, g - a.g); }
#if 1
	// more acurate, using long long
	BcFixed operator *(const BcFixed& a) const { return BcFixed(RAW,  (int)( ((long long)g * (long long)a.g ) >> BP)); }
#else
	// faster, but with only half as many bits right of binary point
	BcFixed operator *(const BcFixed& a) const { return BcFixed(RAW, (g>>BPhalf) * (a.g>>BPhalf) ); }
#endif
	BcFixed operator /(const BcFixed& a) const { return BcFixed(RAW, int( (((long long)g << BP2) / (long long)(a.g)) >> BP) ); }

	BcFixed operator +(BcF32 a) const { return BcFixed(RAW, g + BcFixed(a).g); }
	BcFixed operator -(BcF32 a) const { return BcFixed(RAW, g - BcFixed(a).g); }
	BcFixed operator *(BcF32 a) const { return BcFixed(RAW, (g>>BPhalf) * (BcFixed(a).g>>BPhalf) ); }
	BcFixed operator /(BcF32 a) const { return BcFixed(RAW, int( (((long long)g << BP2) / (long long)(BcFixed(a).g)) >> BP) ); }

	BcFixed& operator +=(BcFixed a) { return *this = *this + a; return *this; }
	BcFixed& operator -=(BcFixed a) { return *this = *this - a; return *this; }
	BcFixed& operator *=(BcFixed a) { return *this = *this * a; return *this; }
	BcFixed& operator /=(BcFixed a) { return *this = *this / a; return *this; }

	BcFixed& operator +=(int a) { return *this = *this + (BcFixed)a; return *this; }
	BcFixed& operator -=(int a) { return *this = *this - (BcFixed)a; return *this; }
	BcFixed& operator *=(int a) { return *this = *this * (BcFixed)a; return *this; }
	BcFixed& operator /=(int a) { return *this = *this / (BcFixed)a; return *this; }

	BcFixed& operator +=(long a) { return *this = *this + (BcFixed)a; return *this; }
	BcFixed& operator -=(long a) { return *this = *this - (BcFixed)a; return *this; }
	BcFixed& operator *=(long a) { return *this = *this * (BcFixed)a; return *this; }
	BcFixed& operator /=(long a) { return *this = *this / (BcFixed)a; return *this; }

	BcFixed& operator +=(BcF32 a) { return *this = *this + a; return *this; }
	BcFixed& operator -=(BcF32 a) { return *this = *this - a; return *this; }
	BcFixed& operator *=(BcF32 a) { return *this = *this * a; return *this; }
	BcFixed& operator /=(BcF32 a) { return *this = *this / a; return *this; }

	bool operator ==(const BcFixed& a) const { return g == a.g; }
	bool operator !=(const BcFixed& a) const { return g != a.g; }
	bool operator <=(const BcFixed& a) const { return g <= a.g; }
	bool operator >=(const BcFixed& a) const { return g >= a.g; }
	bool operator  <(const BcFixed& a) const { return g  < a.g; }
	bool operator  >(const BcFixed& a) const { return g  > a.g; }

	bool operator ==(BcF32 a) const { return g == BcFixed(a).g; }
	bool operator !=(BcF32 a) const { return g != BcFixed(a).g; }
	bool operator <=(BcF32 a) const { return g <= BcFixed(a).g; }
	bool operator >=(BcF32 a) const { return g >= BcFixed(a).g; }
	bool operator  <(BcF32 a) const { return g  < BcFixed(a).g; }
	bool operator  >(BcF32 a) const { return g  > BcFixed(a).g; }
};

template< typename _Ty, int _BP >
inline BcFixed< _Ty, _BP > operator +(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)+b; }

template< typename _Ty, int _BP >
inline BcFixed< _Ty, _BP > operator -(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)-b; }

template< typename _Ty, int _BP >
inline BcFixed< _Ty, _BP > operator *(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)*b; }

template< typename _Ty, int _BP >
inline BcFixed< _Ty, _BP > operator /(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)/b; }

template< typename _Ty, int _BP >
inline bool operator ==(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a) == b; }

template< typename _Ty, int _BP >
inline bool operator !=(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a) != b; }

template< typename _Ty, int _BP >
inline bool operator <=(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a) <= b; }

template< typename _Ty, int _BP >
inline bool operator >=(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a) >= b; }

template< typename _Ty, int _BP >
inline bool operator  <(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)  < b; }

template< typename _Ty, int _BP >
inline bool operator  >(BcF32 a, const BcFixed< _Ty, _BP >& b) { return BcFixed< _Ty, _BP >(a)  > b; }

template< typename _Ty, int _BP >
inline int& operator +=(int& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a + b; return a; }

template< typename _Ty, int _BP >
inline int& operator -=(int& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a - b; return a; }

template< typename _Ty, int _BP >
inline int& operator *=(int& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a * b; return a; }

template< typename _Ty, int _BP >
inline int& operator /=(int& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a / b; return a; }

template< typename _Ty, int _BP >
inline long& operator +=(long& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a + b; return a; }

template< typename _Ty, int _BP >
inline long& operator -=(long& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a - b; return a; }

template< typename _Ty, int _BP >
inline long& operator *=(long& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a * b; return a; }

template< typename _Ty, int _BP >
inline long& operator /=(long& a, const BcFixed< _Ty, _BP >& b) { a = (BcFixed< _Ty, _BP >)a / b; return a; }

template< typename _Ty, int _BP >
inline BcF32& operator +=(BcF32& a, const BcFixed< _Ty, _BP >& b) { a = a + b; return a; }

template< typename _Ty, int _BP >
inline BcF32& operator -=(BcF32& a, const BcFixed< _Ty, _BP >& b) { a = a - b; return a; }

template< typename _Ty, int _BP >
inline BcF32& operator *=(BcF32& a, const BcFixed< _Ty, _BP >& b) { a = a * b; return a; }

template< typename _Ty, int _BP >
inline BcF32& operator /=(BcF32& a, const BcFixed< _Ty, _BP >& b) { a = a / b; return a; }

//////////////////////////////////////////////////////////////////////////
// BcSqrt
template< typename _Ty, int _BP >
inline BcFixed< _Ty, _BP > BcSqrt( BcFixed< _Ty, _BP > v )
{
	return BcFixed< _Ty, _BP >( BcFixed< _Ty, _BP >::RAW, (_Ty)BcSqrtFixed( v.getGuts(), _BP ) );
}

#endif
