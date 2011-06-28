#=============================================
# Functor Generator
# Author: Neil Richardson
#=============================================

#=============================================
# Generate a seperated string based on a template.
# template could be "typename _P%NUM%" or something similar.
def genSeperatedString( range, template, seperator ):
	outString = ""
	for each in range:
		if each != 0:
			outString = outString + seperator
		outString = outString + template.replace( "%NUM%", str( each ) )
	return outString

#=============================================
# Generate a BcFuncTraits class
def BcFuncTraits_header( outFile ):
	outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
	outFile.write( "// BcFuncTraits\n\n" )
	outFile.write( "template< typename _Fn >\n" )
	outFile.write( "struct BcFuncTraits;\n\n" )

def BcFuncTraits_gen( outFile, numParams, isClass ):
	outString = ""
	if isClass == 0:
		templateList = genSeperatedString( range( 0, numParams ), ", typename _P%NUM%", "" )
		paramList = genSeperatedString( range( 0, numParams ), "_P%NUM%", ", " )
		typedefList = genSeperatedString( range( 0, numParams ), "\ttypedef _P%NUM% param%NUM%_type;\n", "" )
		outString += "template< typename _R" + templateList + " >\n"
		outString += "struct BcFuncTraits< _R(*)(" + paramList + ") >\n"
		outString += "{\n"
		outString += "	static const int PARAMS = " + str( numParams ) + ";\n"
		outString += "	typedef _R return_type;\n"
		outString += typedefList
		outString += "	typedef _R(*signature_type)(" + paramList + ");\n"
		outString += "};\n\n"
	else:
		templateList = genSeperatedString( range( 0, numParams ), ", typename _P%NUM%", "" )
		paramList = genSeperatedString( range( 0, numParams ), "_P%NUM%", ", " )
		typedefList = genSeperatedString( range( 0, numParams ), "\ttypedef _P%NUM% param%NUM%_type;\n", "" )
		outString += "template< typename _Ty, typename _R" + templateList + " >\n"
		outString += "struct BcFuncTraits< _R(_Ty::*)(" + paramList + ") >\n"
		outString += "{\n"
		outString += "	static const int PARAMS = " + str( numParams ) + ";\n"
		outString += "	typedef _Ty class_type;\n"
		outString += "	typedef _R return_type;\n"
		outString += typedefList
		outString += "	typedef _R(*signature_type)(" + paramList + ");\n"
		outString += "};\n\n"
	outFile.write( outString )
	
#=============================================
# Generate the functor.
def BcDelegate_header( outFile ):
	outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
	outFile.write( "// _BcDelegateInternal\n\n" )
	outFile.write( "template< typename _Fn, int >\n" )
	outFile.write( "class _BcDelegateInternal;\n\n" )

	outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
	outFile.write( "// BcDelegateCallBase\n\n" )
	outFile.write( "class BcDelegateCallBase\n" )
	outFile.write( "{\n" )
	outFile.write( "public:\n" )
	outFile.write( "	BcDelegateCallBase():\n" )
	outFile.write( "		HasBeenCalled_( BcTrue )\n" )
	outFile.write( "	{\n\n" )
	outFile.write( "	}\n\n" )
	outFile.write( "	virtual ~BcDelegateCallBase(){}\n" )
	outFile.write( "	virtual void operator()() = 0;\n\n" )
	outFile.write( "	BcBool hasBeenCalled() const\n" )
	outFile.write( "	{\n" )
	outFile.write( "		return HasBeenCalled_;\n" )
	outFile.write( "	}\n\n" )
	outFile.write( "	void resetHasBeenCalled()\n" )
	outFile.write( "	{\n" )
	outFile.write( "		HasBeenCalled_ = BcFalse;\n" )
	outFile.write( "	}\n" )
	outFile.write( "	protected:\n" )
	outFile.write( "		BcBool HasBeenCalled_;\n" )
	outFile.write( "	};\n" )

def BcDelegateCall_header( outFile ):
	outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
	outFile.write( "// _BcDelegateCallInternal\n\n" )
	outFile.write( "template< typename _Fn, int >\n" );
	outFile.write( "class _BcDelegateCallInternal;\n" );


