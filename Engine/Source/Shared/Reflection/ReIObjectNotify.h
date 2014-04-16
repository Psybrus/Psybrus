#ifndef __REFLECTION_IOBJECTNOTIFY_H__
#define __REFLECTION_IOBJECTNOTIFY_H__

//////////////////////////////////////////////////////////////////////////
// IObjectNotify
class ReIObjectNotify
{
public:
    ReIObjectNotify(){};
    virtual ~ReIObjectNotify(){};

	/**
		* @brief Notify on deletion.
		*/
	virtual void onObjectDeleted( class Object* Object ) = 0;
			
};

#endif // __REFLECTION_IOBJECTNOTIFY_H__
