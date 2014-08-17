#ifndef __BCCOMREF_H__
#define __BCCOMREF_H__

#include "Base/BcWindows.h"
#include "Base/BcDebug.h"

#include <Unknwn.h>

//////////////////////////////////////////////////////////////////////////
/**
	* @class BcComRef
	* @brief COM object reference.
	*/
template< class _Ty = IUnknown >
class BcComRef
{
private:
	IUnknown* pObject_;

private:
	inline void _acquireNew( IUnknown* pObject );
	inline void _acquireNewReleaseOld( IUnknown* pObject );
	inline void _acquireAssign( IUnknown* pObject );
	inline void _releaseThis();
	inline static void assertPendingDeletion( const IUnknown* pObject );

public:
    inline BcComRef();
    inline BcComRef( const BcComRef& Other );
    inline BcComRef( IUnknown* pObject );
    inline BcComRef& operator = ( const BcComRef& Other );
    inline BcComRef& operator = ( IUnknown* pObject );
    inline ~BcComRef();
	inline bool isValid() const;
	inline operator _Ty* ();
	inline operator const _Ty* () const;
	inline _Ty* operator -> ();
	inline const _Ty* operator -> () const;
	inline _Ty** operator & ();
    inline bool operator == ( const BcComRef& Other ) const;
    inline bool operator != ( const BcComRef& Other ) const;
	inline bool operator == ( _Ty* pObject ) const;
	inline bool operator != ( _Ty* pObject ) const;
	inline void reset();
};

#include "Base/BcComRef.inl"

#endif __BCCOMREF_H__
