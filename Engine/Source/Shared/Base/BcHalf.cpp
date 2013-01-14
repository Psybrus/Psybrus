// Branch-free implementation of half-precision (16 bit) floating point
// Copyright 2006 Mike Acton <macton@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE
//
// Half-precision floating point format
// ------------------------------------
//
//   | Field    | Last | First | Note
//   |----------|------|-------|----------
//   | Sign     | 15   | 15    |
//   | Exponent | 14   | 10    | Bias = 15
//   | Mantissa | 9    | 0     |
//
// Compiling
// ---------
//
//  Preferred compile flags for GCC: 
//     -O3 -fstrict-aliasing -std=c99 -pedantic -Wall -Wstrict-aliasing
//
//     This file is a C99 source file, intended to be compiled with a C99 
//     compliant compiler. However, for the moment it remains combatible
//     with C++98. Therefore if you are using a compiler that poorly implements
//     C standards (e.g. MSVC), it may be compiled as C++. This is not
//     guaranteed for future versions. 
//
// Features
// --------
//
//  * QNaN + <x>  = QNaN
//  * <x>  + +INF = +INF
//  * <x>  - -INF = -INF
//  * INF  - INF  = SNaN
//  * Denormalized values
//  * Difference of ZEROs is always +ZERO
//  * Sum round with guard + round + sticky bit (grs)
//  * And of course... no branching
// 
// Precision of Sum
// ----------------
//
//  (SUM)        BcU16 z = half_add( x, y );
//  (DIFFERENCE) BcU16 z = half_add( x, -y );
//
//     Will have exactly (0 ulps difference) the same result as:
//     (For 32 bit IEEE 784 floating point and same rounding mode)
//
//     union FLOAT_32
//     {
//       float    f32;
//       BcU32 u32;
//     };
//
//     union FLOAT_32 fx = { .u32 = BcHalfToF32( x ) };
//     union FLOAT_32 fy = { .u32 = BcHalfToF32( y ) };
//     union FLOAT_32 fz = { .f32 = fx.f32 + fy.f32    };
//     BcU16       z  = float_to_half( fz );
//

#include "BcHalf.h"
#include <stdio.h>

union BcHalfFloat
{
	float f32;
	BcU32 u32;
};

#if defined( COMPILER_MSVC )
#pragma warning( disable : 4146 )
#pragma optimize( "", on )
#endif

// Load immediate
static inline BcU32 _uint32_li( BcU32 a )
{
  return (a);
}

// Decrement
static inline BcU32 _uint32_dec( BcU32 a )
{
  return (a - 1);
}

// Complement
static inline BcU32 _uint32_not( BcU32 a )
{
  return (~a);
}

// Negate
static inline BcU32 _uint32_neg( BcU32 a )
{
  return ( BcU32 ) (-((BcS32)a) );
}

// Extend sign
static inline BcU32 _uint32_ext( BcU32 a )
{
  return ( BcU32 ) (((BcS32)a)>>31);
}

// And
static inline BcU32 _uint32_and( BcU32 a, BcU32 b )
{
  return (a & b);
}

// And with Complement
static inline BcU32 _uint32_andc( BcU32 a, BcU32 b )
{
  return (a & ~b);
}

// Or
static inline BcU32 _uint32_or( BcU32 a, BcU32 b )
{
  return (a | b);
}

// Shift Right Logical
static inline BcU32 _uint32_srl( BcU32 a, int sa )
{
  return (a >> sa);
}

// Shift Left Logical
static inline BcU32 _uint32_sll( BcU32 a, int sa )
{
  return (a << sa);
}

// Add
static inline BcU32 _uint32_add( BcU32 a, BcU32 b )
{
  return (a + b);
}

// Subtract
static inline BcU32 _uint32_sub( BcU32 a, BcU32 b )
{
  return (a - b);
}

