/**************************************************************************
*
* File:		RsD3D11.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL includes.
*		
*
*
* 
**************************************************************************/

#ifndef __RSD3D11_H__
#define __RSD3D11_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"

////////////////////////////////////////////////////////////////////////////////
// Win32
#if PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  define NOGDICAPMASKS    
#  define NOMENUS         
#  define NORASTEROPS     
#  define OEMRESOURCE     
#  define NOATOM          
#  define NOCLIPBOARD     
#  define NODRAWTEXT      
#  define NONLS           
#  define NOMEMMGR        
#  define NOMETAFILE      
#  define NOMINMAX        
#  define NOOPENFILE      
#  define NOSCROLL        
#  define NOSERVICE       
#  define NOSOUND         
#  define NOTEXTMETRIC    
#  define NOWINOFFSETS    
#  define NOKANJI         
#  define NOHELP          
#  define NOPROFILER      
#  define NODEFERWINDOWPOS
#  define NOMCX
#endif

#include <d3d11.h>
#include <d3d11_1.h>

#endif

