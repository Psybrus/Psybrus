#ifndef __OBJECTREF_H__
#define __OBJECTREF_H__

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class ReObject;

//////////////////////////////////////////////////////////////////////////
/**
	* @class ObjectRef
	* @brief Object reference.
	* 
	* Automatically handles inc/dec of ref count + sanity checking.
	* Should not be used to pass objects to functions. This is a sure fire
	* way to cause performance hits. It is intended solely for holding references
	* of objects when they are in use. Anything passing round object pointers
	* should hold an ObjectRef, or one of the parent calls should.
	*
	* Recommended practice is to use it only as a member of a class, or
	* local in the cases where the object may be lost due to an owning object
	* being destructed.
	*/
template< class _Ty = ReObject, bool _IsWeak = false >
class ReObjectRef
{
private:
	ReObject* pObject_;

private:
	inline void _acquireNew( ReObject* pObject );
	inline void _acquireNewReleaseOld( ReObject* pObject );
	inline void _acquireAssign( ReObject* pObject );
	inline void _releaseThis();
	inline static void assertPendingDeletion( const ReObject* pObject );

public:
    inline ReObjectRef();
    inline ReObjectRef( const ReObjectRef& Other );
    inline ReObjectRef( ReObject* pObject );
    inline ReObjectRef& operator = ( const ReObjectRef& Other );
    inline ReObjectRef& operator = ( ReObject* pObject );
    inline ~ReObjectRef();
	inline bool isValid() const;
	inline operator _Ty* ();
	inline operator const _Ty* () const;
	inline _Ty* operator -> ();
	inline const _Ty* operator -> () const;
    inline bool operator == ( const ReObjectRef& Other ) const;
    inline bool operator != ( const ReObjectRef& Other ) const;
	inline bool operator == ( _Ty* pObject ) const;
	inline bool operator != ( _Ty* pObject ) const;
	inline void reset();
};

#endif __OBJECTREF_H__
