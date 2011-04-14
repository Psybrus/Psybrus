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

#ifndef __BCFIXED_H__
#define __BCFIXED_H__

class BcFixed {

private:

	int	g; // the guts

	const static int BP= 16;  // how many low bits are right of Binary Point
	const static int BP2= BP*2;  // how many low bits are right of Binary Point
	const static int BPhalf= BP/2;  // how many low bits are right of Binary Point

	static BcF32 STEP() { return 1.0f / (1<<BP); }  // smallest step we can represent

	// for private construction via guts
	enum FixedRaw { RAW };
	BcFixed(FixedRaw, int guts) : g(guts) {}


public:
	BcFixed() : g(0) {}
	BcFixed(const BcFixed& a) : g( a.g ) {}
	BcFixed(BcF32 a) : g( int(a / (BcF32)STEP()) ) {}
	BcFixed(int a) : g( a << BP ) {}
	BcFixed(long a) : g( a << BP ) {}
	BcFixed& operator =(const BcFixed& a) { g= a.g; return *this; }
	BcFixed& operator =(BcF32 a) { g= BcFixed(a).g; return *this; }
	BcFixed& operator =(int a) { g= BcFixed(a).g; return *this; }
	BcFixed& operator =(long a) { g= BcFixed(a).g; return *this; }

	operator BcF32() const { return g * (BcF32)STEP(); }
	operator int() const { return g>>BP; }
	operator long() const { return g>>BP; }

	BcReal asReal() const { return g * (BcReal)STEP(); }

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

inline BcFixed operator +(BcF32 a, const BcFixed& b) { return BcFixed(a)+b; }
inline BcFixed operator -(BcF32 a, const BcFixed& b) { return BcFixed(a)-b; }
inline BcFixed operator *(BcF32 a, const BcFixed& b) { return BcFixed(a)*b; }
inline BcFixed operator /(BcF32 a, const BcFixed& b) { return BcFixed(a)/b; }

inline bool operator ==(BcF32 a, const BcFixed& b) { return BcFixed(a) == b; }
inline bool operator !=(BcF32 a, const BcFixed& b) { return BcFixed(a) != b; }
inline bool operator <=(BcF32 a, const BcFixed& b) { return BcFixed(a) <= b; }
inline bool operator >=(BcF32 a, const BcFixed& b) { return BcFixed(a) >= b; }
inline bool operator  <(BcF32 a, const BcFixed& b) { return BcFixed(a)  < b; }
inline bool operator  >(BcF32 a, const BcFixed& b) { return BcFixed(a)  > b; }


inline int& operator +=(int& a, const BcFixed& b) { a = (BcFixed)a + b; return a; }
inline int& operator -=(int& a, const BcFixed& b) { a = (BcFixed)a - b; return a; }
inline int& operator *=(int& a, const BcFixed& b) { a = (BcFixed)a * b; return a; }
inline int& operator /=(int& a, const BcFixed& b) { a = (BcFixed)a / b; return a; }

inline long& operator +=(long& a, const BcFixed& b) { a = (BcFixed)a + b; return a; }
inline long& operator -=(long& a, const BcFixed& b) { a = (BcFixed)a - b; return a; }
inline long& operator *=(long& a, const BcFixed& b) { a = (BcFixed)a * b; return a; }
inline long& operator /=(long& a, const BcFixed& b) { a = (BcFixed)a / b; return a; }

inline BcF32& operator +=(BcF32& a, const BcFixed& b) { a = a + b; return a; }
inline BcF32& operator -=(BcF32& a, const BcFixed& b) { a = a - b; return a; }
inline BcF32& operator *=(BcF32& a, const BcFixed& b) { a = a * b; return a; }
inline BcF32& operator /=(BcF32& a, const BcFixed& b) { a = a / b; return a; }

#endif