// Select on Sign bit
static inline BcU32 _uint32_sels( BcU32 test, BcU32 a, BcU32 b )
{
  const BcU32 mask   = _uint32_ext( test );
  const BcU32 sel_a  = _uint32_and(  a,     mask  );
  const BcU32 sel_b  = _uint32_andc( b,     mask  );
  const BcU32 result = _uint32_or(   sel_a, sel_b );

  return (result);
}

// Load Immediate
static inline BcU16 _uint16_li( BcU16 a )
{
  return (a);
}

// Extend sign
static inline BcU16 _uint16_ext( BcU16 a )
{
  return ( BcU16)(((BcS16)a)>>15);
}

// Negate
static inline BcU16 _uint16_neg( BcU16 a )
{
  return ( BcU16)(-a);
}

// Complement
static inline BcU16 _uint16_not( BcU16 a )
{
  return ( BcU16)(~a);
}

// Decrement
static inline BcU16 _uint16_dec( BcU16 a )
{
  return (BcU16)(a - 1);
}

// Shift Left Logical
static inline BcU16 _uint16_sll( BcU16 a, int sa )
{
  return (BcU16) (a << sa);
}

// Shift Right Logical
static inline BcU16 _uint16_srl( BcU16 a, int sa )
{
  return (BcU16) (a >> sa);
}

// Add
static inline BcU16 _uint16_add( BcU16 a, BcU16 b )
{
  return (BcU16) (a + b);
}

// Subtract
static inline BcU16 _uint16_sub( BcU16 a, BcU16 b )
{
  return (BcU16)(a - b);
}

// And
static inline BcU16 _uint16_and( BcU16 a, BcU16 b )
{
  return (a & b);
}

// Or
static inline BcU16 _uint16_or( BcU16 a, BcU16 b )
{
  return (a | b);
}

// Exclusive Or
static inline BcU16 _uint16_xor( BcU16 a, BcU16 b )
{
  return (a ^ b);
}

// And with Complement
static inline BcU16 _uint16_andc( BcU16 a, BcU16 b )
{
  return (a & ~b);
}

// And then Shift Right Logical
static inline BcU16 _uint16_andsrl( BcU16 a, BcU16 b, int sa )
{
  return (BcU16) ((a & b) >> sa);
}

// Shift Right Logical then Mask
static inline BcU16 _uint16_srlm( BcU16 a, int sa, BcU16 mask )
{
  return (BcU16) ((a >> sa) & mask);
}

// Add then Mask
static inline BcU16 _uint16_addm( BcU16 a, BcU16 b, BcU16 mask )
{
  return (BcU16) ((a + b) & mask);
}


// Select on Sign bit
static inline BcU16 _uint16_sels( BcU16 test, BcU16 a, BcU16 b )
{
  const BcU16 mask   = _uint16_ext( test );
  const BcU16 sel_a  = _uint16_and(  a,     mask  );
  const BcU16 sel_b  = _uint16_andc( b,     mask  );
  const BcU16 result = _uint16_or(   sel_a, sel_b );

  return (result);
}

// Count Leading Zeros
static inline BcU32 _uint32_cntlz( BcU32 x )
{
  const BcU32 x0  = _uint32_srl(  x,  1 );
  const BcU32 x1  = _uint32_or(   x,  x0 );
  const BcU32 x2  = _uint32_srl(  x1, 2 );
  const BcU32 x3  = _uint32_or(   x1, x2 );
  const BcU32 x4  = _uint32_srl(  x3, 4 );
  const BcU32 x5  = _uint32_or(   x3, x4 );
  const BcU32 x6  = _uint32_srl(  x5, 8 );
  const BcU32 x7  = _uint32_or(   x5, x6 );
  const BcU32 x8  = _uint32_srl(  x7, 16 );
  const BcU32 x9  = _uint32_or(   x7, x8 );
  const BcU32 xA  = _uint32_not(  x9 );
  const BcU32 xB  = _uint32_srl(  xA, 1 );
  const BcU32 xC  = _uint32_and(  xB, 0x55555555 );
  const BcU32 xD  = _uint32_sub(  xA, xC );
  const BcU32 xE  = _uint32_and(  xD, 0x33333333 );
  const BcU32 xF  = _uint32_srl(  xD, 2 );
  const BcU32 x10 = _uint32_and(  xF, 0x33333333 );
  const BcU32 x11 = _uint32_add(  xE, x10 );
  const BcU32 x12 = _uint32_srl(  x11, 4 );
  const BcU32 x13 = _uint32_add(  x11, x12 );
  const BcU32 x14 = _uint32_and(  x13, 0x0f0f0f0f );
  const BcU32 x15 = _uint32_srl(  x14, 8 );
  const BcU32 x16 = _uint32_add(  x14, x15 );
  const BcU32 x17 = _uint32_srl(  x16, 16 );
  const BcU32 x18 = _uint32_add(  x16, x17 );
  const BcU32 x19 = _uint32_and(  x18, 0x0000003f );
  return ( x19 );
}

