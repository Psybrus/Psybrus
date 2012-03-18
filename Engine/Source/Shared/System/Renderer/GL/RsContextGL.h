/**************************************************************************
*
* File:		RsContextGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTGL_H__
#define __RSCONTEXTGL_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsContextGL
class RsContextGL:
	public RsContext
{
public:
	typedef BcDelegate< void(*)() > MakeCurrentDelegate;
	typedef BcDelegate< void(*)() > SwapBuffersDelegate;

public:
	RsContextGL( OsClient* pClient, RsContextGL* pParent );
	virtual ~RsContextGL();
	
	virtual BcU32						getWidth() const;
	virtual BcU32						getHeight() const;

	void								makeCurrent();
	void								swapBuffers();
	void								takeScreenshot();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
#if PLATFORM_WINDOWS
	HDC WindowDC_;
	HGLRC WindowRC_;

	static HDC LastWindowDC_;
	static HGLRC LastWindowRC_;

	RsContextGL* pParent_;
#endif
	BcBool ScreenshotRequested_;

};


#endif
