/**************************************************************************
*
* File:		CsResourceRef.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Resource handle
*
*
*
*
**************************************************************************/

#ifndef __CSRESOURCEREF_H__
#define __CSRESOURCEREF_H__

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsResource;

//////////////////////////////////////////////////////////////////////////
// CsResourceRef
template< class _Ty = CsResource, bool _IsWeak = false >
class CsResourceRef
{
private:
	CsResource* pObject_;

private:
	void _acquireNew( CsResource* pObject );
	void _acquireNewReleaseOld( CsResource* pObject );
	void _acquireAssign( CsResource* pObject );
	void _releaseThis();

public:
	CsResourceRef();
	CsResourceRef( const CsResourceRef& Other );
	CsResourceRef( CsResource* pObject );
	CsResourceRef& operator = ( const CsResourceRef& Other );
	CsResourceRef& operator = ( CsResource* pObject );
	~CsResourceRef();
	BcBool isValid() const;
	BcBool isReady() const;
	BcU32 refCount() const;
	operator _Ty* ();
	_Ty* operator -> ();
	BcBool operator == ( const CsResourceRef& Other ) const;
	BcBool operator != ( const CsResourceRef& Other ) const;
	BcBool operator == ( _Ty* pObject ) const;
	BcBool operator != ( _Ty* pObject ) const;
};

#endif