// Count Leading Zeros
static inline BcU16 _uint16_cntlz( BcU16 x )
{
  const BcU16 x0  = _uint16_srl(  x,  1 );
  const BcU16 x1  = _uint16_or(   x,  x0 );
  const BcU16 x2  = _uint16_srl(  x1, 2 );
  const BcU16 x3  = _uint16_or(   x1, x2 );
  const BcU16 x4  = _uint16_srl(  x3, 4 );
  const BcU16 x5  = _uint16_or(   x3, x4 );
  const BcU16 x6  = _uint16_srl(  x5, 8 );
  const BcU16 x7  = _uint16_or(   x5, x6 );
  const BcU16 x8  = _uint16_not(  x7 );
  const BcU16 x9  = _uint16_srlm( x8, 1, 0x5555 );
  const BcU16 xA  = _uint16_sub(  x8, x9 );
  const BcU16 xB  = _uint16_and(  xA, 0x3333 );
  const BcU16 xC  = _uint16_srlm( xA, 2, 0x3333 );
  const BcU16 xD  = _uint16_add(  xB, xC );
  const BcU16 xE  = _uint16_srl(  xD, 4 );
  const BcU16 xF  = _uint16_addm( xD, xE, 0x0f0f );
  const BcU16 x10 = _uint16_srl(  xF, 8 );
  const BcU16 x11 = _uint16_addm( xF, x10, 0x001f );
  return ( x11 );
}

