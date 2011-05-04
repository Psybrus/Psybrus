/**************************************************************************
*
* File:		GaLibraryMath.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaLibraryMath.h"
#include "GaLibrary.h"

#include "GaCore.h"

//////////////////////////////////////////////////////////////////////////
// Maths
#include "BcMath.h"
#include "BcVectors.h"
#include "BcMat4d.h"
#include "BcAABB.h"
#include "BcPlane.h"
#include "BcQuat.h"

//////////////////////////////////////////////////////////////////////////
// GaVec2
gmType GaVec2::GM_TYPE = GM_NULL;
gmFunctionEntry GaVec2::GM_LIB[] = 
{
	{ "Vec2",						GaVec2::Create },
};

gmFunctionEntry GaVec2::GM_TYPELIB[] = 
{
	{ "Magnitude",					GaVec2::Magnitude },
	{ "MagnitudeSquared",			GaVec2::MagnitudeSquared },
	{ "Normal",						GaVec2::Normal },
	{ "Normalise",					GaVec2::Normalise },
	{ "Dot",						GaVec2::Dot },
	{ "Lerp",						GaVec2::Lerp }
};


bool GM_CDECL GaVec2::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	a_workDone++;	
	return true;
}

void GM_CDECL GaVec2::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	BcVec2d* pObj = (BcVec2d*)a_object->m_user;
	a_machine->AdjustKnownMemoryUsed( -sizeof( BcVec2d ) );
	delete pObj;
}

BcVec2d* GM_CDECL GaVec2::Alloc( gmMachine* a_machine, BcReal X, BcReal Y )
{
	// TODO: Allocate from a pool.
	BcVec2d* pObj = new BcVec2d( X, Y );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec2d ) );
	return pObj;
}

BcVec2d* GM_CDECL GaVec2::Alloc( gmMachine* a_machine, const BcVec2d& Vector )
{
	// TODO: Allocate from a pool.
	BcVec2d* pObj = new BcVec2d( Vector );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec2d ) );
	return pObj;
}

void GM_CDECL GaVec2::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	BcVec2d* pObj = (BcVec2d*)a_object->m_user;
	
	BcSPrintf( a_buffer, "<Vec2 Object (%f, %f) @ %p>", pObj->x(), pObj->y(), pObj );
}

int GM_CDECL GaVec2::Create( gmThread* a_thread )
{
	int NoofParams = a_thread->GetNumParams();
	gmMachine* a_machine = a_thread->GetMachine();
	
	if( NoofParams == 0 || a_thread->ParamType( 0 ) != GaVec2::GM_TYPE )
	{
		float X = 0.0f;
		float Y = 0.0f;
		switch( NoofParams )
		{
			case 2:
				gmGetFloatOrIntParamAsFloat( a_thread, 1, Y );
			case 1:
				gmGetFloatOrIntParamAsFloat( a_thread, 0, X );
			default:
				break;
		}

		BcVec2d* pObj = Alloc( a_machine, X, Y );	
		a_thread->PushNewUser( pObj, GaVec2::GM_TYPE );
	}
	else
	{
		GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pOtherVec, 0 );
		
		BcVec2d* pObj = Alloc( a_machine, *pOtherVec );
		a_thread->PushNewUser( pObj, GaVec2::GM_TYPE );	
	}
	
	return GM_OK;
}

int GM_CDECL GaVec2::Magnitude( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitude() );
	
	return GM_OK;
}

int GM_CDECL GaVec2::MagnitudeSquared( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitudeSquared() );
	
	return GM_OK;
}

int GM_CDECL GaVec2::Normal( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->normal() ), GaVec2::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec2::Normalise( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	pObj->normalise();
	
	return GM_OK;
}

int GM_CDECL GaVec2::Dot( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pOtherVec, 0 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->dot( *pOtherVec ) );
	
	return GM_OK;
}

int GM_CDECL GaVec2::Lerp( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pVecA, 0 );
	GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pVecB, 1 );
	GM_CHECK_FLOAT_PARAM( Value, 2 );
	BcVec2d* pObj = (BcVec2d*)a_thread->ThisUser_NoChecks();
	
	pObj->lerp( *pVecA, *pVecB, Value );
	
	return GM_OK;
}

void GM_CDECL GaVec2::OpAdd( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec2::GM_TYPE || a_operands[1].m_type != GaVec2::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec2d* pObjA = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec2d* pObjB = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA + *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec2::OpSub( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec2::GM_TYPE || a_operands[1].m_type != GaVec2::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec2d* pObjA = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec2d* pObjB = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA - *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec2::OpMul( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec2::GM_TYPE )
	{
		BcVec2d* pObjA = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec2::GM_TYPE )
		{
			BcVec2d* pObjB = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
		else if ( a_operands[1].m_type == GaMat4::GM_TYPE )
		{
			BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}	

void GM_CDECL GaVec2::OpDiv( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec2::GM_TYPE )
	{
		BcVec2d* pObjA = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec2::GM_TYPE )
		{
			BcVec2d* pObjB = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}

void GM_CDECL GaVec2::OpNeg( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec2::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec2d* pObj = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec2d* pOutObj = Alloc( a_thread->GetMachine(), -*pObj );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec2::OpGetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec2d* pObj = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[1].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			a_operands[0].SetFloat( pObj->x() );
			break;
		case 'y':
			a_operands[0].SetFloat( pObj->y() );
			break;
		default:
			a_operands[0].Nullify();
			break;
	}
}

void GM_CDECL GaVec2::OpSetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec2d* pObj = (BcVec2d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[2].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	// Grab value.
	BcReal Value = 0.0f;
	if( a_operands[1].m_type == GM_FLOAT )
	{
		Value = a_operands[1].m_value.m_float;
	}
	else if( a_operands[1].m_type == GM_INT )
	{
		Value = (BcReal)a_operands[1].m_value.m_int;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			pObj->x( Value );
			break;
		case 'y':
			pObj->y( Value );
			break;
		default:
			break;
	}
}

void GM_CDECL GaVec2::CreateType( gmMachine* a_machine )
{
	a_machine->RegisterLibrary( GaVec2::GM_LIB, 1, "Math", false );
	
	GM_TYPE = a_machine->CreateUserType( "Vec2" );

	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_ADD, NULL, GaVec2::OpAdd );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_SUB, NULL, GaVec2::OpSub );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_MUL, NULL, GaVec2::OpMul );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_DIV, NULL, GaVec2::OpDiv );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_NEG, NULL, GaVec2::OpNeg );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_GETDOT, NULL, GaVec2::OpGetDot );
	a_machine->RegisterTypeOperator( GaVec2::GM_TYPE, O_SETDOT, NULL, GaVec2::OpSetDot );
	
	int NoofEntries = sizeof( GaVec2::GM_TYPELIB ) / sizeof( GaVec2::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaVec2::GM_TYPE, GaVec2::GM_TYPELIB, NoofEntries );
	
	a_machine->RegisterUserCallbacks( GaVec2::GM_TYPE,
									 &GaVec2::Trace,
									 &GaVec2::Destruct,
									 &GaVec2::AsString ); 
}

//////////////////////////////////////////////////////////////////////////
// GaVec3
gmType GaVec3::GM_TYPE = GM_NULL;
gmFunctionEntry GaVec3::GM_LIB[] = 
{
	{ "Vec3",						GaVec3::Create },
};

gmFunctionEntry GaVec3::GM_TYPELIB[] = 
{
	{ "Magnitude",					GaVec3::Magnitude },
	{ "MagnitudeSquared",			GaVec3::MagnitudeSquared },
	{ "Normal",						GaVec3::Normal },
	{ "Normalise",					GaVec3::Normalise },
	{ "Dot",						GaVec3::Dot },
	{ "Cross",						GaVec3::Cross },
	{ "Reflect",					GaVec3::Reflect },
	{ "Lerp",						GaVec3::Lerp }
};


bool GM_CDECL GaVec3::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	a_workDone++;	
	return true;
}

void GM_CDECL GaVec3::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	BcVec3d* pObj = (BcVec3d*)a_object->m_user;
	a_machine->AdjustKnownMemoryUsed( -sizeof( BcVec3d ) );
	delete pObj;
}

BcVec3d* GM_CDECL GaVec3::Alloc( gmMachine* a_machine, BcReal X, BcReal Y, BcReal Z )
{
	// TODO: Allocate from a pool.
	BcVec3d* pObj = new BcVec3d( X, Y, Z );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec3d ) );
	return pObj;
}

BcVec3d* GM_CDECL GaVec3::Alloc( gmMachine* a_machine, const BcVec3d& Vector )
{
	// TODO: Allocate from a pool.
	BcVec3d* pObj = new BcVec3d( Vector );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec3d ) );
	return pObj;
}

void GM_CDECL GaVec3::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	BcVec3d* pObj = (BcVec3d*)a_object->m_user;
	
	BcSPrintf( a_buffer, "<Vec3 Object (%f, %f, %f) @ %p>", pObj->x(), pObj->y(), pObj->z(), pObj );
}

int GM_CDECL GaVec3::Create( gmThread* a_thread )
{
	int NoofParams = a_thread->GetNumParams();
	gmMachine* a_machine = a_thread->GetMachine();
	
	if( NoofParams == 0 || a_thread->ParamType( 0 ) != GaVec3::GM_TYPE )
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
		switch( NoofParams )
		{
			case 3:
				gmGetFloatOrIntParamAsFloat( a_thread, 2, Z );
			case 2:
				gmGetFloatOrIntParamAsFloat( a_thread, 1, Y );
			case 1:
				gmGetFloatOrIntParamAsFloat( a_thread, 0, X );
			default:
				break;
		}
		
		BcVec3d* pObj = Alloc( a_machine, X, Y, Z );
		a_thread->PushNewUser( pObj, GaVec3::GM_TYPE );
	}
	else
	{
		GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pOtherVec, 0 );
		
		BcVec3d* pObj = Alloc( a_machine, *pOtherVec );
		a_thread->PushNewUser( pObj, GaVec3::GM_TYPE );	
	}

	
	return GM_OK;
}

int GM_CDECL GaVec3::Magnitude( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitude() );
	
	return GM_OK;
}

int GM_CDECL GaVec3::MagnitudeSquared( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitudeSquared() );
	
	return GM_OK;
}

int GM_CDECL GaVec3::Normal( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->normal() ), GaVec3::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec3::Normalise( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	pObj->normalise();
	
	return GM_OK;
}

int GM_CDECL GaVec3::Dot( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pOtherVec, 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->dot( *pOtherVec ) );
	
	return GM_OK;
}

int GM_CDECL GaVec3::Cross( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pOtherVec, 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->cross( *pOtherVec ) ), GaVec3::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec3::Reflect( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pOtherVec, 0 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->reflect( *pOtherVec ) ), GaVec3::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec3::Lerp( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pVecA, 0 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pVecB, 1 );
	GM_CHECK_FLOAT_PARAM( Value, 2 );
	BcVec3d* pObj = (BcVec3d*)a_thread->ThisUser_NoChecks();
	
	pObj->lerp( *pVecA, *pVecB, Value );
	
	return GM_OK;
}

void GM_CDECL GaVec3::OpAdd( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec3::GM_TYPE || a_operands[1].m_type != GaVec3::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec3d* pObjA = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec3d* pObjB = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA + *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec3::OpSub( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec3::GM_TYPE || a_operands[1].m_type != GaVec3::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec3d* pObjA = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec3d* pObjB = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA - *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec3::OpMul( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec3::GM_TYPE )
	{
		BcVec3d* pObjA = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec3::GM_TYPE )
		{
			BcVec3d* pObjB = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
		else if ( a_operands[1].m_type == GaMat4::GM_TYPE )
		{
			BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}	

void GM_CDECL GaVec3::OpDiv( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec3::GM_TYPE )
	{
		BcVec3d* pObjA = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec3::GM_TYPE )
		{
			BcVec3d* pObjB = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}

void GM_CDECL GaVec3::OpNeg( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec3::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec3d* pObj = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec3d* pOutObj = Alloc( a_thread->GetMachine(), -*pObj );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec3::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec3::OpGetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec3d* pObj = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[1].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			a_operands[0].SetFloat( pObj->x() );
			break;
		case 'y':
			a_operands[0].SetFloat( pObj->y() );
			break;
		case 'z':
			a_operands[0].SetFloat( pObj->z() );
			break;
		default:
			a_operands[0].Nullify();
			break;
	}
}

void GM_CDECL GaVec3::OpSetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec3d* pObj = (BcVec3d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[2].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	// Grab value.
	BcReal Value = 0.0f;
	if( a_operands[1].m_type == GM_FLOAT )
	{
		Value = a_operands[1].m_value.m_float;
	}
	else if( a_operands[1].m_type == GM_INT )
	{
		Value = (BcReal)a_operands[1].m_value.m_int;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			pObj->x( Value );
			break;
		case 'y':
			pObj->y( Value );
			break;
		case 'z':
			pObj->z( Value );
			break;
		default:
			break;
	}
}

void GM_CDECL GaVec3::CreateType( gmMachine* a_machine )
{
	a_machine->RegisterLibrary( GaVec3::GM_LIB, 1, "Math", false );
	
	GM_TYPE = a_machine->CreateUserType( "Vec3" );
	
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_ADD, NULL, GaVec3::OpAdd );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_SUB, NULL, GaVec3::OpSub );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_MUL, NULL, GaVec3::OpMul );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_DIV, NULL, GaVec3::OpDiv );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_NEG, NULL, GaVec3::OpNeg );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_GETDOT, NULL, GaVec3::OpGetDot );
	a_machine->RegisterTypeOperator( GaVec3::GM_TYPE, O_SETDOT, NULL, GaVec3::OpSetDot );
	
	int NoofEntries = sizeof( GaVec3::GM_TYPELIB ) / sizeof( GaVec3::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaVec3::GM_TYPE, GaVec3::GM_TYPELIB, NoofEntries );
	
	a_machine->RegisterUserCallbacks( GaVec3::GM_TYPE,
									 &GaVec3::Trace,
									 &GaVec3::Destruct,
									 &GaVec3::AsString ); 
}

//////////////////////////////////////////////////////////////////////////
// GaVec4
gmType GaVec4::GM_TYPE = GM_NULL;
gmFunctionEntry GaVec4::GM_LIB[] = 
{
	{ "Vec4",						GaVec4::Create },
};

gmFunctionEntry GaVec4::GM_TYPELIB[] = 
{
	{ "Magnitude",					GaVec4::Magnitude },
	{ "MagnitudeSquared",			GaVec4::MagnitudeSquared },
	{ "Normal",						GaVec4::Normal },
	{ "Normalise",					GaVec4::Normalise },
	{ "Normal3",					GaVec4::Normal3 },
	{ "Normalise3",					GaVec4::Normalise3 },
	{ "Dot",						GaVec4::Dot },
	{ "Lerp",						GaVec4::Lerp }
};


bool GM_CDECL GaVec4::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	a_workDone++;	
	return true;
}

void GM_CDECL GaVec4::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	BcVec4d* pObj = (BcVec4d*)a_object->m_user;
	a_machine->AdjustKnownMemoryUsed( -sizeof( BcVec4d ) );
	delete pObj;
}

BcVec4d* GM_CDECL GaVec4::Alloc( gmMachine* a_machine, BcReal X, BcReal Y, BcReal Z, BcReal W )
{
	// TODO: Allocate from a pool.
	BcVec4d* pObj = new BcVec4d( X, Y, Z, W );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec4d ) );
	return pObj;
}

BcVec4d* GM_CDECL GaVec4::Alloc( gmMachine* a_machine, const BcVec4d& Vector )
{
	// TODO: Allocate from a pool.
	BcVec4d* pObj = new BcVec4d( Vector );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcVec4d ) );
	return pObj;
}

void GM_CDECL GaVec4::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	BcVec4d* pObj = (BcVec4d*)a_object->m_user;
	
	BcSPrintf( a_buffer, "<Vec4 Object (%f, %f, %f, %f) @ %p>", pObj->x(), pObj->y(), pObj->z(), pObj->w(), pObj );
}

int GM_CDECL GaVec4::Create( gmThread* a_thread )
{
	int NoofParams = a_thread->GetNumParams();
	gmMachine* a_machine = a_thread->GetMachine();
	
	if( NoofParams == 0 || a_thread->ParamType( 0 ) != GaVec4::GM_TYPE )
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
		float W = 0.0f;
		switch( NoofParams )
		{
			case 4:
				gmGetFloatOrIntParamAsFloat( a_thread, 3, W );
			case 3:
				gmGetFloatOrIntParamAsFloat( a_thread, 2, Z );
			case 2:
				gmGetFloatOrIntParamAsFloat( a_thread, 1, Y );
			case 1:
				gmGetFloatOrIntParamAsFloat( a_thread, 0, X );
			default:
				break;
		}
		
		// Allocate vector.
		BcVec4d* pObj = Alloc( a_machine, X, Y, Z, W );
		
		a_thread->PushNewUser( pObj, GaVec4::GM_TYPE );
	}
	else
	{
		GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pOtherVec, 0 );
		
		BcVec4d* pObj = Alloc( a_machine, *pOtherVec );
		a_thread->PushNewUser( pObj, GaVec4::GM_TYPE );	
	}

	return GM_OK;
}

int GM_CDECL GaVec4::Magnitude( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitude() );
	
	return GM_OK;
}

int GM_CDECL GaVec4::MagnitudeSquared( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->magnitudeSquared() );
	
	return GM_OK;
}

int GM_CDECL GaVec4::Normal( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->normal() ), GaVec4::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec4::Normalise( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	pObj->normalise();
	
	return GM_OK;
}

int GM_CDECL GaVec4::Normal3( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->normal3() ), GaVec3::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaVec4::Normalise3( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	pObj->normalise3();
	
	return GM_OK;
}

int GM_CDECL GaVec4::Dot( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pOtherVec, 0 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->dot( *pOtherVec ) );
	
	return GM_OK;
}

int GM_CDECL GaVec4::Lerp( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pVecA, 0 );
	GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pVecB, 1 );
	GM_CHECK_FLOAT_PARAM( Value, 2 );
	BcVec4d* pObj = (BcVec4d*)a_thread->ThisUser_NoChecks();
	
	pObj->lerp( *pVecA, *pVecB, Value );
	
	return GM_OK;
}

void GM_CDECL GaVec4::OpAdd( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec4::GM_TYPE || a_operands[1].m_type != GaVec4::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec4d* pObjA = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec4d* pObjB = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA + *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec4::OpSub( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec4::GM_TYPE || a_operands[1].m_type != GaVec4::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec4d* pObjA = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec4d* pObjB = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA - *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec4::OpMul( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec4::GM_TYPE )
	{
		BcVec4d* pObjA = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec4::GM_TYPE )
		{
			BcVec4d* pObjB = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
		else if ( a_operands[1].m_type == GaMat4::GM_TYPE )
		{
			BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec2::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );				
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}	

void GM_CDECL GaVec4::OpDiv( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaVec4::GM_TYPE )
	{
		BcVec4d* pObjA = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		
		if( a_operands[1].m_type == GaVec4::GM_TYPE )
		{
			BcVec4d* pObjB = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / *pObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );
			
			return;
		}
		else if ( a_operands[1].m_type == GM_FLOAT )
		{
			BcReal ObjB = a_operands[1].m_value.m_float;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
		else if ( a_operands[1].m_type == GM_INT )
		{
			BcReal ObjB = (BcReal)a_operands[1].m_value.m_int;	
			
			BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
			gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
			a_operands[0].SetUser( pOutUserObj );	
			
			return;
		}
	}
	
	a_operands[0].Nullify();
}

void GM_CDECL GaVec4::OpNeg( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaVec4::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec4d* pObj = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcVec4d* pOutObj = Alloc( a_thread->GetMachine(), -*pObj );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaVec4::OpGetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec4d* pObj = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[1].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			a_operands[0].SetFloat( pObj->x() );
			break;
		case 'y':
			a_operands[0].SetFloat( pObj->y() );
			break;
		case 'z':
			a_operands[0].SetFloat( pObj->z() );
			break;
		case 'w':
			a_operands[0].SetFloat( pObj->w() );
			break;
		default:
			a_operands[0].Nullify();
			break;
	}
}

void GM_CDECL GaVec4::OpSetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcVec4d* pObj = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[2].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	// Grab value.
	BcReal Value = 0.0f;
	if( a_operands[1].m_type == GM_FLOAT )
	{
		Value = a_operands[1].m_value.m_float;
	}
	else if( a_operands[1].m_type == GM_INT )
	{
		Value = (BcReal)a_operands[1].m_value.m_int;
	}
	
	switch( pStr[0] )
	{
		case 'x':
			pObj->x( Value );
			break;
		case 'y':
			pObj->y( Value );
			break;
		case 'z':
			pObj->z( Value );
			break;
		case 'w':
			pObj->w( Value );
			break;
		default:
			break;
	}
}

void GM_CDECL GaVec4::CreateType( gmMachine* a_machine )
{
	a_machine->RegisterLibrary( GaVec4::GM_LIB, 1, "Math", false );
	
	GM_TYPE = a_machine->CreateUserType( "Vec4" );
	
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_ADD, NULL, GaVec4::OpAdd );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_SUB, NULL, GaVec4::OpSub );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_MUL, NULL, GaVec4::OpMul );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_DIV, NULL, GaVec4::OpDiv );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_NEG, NULL, GaVec4::OpNeg );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_GETDOT, NULL, GaVec4::OpGetDot );
	a_machine->RegisterTypeOperator( GaVec4::GM_TYPE, O_SETDOT, NULL, GaVec4::OpSetDot );
	
	int NoofEntries = sizeof( GaVec4::GM_TYPELIB ) / sizeof( GaVec4::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaVec4::GM_TYPE, GaVec4::GM_TYPELIB, NoofEntries );
	
	a_machine->RegisterUserCallbacks( GaVec4::GM_TYPE,
									 &GaVec4::Trace,
									 &GaVec4::Destruct,
									 &GaVec4::AsString ); 
}


//////////////////////////////////////////////////////////////////////////
// GaMat4
gmType GaMat4::GM_TYPE = GM_NULL;
gmFunctionEntry GaMat4::GM_LIB[] = 
{
	{ "Mat4",						GaMat4::Create },
};

gmFunctionEntry GaMat4::GM_TYPELIB[] = 
{
	{ "Identity",					GaMat4::Identity },
	{ "Transposed",					GaMat4::Transposed },
	{ "Transpose",					GaMat4::Transpose },
	{ "Rotation",					GaMat4::Rotation },
	{ "Translation",				GaMat4::Translation },
	{ "Scale",						GaMat4::Scale },
	{ "Determinant",				GaMat4::Determinant },
	{ "Inverse",					GaMat4::Inverse },
	{ "LookAt",						GaMat4::LookAt },
	{ "OrthoProjection",			GaMat4::OrthoProjection },
	{ "PerspProjection",			GaMat4::PerspProjection },
	{ "Frustum",					GaMat4::Frustum },
};

bool GM_CDECL GaMat4::Trace( gmMachine* a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workRemaining, int& a_workDone )
{
	a_workDone++;	
	return true;
}

void GM_CDECL GaMat4::Destruct( gmMachine* a_machine, gmUserObject* a_object )
{
	BcMat4d* pObj = (BcMat4d*)a_object->m_user;
	a_machine->AdjustKnownMemoryUsed( -sizeof( BcMat4d ) );
	delete pObj;
}

BcMat4d* GM_CDECL GaMat4::Alloc( gmMachine* a_machine )
{
	// TODO: Allocate from a pool.
	BcMat4d* pObj = new BcMat4d();
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcMat4d ) );
	return pObj;
}

BcMat4d* GM_CDECL GaMat4::Alloc( gmMachine* a_machine, const BcMat4d& Matrix )
{
	// TODO: Allocate from a pool.
	BcMat4d* pObj = new BcMat4d( Matrix );		
	a_machine->AdjustKnownMemoryUsed( +sizeof( BcMat4d ) );
	return pObj;
}

void GM_CDECL GaMat4::AsString( gmUserObject* a_object, char* a_buffer, int a_bufferLen )
{
	BcMat4d* pObj = (BcMat4d*)a_object->m_user;
	
	BcSPrintf( a_buffer, "<Mat4 Object @ %p>", pObj );
}

int GM_CDECL GaMat4::Create( gmThread* a_thread )
{
	int NoofParams = a_thread->GetNumParams();
	gmMachine* a_machine = a_thread->GetMachine();

	if( NoofParams == 0 || a_thread->ParamType( 0 ) != GaMat4::GM_TYPE )
	{
		BcMat4d* pObj = Alloc( a_machine );
		a_thread->PushNewUser( pObj, GaMat4::GM_TYPE );
	}
	else
	{
		GM_CHECK_USER_PARAM( BcMat4d*, GaMat4::GM_TYPE, pOtherMat, 0 );
		
		BcMat4d* pObj = Alloc( a_machine, *pOtherMat );
		a_thread->PushNewUser( pObj, GaMat4::GM_TYPE );	
	}
	
	return GM_OK;
}

int GM_CDECL GaMat4::Identity( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();

	pObj->identity();	
	
	return GM_OK;
}

int GM_CDECL GaMat4::Transposed( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushNewUser( Alloc( a_thread->GetMachine(), pObj->transposed() ), GaMat4::GM_TYPE );
	
	return GM_OK;
}

int GM_CDECL GaMat4::Transpose( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();

	pObj->transpose();
	
	return GM_OK;
}

int GM_CDECL GaMat4::Rotation( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pVec, 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->rotation( *pVec );
	
	return GM_OK;
}

int GM_CDECL GaMat4::Translation( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	
	if( a_thread->ParamType( 0 ) == GaVec2::GM_TYPE )
	{
		GM_CHECK_USER_PARAM( BcVec2d*, GaVec2::GM_TYPE, pVec, 0 );
		BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
		pObj->translation( BcVec3d( pVec->x(), pVec->y(), 0.0f ) );
	}
	else if( a_thread->ParamType( 0 ) == GaVec3::GM_TYPE )
	{
		GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pVec, 0 );
		BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
		pObj->translation( *pVec );
	}
	else if( a_thread->ParamType( 0 ) == GaVec4::GM_TYPE )
	{
		GM_CHECK_USER_PARAM( BcVec4d*, GaVec4::GM_TYPE, pVec, 0 );
		BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
		pObj->translation( *pVec );
	}
		
	return GM_OK;
}

int GM_CDECL GaMat4::Scale( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pVec, 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->scale( *pVec );

	return GM_OK;
}

int GM_CDECL GaMat4::Determinant( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	a_thread->PushFloat( pObj->determinant() );
	
	return GM_OK;
}

int GM_CDECL GaMat4::Inverse( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 0 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();

	pObj->inverse();
	
	return GM_OK;
}

int GM_CDECL GaMat4::LookAt( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 3 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pPosition, 0 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pLookAt, 1 );
	GM_CHECK_USER_PARAM( BcVec3d*, GaVec3::GM_TYPE, pUpVec, 2 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->lookAt( *pPosition, *pLookAt, *pUpVec );

	return GM_OK;
}

int GM_CDECL GaMat4::OrthoProjection( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 6 );
	GM_CHECK_FLOAT_PARAM( Left, 0 );
	GM_CHECK_FLOAT_PARAM( Right, 1 );
	GM_CHECK_FLOAT_PARAM( Top, 2 );
	GM_CHECK_FLOAT_PARAM( Bottom, 3 );
	GM_CHECK_FLOAT_PARAM( Near, 4 );
	GM_CHECK_FLOAT_PARAM( Far, 5 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->orthoProjection( Left, Right, Top, Bottom, Near, Far );
	
	return GM_OK;
}

int GM_CDECL GaMat4::PerspProjection( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 4 );
	GM_CHECK_FLOAT_PARAM( Fov, 0 );
	GM_CHECK_FLOAT_PARAM( Aspect, 1 );
	GM_CHECK_FLOAT_PARAM( Near, 2 );
	GM_CHECK_FLOAT_PARAM( Far, 3 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->perspProjection( Fov, Aspect, Near, Far );
	
	return GM_OK;
}

int GM_CDECL GaMat4::Frustum( gmThread* a_thread )
{
	GM_CHECK_NUM_PARAMS( 6 );
	GM_CHECK_FLOAT_PARAM( Left, 0 );
	GM_CHECK_FLOAT_PARAM( Right, 1 );
	GM_CHECK_FLOAT_PARAM( Top, 2 );
	GM_CHECK_FLOAT_PARAM( Bottom, 3 );
	GM_CHECK_FLOAT_PARAM( Near, 4 );
	GM_CHECK_FLOAT_PARAM( Far, 5 );
	BcMat4d* pObj = (BcMat4d*)a_thread->ThisUser_NoChecks();
	
	pObj->frustum( Left, Right, Top, Bottom, Near, Far );
	
	return GM_OK;
}

void GM_CDECL GaMat4::OpAdd( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaMat4::GM_TYPE || a_operands[1].m_type != GaMat4::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcMat4d* pObjA = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcMat4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA + *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaMat4::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaMat4::OpSub( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type != GaMat4::GM_TYPE || a_operands[1].m_type != GaMat4::GM_TYPE )
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcMat4d* pObjA = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
	
	BcMat4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA - *pObjB );
	gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaMat4::GM_TYPE );
	a_operands[0].SetUser( pOutUserObj );
}

void GM_CDECL GaMat4::OpMul( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaMat4::GM_TYPE && a_operands[1].m_type == GaMat4::GM_TYPE )
	{
		BcMat4d* pObjA = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		BcMat4d* pObjB = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;	
		
		BcMat4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * *pObjB );
		gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaMat4::GM_TYPE );
		a_operands[0].SetUser( pOutUserObj );
		
		return;
	}
	else if( a_operands[0].m_type == GaMat4::GM_TYPE && a_operands[1].m_type == GM_FLOAT )
	{
		BcMat4d* pObjA = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		BcReal ObjB = a_operands[1].m_value.m_float;	
		
		BcMat4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA * ObjB );
		gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaMat4::GM_TYPE );
		a_operands[0].SetUser( pOutUserObj );

		return;
	}
	
	a_operands[0].Nullify();
}

void GM_CDECL GaMat4::OpDiv( gmThread* a_thread, gmVariable* a_operands )
{
	if( a_operands[0].m_type == GaMat4::GM_TYPE && a_operands[1].m_type == GM_FLOAT )
	{
		BcMat4d* pObjA = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
		BcReal ObjB = a_operands[1].m_value.m_float;	
		
		BcMat4d* pOutObj = Alloc( a_thread->GetMachine(), *pObjA / ObjB );
		gmUserObject* pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaMat4::GM_TYPE );
		a_operands[0].SetUser( pOutUserObj );
		
		return;
	}
	
	a_operands[0].Nullify();
}

void GM_CDECL GaMat4::OpGetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcMat4d* pObj = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[1].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 2)
	{
		a_operands[0].Nullify();
		return;
	}
	
	BcVec4d* pOutObj = NULL;
	gmUserObject* pOutUserObj = NULL;
	if( pStr[0] == 'r' )
	{
		switch( pStr[1] )
		{
			case '0':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->row0() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '1':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->row1() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '2':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->row2() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '3':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->row3() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			default:
				a_operands[0].Nullify();
				break;
		}
	}
	else if( pStr[0] == 'c' )
	{
		switch( pStr[1] )
		{
			case '0':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->col0() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '1':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->col1() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '2':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->col2() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			case '3':
				pOutObj = GaVec4::Alloc( a_thread->GetMachine(), pObj->col3() );
				pOutUserObj = a_thread->GetMachine()->AllocUserObject( pOutObj, GaVec4::GM_TYPE );
				a_operands[0].SetUser( pOutUserObj );
				break;
			default:
				a_operands[0].Nullify();
				break;
		}
	}
}

void GM_CDECL GaMat4::OpSetDot( gmThread* a_thread, gmVariable* a_operands )
{
	BcMat4d* pObj = (BcMat4d*) ((gmUserObject*)GM_OBJECT(a_operands[0].m_value.m_ref))->m_user;
	
	gmStringObject* pStrObj = (gmStringObject*)GM_OBJECT(a_operands[2].m_value.m_ref);
	const char* pStr = pStrObj->GetString();
	if(pStrObj->GetLength() != 1)
	{
		a_operands[0].Nullify();
		return;
	}
	
	// Grab value.
	BcReal Value = 0.0f;
	if( a_operands[1].m_type == GaVec4::GM_TYPE )
	{
		BcVec4d* pVec = (BcVec4d*) ((gmUserObject*)GM_OBJECT(a_operands[1].m_value.m_ref))->m_user;
		if( pStr[0] == 'r' )
		{
			switch( pStr[1] )
			{
				case 0:
					pObj->row0( *pVec );
					break;
				case 1:
					pObj->row1( *pVec );
					break;
				case 2:
					pObj->row2( *pVec );
					break;
				case 3:
					pObj->row3( *pVec );
					break;
			}
		}
		else if( pStr[0] == 'c' )
		{
			switch( pStr[1] )
			{
				case 0:
					pObj->col0( *pVec );
					break;
				case 1:
					pObj->col1( *pVec );
					break;
				case 2:
					pObj->col2( *pVec );
					break;
				case 3:
					pObj->col3( *pVec );
					break;
			}
		}
	}
}

void GM_CDECL GaMat4::CreateType( gmMachine* a_machine )
{
	a_machine->RegisterLibrary( GaMat4::GM_LIB, 1, "Math", false );
	
	GM_TYPE = a_machine->CreateUserType( "Mat4" );
	
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_ADD, NULL, GaMat4::OpAdd );
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_SUB, NULL, GaMat4::OpSub );
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_MUL, NULL, GaMat4::OpMul );
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_DIV, NULL, GaMat4::OpDiv );
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_GETDOT, NULL, GaMat4::OpGetDot );
	a_machine->RegisterTypeOperator( GaMat4::GM_TYPE, O_SETDOT, NULL, GaMat4::OpSetDot );
	
	int NoofEntries = sizeof( GaMat4::GM_TYPELIB ) / sizeof( GaMat4::GM_TYPELIB[0] );
	a_machine->RegisterTypeLibrary( GaMat4::GM_TYPE, GaMat4::GM_TYPELIB, NoofEntries );
	
	a_machine->RegisterUserCallbacks( GaMat4::GM_TYPE,
									 &GaMat4::Trace,
									 &GaMat4::Destruct,
									 &GaMat4::AsString ); 
}

//////////////////////////////////////////////////////////////////////////
// GaLibraryMathBinder
void GaLibraryMathBinder( class gmMachine* pGmMachine )
{
	// Create vector types.
	GaVec2::CreateType( pGmMachine );
	GaVec3::CreateType( pGmMachine );
	GaVec4::CreateType( pGmMachine );
	
	// Create matrix types.
	GaMat4::CreateType( pGmMachine );
}

