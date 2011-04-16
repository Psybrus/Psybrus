/**************************************************************************
*
* File:		RsProgram.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAM_H__
#define __RSPROGRAM_H__

#include "RsTypes.h"
#include "RsTexture.h"

////////////////////////////////////////////////////////////////////////////////
// RsProgramParameter
class RsProgramParameter
{
public:
	/**
	 * Get name.
	 * @return Name.
	 */
	const std::string&					getName() const;
	
public:
	
	/**
	 * Set int.
	 * @param Value Value.
	 */
	virtual void						setInt( BcS32 Value ) = 0;
	
	/**
	 * Set float.
	 * @param Value Value.
	 */
	virtual void						setFloat( BcReal Value ) = 0;
	
	/**
	 * Set vector.
	 * @param Value Value.
	 */
	virtual void						setVector( const BcVec2d& Value ) = 0;
	
	/**
	 * Set vector.
	 * @param Value Value.
	 */
	virtual void						setVector( const BcVec3d& Value ) = 0;
	
	/**
	 * Set vector.
	 * @param Value Value.
	 */
	virtual void						setVector( const BcVec4d& Value ) = 0;
	
	/**
	 * Set matrix.
	 * @param Value Value.
	 */
	virtual void						setMatrix( const BcMat4d& Value ) = 0;
		
protected:
	std::string							Name_;
	
protected:
	RsProgramParameter( const std::string& Name );
	virtual ~RsProgramParameter();
};

////////////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline const std::string& RsProgramParameter::getName() const
{
	return Name_;
}

////////////////////////////////////////////////////////////////////////////////
// RsProgram
class RsProgram:
	public RsResource
{
public:
	RsProgram();
	virtual ~RsProgram();
	
	/**
	 * Find parameter.
	 * @param Name Name of parameter.
	 * @return Pointer to program parameter object.
	 */
	virtual RsProgramParameter*			findParameter( const std::string& Name ) = 0;
	
	/**
	 * Bind program.
	 */
	virtual void						bind() = 0;
};
#endif
