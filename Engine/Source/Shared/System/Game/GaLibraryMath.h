/**************************************************************************
*
* File:		GaLibraryMath.h
* Author:	Neil Richardson 
* Ver/Date:	25/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaLibraryMath_H__
#define __GaLibraryMath_H__

#include "GaLibrary.h"

//////////////////////////////////////////////////////////////////////////
// GaVec2
class GaVec2
{
public:
	static gmType GM_TYPE;
	static gmFunctionEntry GM_LIB[];
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static bool GM_CDECL Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone );
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object );
	static BcVec2d* GM_CDECL Alloc( gmMachine* a_machine, BcReal X, BcReal Y );
	static BcVec2d* GM_CDECL Alloc( gmMachine* a_machine, const BcVec2d& Vector );
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen );
	
	static int GM_CDECL Create( gmThread* a_thread );
	
	static int GM_CDECL Magnitude( gmThread* a_thread );
	static int GM_CDECL MagnitudeSquared( gmThread* a_thread );	
	static int GM_CDECL Normal( gmThread* a_thread );
	static int GM_CDECL Normalise( gmThread* a_thread );
	static int GM_CDECL Dot( gmThread* a_thread );
	static int GM_CDECL Lerp( gmThread* a_thread );
	
	static void GM_CDECL OpAdd( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSub( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpMul( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpDiv( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpNeg( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpGetDot( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSetDot( gmThread* a_thread, gmVariable* a_operands );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaVec3
class GaVec3
{
public:
	static gmType GM_TYPE;
	static gmFunctionEntry GM_LIB[];
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static bool GM_CDECL Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone );
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object );
	static BcVec3d* GM_CDECL Alloc( gmMachine* a_machine, BcReal X, BcReal Y, BcReal Z );
	static BcVec3d* GM_CDECL Alloc( gmMachine* a_machine, const BcVec3d& Vector );
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen );

	static int GM_CDECL Create( gmThread* a_thread );
	
	static int GM_CDECL Magnitude( gmThread* a_thread );
	static int GM_CDECL MagnitudeSquared( gmThread* a_thread );	
	static int GM_CDECL Normal( gmThread* a_thread );
	static int GM_CDECL Normalise( gmThread* a_thread );
	static int GM_CDECL Dot( gmThread* a_thread );
	static int GM_CDECL Cross( gmThread* a_thread );
	static int GM_CDECL Reflect( gmThread* a_thread );
	static int GM_CDECL Lerp( gmThread* a_thread );
	
	static void GM_CDECL OpAdd( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSub( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpMul( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpDiv( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpNeg( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpGetDot( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSetDot( gmThread* a_thread, gmVariable* a_operands );

	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaVec4
class GaVec4
{
public:
	static gmType GM_TYPE;
	static gmFunctionEntry GM_LIB[];
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static bool GM_CDECL Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone );
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object );
	static BcVec4d* GM_CDECL Alloc( gmMachine* a_machine, BcReal X, BcReal Y, BcReal Z, BcReal W );
	static BcVec4d* GM_CDECL Alloc( gmMachine* a_machine, const BcVec4d& Vector );
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen );
	
	static int GM_CDECL Create( gmThread* a_thread );
	
	static int GM_CDECL Magnitude( gmThread* a_thread );
	static int GM_CDECL MagnitudeSquared( gmThread* a_thread );	
	static int GM_CDECL Normal( gmThread* a_thread );
	static int GM_CDECL Normalise( gmThread* a_thread );
	static int GM_CDECL Normal3( gmThread* a_thread );
	static int GM_CDECL Normalise3( gmThread* a_thread );
	static int GM_CDECL Dot( gmThread* a_thread );
	static int GM_CDECL Lerp( gmThread* a_thread );
	
	static void GM_CDECL OpAdd( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSub( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpMul( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpDiv( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpNeg( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpGetDot( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSetDot( gmThread* a_thread, gmVariable* a_operands );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaMat4
class GaMat4
{
public:
	static gmType GM_TYPE;
	static gmFunctionEntry GM_LIB[];
	static gmFunctionEntry GM_TYPELIB[];
	
public:
	static bool GM_CDECL Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone );
	static void GM_CDECL Destruct( gmMachine* a_machine, gmUserObject* a_object );
	static BcMat4d* GM_CDECL Alloc( gmMachine* a_machine );
	static BcMat4d* GM_CDECL Alloc( gmMachine* a_machine, const BcMat4d& Matrix );
	static void GM_CDECL AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen );
	
	static int GM_CDECL Create( gmThread* a_thread );
	
	static int GM_CDECL Identity( gmThread* a_thread );
	static int GM_CDECL Transposed( gmThread* a_thread );	
	static int GM_CDECL Transpose( gmThread* a_thread );
	static int GM_CDECL Rotation( gmThread* a_thread );
	static int GM_CDECL Translation( gmThread* a_thread );
	static int GM_CDECL Scale( gmThread* a_thread );
	static int GM_CDECL Determinant( gmThread* a_thread );
	static int GM_CDECL Inverse( gmThread* a_thread );
	static int GM_CDECL LookAt( gmThread* a_thread );
	static int GM_CDECL OrthoProjection( gmThread* a_thread );
	static int GM_CDECL PerspProjection( gmThread* a_thread );
	static int GM_CDECL Frustum( gmThread* a_thread );
	
	static void GM_CDECL OpAdd( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSub( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpMul( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpDiv( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpGetDot( gmThread* a_thread, gmVariable* a_operands );
	static void GM_CDECL OpSetDot( gmThread* a_thread, gmVariable* a_operands );
	
	static void GM_CDECL CreateType( gmMachine* a_machine );
};

//////////////////////////////////////////////////////////////////////////
// GaLibraryMathBinder
void GaLibraryMathBinder( class gmMachine* pGmMachine );


#endif