def BcDelegate_gen( outFile, numParams ):
	outString = ""
	typedefList = genSeperatedString( range( 0, numParams ), "	typedef typename BcFuncTraits< _Fn >::param%NUM%_type param%NUM%_type;\n", "" )
	paramList = genSeperatedString( range( 0, numParams ), "typename BcFuncTraits< _Fn >::param%NUM%_type P%NUM%", ", " )
	paramTypeList = genSeperatedString( range( 0, numParams ), "typename BcFuncTraits< _Fn >::param%NUM%_type", ", " )
	paramCallList = genSeperatedString( range( 0, numParams ), "P%NUM%", ", " )

	outString += "template< typename _Fn >\n"
	outString += "class _BcDelegateInternal< _Fn, " + str( numParams ) + " >\n"
	outString += "{\n"
	outString += "public:\n"
	outString += "	typedef typename BcFuncTraits< _Fn >::return_type return_type;\n"
	outString += typedefList
	if numParams == 0:
		outString += "	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*);\n"
	else:
		outString += "	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,"+paramTypeList+");\n"
	outString += "public:\n"
	
	outString += "	BcForceInline _BcDelegateInternal(): pThis_( NULL ), stubFunc_( NULL ){};\n"
	
	outString += "	BcForceInline return_type operator()(" + paramList + ")\n"
	outString += "	{\n"
	outString += "		BcAssert( stubFunc_ != NULL );\n"
	if numParams == 0:
		outString += "		return (*stubFunc_)(pThis_);\n"
	else:
		outString += "		return (*stubFunc_)(pThis_, " + paramCallList + ");\n"
	outString += "	}\n\n"
	
	outString += "	BcForceInline BcBool isValid() const\n"
	outString += "	{\n"
	outString += "		return ( stubFunc_ != NULL );\n"
	outString += "	}\n\n"
	
	outString += "	template< _Fn _func >\n"
	outString += "	static _BcDelegateInternal< _Fn, " + str( numParams ) + " > bind()\n"
	outString += "	{\n"
	outString += "		_BcDelegateInternal< _Fn, " + str( numParams ) + " > Func;\n"
	outString += "		Func.pThis_ = NULL;\n"
	outString += "		Func.stubFunc_ = &global_stub< _func >;\n"
	outString += "		return Func;\n"
	outString += "	}\n\n"
	
	outString += "	template< class _Ty, return_type(_Ty::*_func)("+paramTypeList+") >\n"
	outString += "	static _BcDelegateInternal< _Fn, " + str( numParams ) + " > bind( _Ty* pThis )\n"
	outString += "	{\n"
	outString += "		_BcDelegateInternal< _Fn, " + str( numParams ) + " > Func;\n"
	outString += "		Func.pThis_ = pThis;\n"
	outString += "		Func.stubFunc_ = &method_stub< _Ty, _func >;\n"
	outString += "		BcAssert( pThis != NULL );\n"
	outString += "		return Func;\n"
	outString += "	}\n\n"

	outString += "private:\n"
	outString += "	template< _Fn func >\n"
	if numParams == 0:
		outString += "	static return_type global_stub( void* )\n"
	else:
		outString += "	static return_type global_stub( void*, " + paramList + " )\n"
	outString += "	{\n"
	outString += "		return (*func)( " + paramCallList + " );\n"
	outString += "	}\n\n"

	outString += "	template< class _Ty, return_type (_Ty::*meth)("+paramTypeList+") >\n"
	if numParams == 0:
		outString += "	static return_type method_stub( void* pObj )\n"
	else:
		outString += "	static return_type method_stub( void* pObj, " + paramList + " )\n"
	outString += "	{\n"
	outString += "		_Ty* pThis = static_cast< _Ty* >( pObj );\n"
	outString += "		return (pThis->*meth)( " + paramCallList + " );\n"
	outString += "	}\n\n"
	outString += "private:\n"
	outString += "	void* pThis_;\n"
	outString += "	stub_func stubFunc_;\n"
	outString += "};\n\n"
	outFile.write( outString )