BcU16 BcF32ToHalf_( BcU32 f )
{
  const BcU32 one                        = _uint32_li( 0x00000001 );
  const BcU32 f_e_mask                   = _uint32_li( 0x7f800000 );
  const BcU32 f_m_mask                   = _uint32_li( 0x007fffff );
  const BcU32 f_s_mask                   = _uint32_li( 0x80000000 );
  const BcU32 h_e_mask                   = _uint32_li( 0x00007c00 );
  const BcU32 f_e_pos                    = _uint32_li( 0x00000017 );
  const BcU32 f_m_round_bit              = _uint32_li( 0x00001000 );
  const BcU32 h_nan_em_min               = _uint32_li( 0x00007c01 );
  const BcU32 f_h_s_pos_offset           = _uint32_li( 0x00000010 );
  const BcU32 f_m_hidden_bit             = _uint32_li( 0x00800000 );
  const BcU32 f_h_m_pos_offset           = _uint32_li( 0x0000000d );
  const BcU32 f_h_bias_offset            = _uint32_li( 0x38000000 );
  const BcU32 f_m_snan_mask              = _uint32_li( 0x003fffff );
  const BcU16 h_snan_mask                = (BcU16) _uint32_li( 0x00007e00 );
  const BcU32 f_e                        = _uint32_and( f, f_e_mask  );
  const BcU32 f_m                        = _uint32_and( f, f_m_mask  );
  const BcU32 f_s                        = _uint32_and( f, f_s_mask  );
  const BcU32 f_e_h_bias                 = _uint32_sub( f_e,               f_h_bias_offset );
  const BcU32 f_e_h_bias_amount          = _uint32_srl( f_e_h_bias,        f_e_pos         );
  const BcU32 f_m_round_mask             = _uint32_and( f_m,               f_m_round_bit     );
  const BcU32 f_m_round_offset           = _uint32_sll( f_m_round_mask,    one               );
  const BcU32 f_m_rounded                = _uint32_add( f_m,               f_m_round_offset  );
  const BcU32 f_m_rounded_overflow       = _uint32_and( f_m_rounded,       f_m_hidden_bit    );
  const BcU32 f_m_denorm_sa              = _uint32_sub( one,               f_e_h_bias_amount );
  const BcU32 f_m_with_hidden            = _uint32_or(  f_m_rounded,       f_m_hidden_bit    );
  const BcU32 f_m_denorm                 = _uint32_srl( f_m_with_hidden,   f_m_denorm_sa     );
  const BcU32 f_em_norm_packed           = _uint32_or(  f_e_h_bias,        f_m_rounded       );
  const BcU32 f_e_overflow               = _uint32_add( f_e_h_bias,        f_m_hidden_bit    );
  const BcU32 h_s                        = _uint32_srl( f_s,               f_h_s_pos_offset );
  const BcU32 h_m_nan                    = _uint32_srl( f_m,               f_h_m_pos_offset );
  const BcU32 h_m_denorm                 = _uint32_srl( f_m_denorm,        f_h_m_pos_offset );
  const BcU32 h_em_norm                  = _uint32_srl( f_em_norm_packed,  f_h_m_pos_offset );
  const BcU32 h_em_overflow              = _uint32_srl( f_e_overflow,      f_h_m_pos_offset );
  const BcU32 is_e_eqz_msb               = _uint32_dec(  f_e     );
  const BcU32 is_m_nez_msb               = _uint32_neg(  f_m     );
  const BcU32 is_h_m_nan_nez_msb         = _uint32_neg(  h_m_nan );
  const BcU32 is_e_nflagged_msb          = _uint32_sub(  f_e,                 f_e_mask          );
  const BcU32 is_ninf_msb                = _uint32_or(   is_e_nflagged_msb,   is_m_nez_msb      );
  const BcU32 is_underflow_msb           = _uint32_sub(  is_e_eqz_msb,        f_h_bias_offset   );
  const BcU32 is_nan_nunderflow_msb      = _uint32_or(   is_h_m_nan_nez_msb,  is_e_nflagged_msb );
  const BcU32 is_m_snan_msb              = _uint32_sub(  f_m_snan_mask,       f_m               );
  const BcU32 is_snan_msb                = _uint32_andc( is_m_snan_msb,       is_e_nflagged_msb );
  const BcU32 is_overflow_msb            = _uint32_neg(  f_m_rounded_overflow );
  const BcU32 h_nan_underflow_result     = _uint32_sels( is_nan_nunderflow_msb, h_em_norm,                h_nan_em_min       );
  const BcU32 h_inf_result               = _uint32_sels( is_ninf_msb,           h_nan_underflow_result,   h_e_mask           );
  const BcU32 h_underflow_result         = _uint32_sels( is_underflow_msb,      h_m_denorm,               h_inf_result       );
  const BcU32 h_overflow_result          = _uint32_sels( is_overflow_msb,       h_em_overflow,            h_underflow_result );
  const BcU32 h_em_result                = _uint32_sels( is_snan_msb,           h_snan_mask,              h_overflow_result  );
  const BcU32 h_result                   = _uint32_or( h_em_result, h_s );

  return (BcU16) (h_result);
}

