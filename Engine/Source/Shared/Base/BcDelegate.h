/**************************************************************************
*
* File:		BcDelegate.h
* Author:	Neil Richardson & Autogeneration Script
* Ver/Date:
* Description:
*		Function object.
*
*
*
**************************************************************************/

#ifndef __BCDELEGATE_H__
#define __BCDELEGATE_H__

#include "BcTypes.h"
#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcFuncTraits

template< typename _Fn >
struct BcFuncTraits;

template< typename _R >
struct BcFuncTraits< _R(*)() >
{
	static const int PARAMS = 0;
	typedef _R return_type;
	typedef _R(*signature_type)();
};

template< typename _R, typename _P0 >
struct BcFuncTraits< _R(*)(_P0) >
{
	static const int PARAMS = 1;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _R(*signature_type)(_P0);
};

template< typename _R, typename _P0, typename _P1 >
struct BcFuncTraits< _R(*)(_P0, _P1) >
{
	static const int PARAMS = 2;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _R(*signature_type)(_P0, _P1);
};

template< typename _R, typename _P0, typename _P1, typename _P2 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2) >
{
	static const int PARAMS = 3;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _R(*signature_type)(_P0, _P1, _P2);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2, _P3) >
{
	static const int PARAMS = 4;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4) >
{
	static const int PARAMS = 5;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5) >
{
	static const int PARAMS = 6;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6) >
{
	static const int PARAMS = 7;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6);
};

template< typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6, typename _P7 >
struct BcFuncTraits< _R(*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7) >
{
	static const int PARAMS = 8;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _P7 param7_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7);
};

template< typename _Ty, typename _R >
struct BcFuncTraits< _R(_Ty::*)() >
{
	static const int PARAMS = 0;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _R(*signature_type)();
};

template< typename _Ty, typename _R, typename _P0 >
struct BcFuncTraits< _R(_Ty::*)(_P0) >
{
	static const int PARAMS = 1;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _R(*signature_type)(_P0);
};

template< typename _Ty, typename _R, typename _P0, typename _P1 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1) >
{
	static const int PARAMS = 2;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _R(*signature_type)(_P0, _P1);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2) >
{
	static const int PARAMS = 3;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _R(*signature_type)(_P0, _P1, _P2);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3) >
{
	static const int PARAMS = 4;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4) >
{
	static const int PARAMS = 5;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5) >
{
	static const int PARAMS = 6;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6) >
{
	static const int PARAMS = 7;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6);
};

template< typename _Ty, typename _R, typename _P0, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6, typename _P7 >
struct BcFuncTraits< _R(_Ty::*)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7) >
{
	static const int PARAMS = 8;
	typedef _Ty class_type;
	typedef _R return_type;
	typedef _P0 param0_type;
	typedef _P1 param1_type;
	typedef _P2 param2_type;
	typedef _P3 param3_type;
	typedef _P4 param4_type;
	typedef _P5 param5_type;
	typedef _P6 param6_type;
	typedef _P7 param7_type;
	typedef _R(*signature_type)(_P0, _P1, _P2, _P3, _P4, _P5, _P6, _P7);
};

//////////////////////////////////////////////////////////////////////////
// _BcDelegateInternal

template< typename _Fn, int >
class _BcDelegateInternal;

//////////////////////////////////////////////////////////////////////////
// BcDelegateCallBase

class BcDelegateCallBase
{
public:
	BcDelegateCallBase():
		HasBeenCalled_( BcTrue )
	{

	}

	virtual ~BcDelegateCallBase(){}
	virtual void operator()() = 0;

	BcBool hasBeenCalled() const
	{
		return HasBeenCalled_;
	}

	void resetHasBeenCalled()
	{
		HasBeenCalled_ = BcFalse;
	}
	protected:
		BcBool HasBeenCalled_;
	};
