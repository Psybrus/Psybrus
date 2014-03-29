/**************************************************************************
*
* File:		RsStateBlockGL.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsStateBlockGL_H__
#define __RsStateBlockGL_H__

#include "System/Renderer/RsStateBlock.h"

//////////////////////////////////////////////////////////////////////////
// RsStateBlockGL
class RsStateBlockGL:
	public RsStateBlock
{
public:
	RsStateBlockGL();
	virtual ~RsStateBlockGL();
	
	virtual void					bind();
	
protected:
	void							bindStencilFunc();
	void							bindStencilOp();
	void							bindBlendMode( eRsBlendingMode BlendMode );
	void							bindScissor();

private:
	BcU32							VAO_; // temp.
	
};


#endif