BcU32 BcHalfToF32_( BcU16 h )
{
  const BcU32 h_e_mask              = _uint32_li( 0x00007c00 );
  const BcU32 h_m_mask              = _uint32_li( 0x000003ff );
  const BcU32 h_s_mask              = _uint32_li( 0x00008000 );
  const BcU32 h_f_s_pos_offset      = _uint32_li( 0x00000010 );
  const BcU32 h_f_e_pos_offset      = _uint32_li( 0x0000000d );
  const BcU32 h_f_bias_offset       = _uint32_li( 0x0001c000 );
  const BcU32 f_e_mask              = _uint32_li( 0x7f800000 );
  const BcU32 f_m_mask              = _uint32_li( 0x007fffff );
  const BcU32 h_f_e_denorm_bias     = _uint32_li( 0x0000007e );
  const BcU32 h_f_m_denorm_sa_bias  = _uint32_li( 0x00000008 );
  const BcU32 f_e_pos               = _uint32_li( 0x00000017 );
  const BcU32 h_e_mask_minus_one    = _uint32_li( 0x00007bff );
  const BcU32 h_e                   = _uint32_and( h, h_e_mask );
  const BcU32 h_m                   = _uint32_and( h, h_m_mask );
  const BcU32 h_s                   = _uint32_and( h, h_s_mask );
  const BcU32 h_e_f_bias            = _uint32_add( h_e, h_f_bias_offset );
  const BcU32 h_m_nlz               = _uint32_cntlz( h_m );
  const BcU32 f_s                   = _uint32_sll( h_s,        h_f_s_pos_offset );
  const BcU32 f_e                   = _uint32_sll( h_e_f_bias, h_f_e_pos_offset );
  const BcU32 f_m                   = _uint32_sll( h_m,        h_f_e_pos_offset );
  const BcU32 f_em                  = _uint32_or(  f_e,        f_m              );
  const BcU32 h_f_m_sa              = _uint32_sub( h_m_nlz,             h_f_m_denorm_sa_bias );
  const BcU32 f_e_denorm_unpacked   = _uint32_sub( h_f_e_denorm_bias,   h_f_m_sa             );
  const BcU32 h_f_m                 = _uint32_sll( h_m,                 h_f_m_sa             );
  const BcU32 f_m_denorm            = _uint32_and( h_f_m,               f_m_mask             );
  const BcU32 f_e_denorm            = _uint32_sll( f_e_denorm_unpacked, f_e_pos              );
  const BcU32 f_em_denorm           = _uint32_or(  f_e_denorm,          f_m_denorm           );
  const BcU32 f_em_nan              = _uint32_or(  f_e_mask,            f_m                  );
  const BcU32 is_e_eqz_msb          = _uint32_dec(  h_e );
  const BcU32 is_m_nez_msb          = _uint32_neg(  h_m );
  const BcU32 is_e_flagged_msb      = _uint32_sub(  h_e_mask_minus_one, h_e );
  const BcU32 is_zero_msb           = _uint32_andc( is_e_eqz_msb,       is_m_nez_msb );
  const BcU32 is_inf_msb            = _uint32_andc( is_e_flagged_msb,   is_m_nez_msb );
  const BcU32 is_denorm_msb         = _uint32_and(  is_m_nez_msb,       is_e_eqz_msb );
  const BcU32 is_nan_msb            = _uint32_and(  is_e_flagged_msb,   is_m_nez_msb ); 
  const BcU32 is_zero               = _uint32_ext(  is_zero_msb );
  const BcU32 f_zero_result         = _uint32_andc( f_em, is_zero );
  const BcU32 f_denorm_result       = _uint32_sels( is_denorm_msb, f_em_denorm, f_zero_result );
  const BcU32 f_inf_result          = _uint32_sels( is_inf_msb,    f_e_mask,    f_denorm_result );
  const BcU32 f_nan_result          = _uint32_sels( is_nan_msb,    f_em_nan,    f_inf_result    );
  const BcU32 f_result              = _uint32_or( f_s, f_nan_result );
 
  return (f_result);
}

BcU16 BcF32ToHalf( BcF32 f )
{
	BcHalfFloat halfFloat;
	halfFloat.f32 = f;
	return BcF32ToHalf_( halfFloat.u32 );
}

BcF32 BcHalfToF32( BcU16 h )
{
	BcHalfFloat halfFloat;
	halfFloat.u32 = BcHalfToF32_( h );
	return halfFloat.f32;
}
