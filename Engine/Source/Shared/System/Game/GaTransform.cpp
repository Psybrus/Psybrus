/**************************************************************************
*
* File:		GaTransform.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Game transform.
*		
*
*
* 
**************************************************************************/

#include "Game/GaTransform.h"

//////////////////////////////////////////////////////////////////////////
// Statics
GaTransform	GaTransform::TempTransform_[ 16 ];
BcU32		GaTransform::TempTransformIdx_ = 0;
BcMat4d		GaTransform::TempMatrix_[ 16 ];
BcU32		GaTransform::TempMatrixIdx_ = 0;