template< typename _Fn >
class _BcDelegateInternal< _Fn, 0 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()()
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 0 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 0 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)() >
	static _BcDelegateInternal< _Fn, 0 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 0 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void* )
	{
		return (*func)(  );
	}

	template< class _Ty, return_type (_Ty::*meth)() >
	static return_type method_stub( void* pObj )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)(  );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 1 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 1 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 1 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type) >
	static _BcDelegateInternal< _Fn, 1 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 1 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0 )
	{
		return (*func)( P0 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 2 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 2 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 2 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type) >
	static _BcDelegateInternal< _Fn, 2 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 2 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1 )
	{
		return (*func)( P0, P1 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 3 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 3 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 3 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type) >
	static _BcDelegateInternal< _Fn, 3 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 3 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2 )
	{
		return (*func)( P0, P1, P2 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 4 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2, P3);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 4 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 4 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type) >
	static _BcDelegateInternal< _Fn, 4 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 4 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3 )
	{
		return (*func)( P0, P1, P2, P3 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 5 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2, P3, P4);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 5 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 5 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type) >
	static _BcDelegateInternal< _Fn, 5 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 5 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4 )
	{
		return (*func)( P0, P1, P2, P3, P4 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 6 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2, P3, P4, P5);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 6 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 6 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type) >
	static _BcDelegateInternal< _Fn, 6 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 6 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 7 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::param6_type param6_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2, P3, P4, P5, P6);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 7 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 7 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type) >
	static _BcDelegateInternal< _Fn, 7 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 7 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5, P6 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5, P6 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};

template< typename _Fn >
class _BcDelegateInternal< _Fn, 8 >
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::param6_type param6_type;
	typedef typename BcFuncTraits< _Fn >::param7_type param7_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type, typename BcFuncTraits< _Fn >::param7_type);
public:
	BcForceInline _BcDelegateInternal(): pOwner_( NULL ), stubFunc_( NULL ){};
	BcForceInline return_type operator()(typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6, typename BcFuncTraits< _Fn >::param7_type P7)
	{
		BcAssert( stubFunc_ != NULL );
		return (*stubFunc_)(pOwner_, P0, P1, P2, P3, P4, P5, P6, P7);
	}

	BcForceInline BcBool isValid() const
	{
		return ( stubFunc_ != NULL );
	}

	BcForceInline void* getOwner()
	{
		return pOwner_;
	}

	template< _Fn _func >
	static _BcDelegateInternal< _Fn, 8 > bind( void* pOwner = NULL )
	{
		_BcDelegateInternal< _Fn, 8 > Func;
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &global_stub< _func >;
		return Func;
	}

	template< class _Ty, return_type(_Ty::*_func)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type, typename BcFuncTraits< _Fn >::param7_type) >
	static _BcDelegateInternal< _Fn, 8 > bind( _Ty* pOwner )
	{
		_BcDelegateInternal< _Fn, 8 > Func;
		BcAssert( pOwner != NULL );
		Func.pOwner_ = pOwner;
		Func.stubFunc_ = &method_stub< _Ty, _func >;
		return Func;
	}

private:
	template< _Fn func >
	static return_type global_stub( void*, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6, typename BcFuncTraits< _Fn >::param7_type P7 )
	{
		return (*func)( P0, P1, P2, P3, P4, P5, P6, P7 );
	}

	template< class _Ty, return_type (_Ty::*meth)(typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type, typename BcFuncTraits< _Fn >::param7_type) >
	static return_type method_stub( void* pObj, typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6, typename BcFuncTraits< _Fn >::param7_type P7 )
	{
		_Ty* pThis = static_cast< _Ty* >( pObj );
		return (pThis->*meth)( P0, P1, P2, P3, P4, P5, P6, P7 );
	}

private:
	void* pOwner_;
	stub_func stubFunc_;
};


//////////////////////////////////////////////////////////////////////////
// BcDelegate
template< typename _Fn >
class BcDelegate: public _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >
{
public:
	BcDelegate()
	{
		
	}
		
	BcDelegate( const _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}