def BcDelegateCall_gen( outFile, numParams ):
	outString = ""
	typedefList = genSeperatedString( range( 0, numParams ), "	typedef typename BcFuncTraits< _Fn >::param%NUM%_type param%NUM%_type;\n", "" )
	paramList = genSeperatedString( range( 0, numParams ), "typename BcFuncTraits< _Fn >::param%NUM%_type P%NUM%", ", " )
	paramTypeList = genSeperatedString( range( 0, numParams ), "typename BcFuncTraits< _Fn >::param%NUM%_type", ", " )
	paramCallList = genSeperatedString( range( 0, numParams ), "P%NUM%_", ", " )
	paramMemberList = genSeperatedString( range( 0, numParams ), "	typename BcFuncTraits< _Fn >::param%NUM%_type P%NUM%_;\n", "" )
	paramAssignList = genSeperatedString( range( 0, numParams ), "		P%NUM%_ = P%NUM%;\n", "" )

	outString += "template< typename _Fn >\n"
	outString += "class _BcDelegateCallInternal< _Fn, " + str( numParams ) + " >:\n"
	outString += "	public BcDelegateCallBase\n"
	outString += "{\n"
	outString += "public:\n"
	outString += "	typedef typename BcFuncTraits< _Fn >::return_type return_type;\n"
	outString += typedefList
	if numParams == 0:
		outString += "	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*);\n"
	else:
		outString += "	typedef typename BcFuncTraits< _Fn >::return_type(*stub_func)(void*,"+paramTypeList+");\n"
	outString += "public:\n"
	outString += "	_BcDelegateCallInternal(){}\n"
	outString += "	_BcDelegateCallInternal( const BcDelegate< _Fn >& Delegate ):\n"
	outString += "		Delegate_( Delegate )\n"
	outString += "	{\n"
	outString += "	}\n"
	outString += "	virtual ~_BcDelegateCallInternal(){}\n"
	outString += "	virtual void operator()()\n"
	outString += "	{\n"
	outString += "		Delegate_( " + paramCallList + " );\n"
	outString += "		HasBeenCalled_ = BcTrue;\n"
	outString += "	}\n"
	outString += "	virtual _BcDelegateCallInternal< _Fn, " + str( numParams ) + " >& deferCall( "+ paramList +" )\n"
	outString += "	{\n"
	outString += paramAssignList
	outString += "		return (*this);\n"
	outString += "	}\n"
	outString += "private:\n"
	outString += paramMemberList
	outString += "	BcDelegate< _Fn > Delegate_;\n"
	outString += "};\n"

	outFile.write( outString )
	
def BcDelegate_footer( outFile ):
	outString = """
//////////////////////////////////////////////////////////////////////////
// BcDelegate
template< typename _Fn >
class BcDelegate: public _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >
{
public:
	BcDelegate()
	{
		
	}
		
	BcDelegate( const _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}

	void operator = ( const _BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
};
"""
	outFile.write( outString )


def BcDelegateCall_footer( outFile ):
	outString = """
//////////////////////////////////////////////////////////////////////////
// BcDelegateCall
template< typename _Fn >
class BcDelegateCall: public _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >
{
public:
	BcDelegateCall()
	{
	}
	
	BcDelegateCall( const BcDelegate< _Fn >& Delegate ):
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >( Delegate )
	{
	}
	
	BcDelegateCall( _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
	
	void operator = ( _BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >& Other )
	{
		_BcDelegateCallInternal< _Fn, BcFuncTraits< _Fn >::PARAMS >* pBaseSelf = this;
		*pBaseSelf = Other;
	}
	
	virtual ~BcDelegateCall(){}
};
"""
	outFile.write( outString )

#=============================================
# Generate the functor file.
outFile = open( "BcDelegate.h", "w+" )

outString = """/**************************************************************************
*
* File:		BcDelegate.h
* Author:	Neil Richardson & Autogeneration Script
* Ver/Date:
* Description:
*		Function object.
*
*
*
**************************************************************************/

#ifndef __BCDELEGATE_H__
#define __BCDELEGATE_H__

#include "BcTypes.h"
#include "BcDebug.h"

"""

outFile.write( outString )

numParams = 9

BcFuncTraits_header( outFile )
for each in range( 0, numParams ):
	BcFuncTraits_gen( outFile, each, 0 )
for each in range( 0, numParams ):
	BcFuncTraits_gen( outFile, each, 1 )

BcDelegate_header( outFile )
for each in range( 0, numParams ):
	BcDelegate_gen( outFile, each )

BcDelegate_footer( outFile )

BcDelegateCall_header( outFile )
for each in range( 0, numParams ):
	BcDelegateCall_gen( outFile, each )

BcDelegateCall_footer( outFile )

outString = ""
outString += "#endif\n"
outFile.write( outString )
