#ifndef __REFLECTION_IOBJECTNOTIFY_H__
#define __REFLECTION_IOBJECTNOTIFY_H__

//////////////////////////////////////////////////////////////////////////
// IObjectNotify
class IObjectNotify
{
public:
	IObjectNotify(){};
	virtual ~IObjectNotify(){};

	/**
		* @brief Notify on deletion.
		*/
	virtual void onObjectDeleted( class Object* Object ) = 0;
			
};

#endif // __REFLECTION_IOBJECTNOTIFY_H__
