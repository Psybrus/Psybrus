#ifndef __OBJECTREF_H__
#define __OBJECTREF_H__

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class Object;

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
template< class _Ty = Object >
class ObjectRef
{
private:
	Object* pObject_;

private:
	inline void _acquireNew( Object* pObject );
	inline void _acquireNewReleaseOld( Object* pObject );
	inline void _acquireAssign( Object* pObject );
	inline void _releaseThis();
	inline static void assertPendingDeletion( const Object* pObject );

public:
	inline ObjectRef();
	inline ObjectRef( const ObjectRef& Other );
	inline ObjectRef( Object* pObject );
	inline ObjectRef& operator = ( const ObjectRef& Other );
	inline ObjectRef& operator = ( Object* pObject );
	inline ~ObjectRef();
	inline bool isValid() const;
	inline operator _Ty* ();
	inline _Ty* operator -> ();
	inline bool operator == ( const ObjectRef& Other ) const;
	inline bool operator != ( const ObjectRef& Other ) const;
	inline bool operator == ( _Ty* pObject ) const;
	inline bool operator != ( _Ty* pObject ) const;
	inline void reset();
};

#endif __OBJECTREF_H__