	void operator = ( const _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
};
//////////////////////////////////////////////////////////////////////////
// _BcDelegateCallInternal

template< typename _Fn, int >
class _BcDelegateCallInternal;
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 0 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_(  );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 0 >& deferCall(  )
	{
		return (*this);
	}
private:
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 1 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 1 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0 )
	{
		P0_ = P0;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 2 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 2 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1 )
	{
		P0_ = P0;
		P1_ = P1;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 3 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 3 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 4 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_, P3_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 4 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		P3_ = P3;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	typename BcFuncTraits< _Fn >::param3_type P3_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 5 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_, P3_, P4_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 5 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		P3_ = P3;
		P4_ = P4;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	typename BcFuncTraits< _Fn >::param3_type P3_;
	typename BcFuncTraits< _Fn >::param4_type P4_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 6 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_, P3_, P4_, P5_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 6 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		P3_ = P3;
		P4_ = P4;
		P5_ = P5;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	typename BcFuncTraits< _Fn >::param3_type P3_;
	typename BcFuncTraits< _Fn >::param4_type P4_;
	typename BcFuncTraits< _Fn >::param5_type P5_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 7 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::param6_type param6_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_, P3_, P4_, P5_, P6_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 7 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		P3_ = P3;
		P4_ = P4;
		P5_ = P5;
		P6_ = P6;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	typename BcFuncTraits< _Fn >::param3_type P3_;
	typename BcFuncTraits< _Fn >::param4_type P4_;
	typename BcFuncTraits< _Fn >::param5_type P5_;
	typename BcFuncTraits< _Fn >::param6_type P6_;
	BcDelegate< _Fn > Delegate_;
};
template< typename _Fn >
class _BcDelegateCallInternal< _Fn, 8 >:
	public BcDelegateCallBase
{
public:
	typedef typename BcFuncTraits< _Fn >::return_type return_type;
	typedef typename BcFuncTraits< _Fn >::param0_type param0_type;
	typedef typename BcFuncTraits< _Fn >::param1_type param1_type;
	typedef typename BcFuncTraits< _Fn >::param2_type param2_type;
	typedef typename BcFuncTraits< _Fn >::param3_type param3_type;
	typedef typename BcFuncTraits< _Fn >::param4_type param4_type;
	typedef typename BcFuncTraits< _Fn >::param5_type param5_type;
	typedef typename BcFuncTraits< _Fn >::param6_type param6_type;
	typedef typename BcFuncTraits< _Fn >::param7_type param7_type;
	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,typename BcFuncTraits< _Fn >::param0_type, typename BcFuncTraits< _Fn >::param1_type, typename BcFuncTraits< _Fn >::param2_type, typename BcFuncTraits< _Fn >::param3_type, typename BcFuncTraits< _Fn >::param4_type, typename BcFuncTraits< _Fn >::param5_type, typename BcFuncTraits< _Fn >::param6_type, typename BcFuncTraits< _Fn >::param7_type);
public:
	_BcDelegateCallInternal(){}
	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):
		Delegate_( Delegate )
	{
	}
	virtual ~_BcDelegateCallInternal(){}
	virtual void operator()()
	{
		Delegate_( P0_, P1_, P2_, P3_, P4_, P5_, P6_, P7_ );
		HasBeenCalled_ = BcTrue;
	}
	virtual _BcDelegateCallInternal< _Fn, 8 >& deferCall( typename BcFuncTraits< _Fn >::param0_type P0, typename BcFuncTraits< _Fn >::param1_type P1, typename BcFuncTraits< _Fn >::param2_type P2, typename BcFuncTraits< _Fn >::param3_type P3, typename BcFuncTraits< _Fn >::param4_type P4, typename BcFuncTraits< _Fn >::param5_type P5, typename BcFuncTraits< _Fn >::param6_type P6, typename BcFuncTraits< _Fn >::param7_type P7 )
	{
		P0_ = P0;
		P1_ = P1;
		P2_ = P2;
		P3_ = P3;
		P4_ = P4;
		P5_ = P5;
		P6_ = P6;
		P7_ = P7;
		return (*this);
	}
private:
	typename BcFuncTraits< _Fn >::param0_type P0_;
	typename BcFuncTraits< _Fn >::param1_type P1_;
	typename BcFuncTraits< _Fn >::param2_type P2_;
	typename BcFuncTraits< _Fn >::param3_type P3_;
	typename BcFuncTraits< _Fn >::param4_type P4_;
	typename BcFuncTraits< _Fn >::param5_type P5_;
	typename BcFuncTraits< _Fn >::param6_type P6_;
	typename BcFuncTraits< _Fn >::param7_type P7_;
	BcDelegate< _Fn > Delegate_;
};

//////////////////////////////////////////////////////////////////////////
// BcDelegateCall
template< typename _Fn >
class BcDelegateCall: public _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >
{
public:
	BcDelegateCall()
	{
	}
	
	BcDelegateCall( const BcDelegate< _Fn >& Delegate ):
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >( Delegate )
	{
	}
	
	BcDelegateCall( _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
	
	void operator = ( _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
	
	virtual ~BcDelegateCall(){}
};
#endif
