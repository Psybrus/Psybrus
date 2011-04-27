/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#include "gmConfig.h"
#include "gmListLib.h"
#include "gmThread.h"
#include "gmMachine.h"
#include "gmHelpers.h"

//
//
// Implementation of list binding
//
//

#if GM_LIST_LIB

// Statics and globals
gmType GM_LIST = GM_NULL;
gmVariable gmUserList::m_null;


bool gmUserList::Construct(gmMachine * a_machine, int a_initialsize)
{
  m_null.Nullify();
  m_list = NULL;
  m_size = 0;
  m_length = 0;
  return Resize(a_machine, a_initialsize);
}


void gmUserList::Destruct(gmMachine * a_machine)
{
  if(m_list)
  {
    a_machine->Sys_Free(m_list);
    m_list = NULL;
  }
  m_size = 0;
}


bool gmUserList::Resize(gmMachine * a_machine, int a_size)
{
  if(a_size < 0) a_size = 0;
  int copysize = (a_size > m_size) ? m_size : a_size;
  gmVariable * list = (gmVariable *) a_machine->Sys_Alloc(sizeof(gmVariable) * a_size);
  // copy contents.
  if(m_list)
  {
    memcpy(list, m_list, sizeof(gmVariable) * copysize);
    if(a_size > copysize)
    {
      memset(list + copysize, 0, sizeof(gmVariable) * (a_size - copysize));
    }
    a_machine->Sys_Free(m_list);
  }
  else
  {
    memset(list, 0, sizeof(gmVariable) * a_size);
  }
  m_list = list;
  m_size = a_size;
  return true;
}

void gmUserList::Append( gmMachine * a_machine, const gmVariable &a_variable )
{
	// If we're at the end of the list then grow.
	if( m_length >= m_size )
	{
		Resize( a_machine, m_size + GM_LIST_LIB_GROW_BY );
	}

	// Set at the end pos.
	SetAt( m_length, a_variable );

	// Increment length.
	++m_length;
}

void gmUserList::RemoveAt( int a_index )
{
	// Not a pleasant or optimal implementation, however it will work.
	for( int i = a_index; i < m_length - 1; ++i )
	{
		m_list[i] = m_list[i+1];
	}

	--m_length;
}

bool gmUserList::Remove( const gmVariable &a_variable )
{
	bool removedItems = false;
	for( int i = 0; i < m_length;  )
	{
		// If there is a match, remove it.
		if( gmVariable::Compare( a_variable, m_list[ i ] ) == 0 )
		{
			RemoveAt( i );
			removedItems = true;
		}
		else
		{
			++i;
		}
	}
	return removedItems;
}

#if GM_USE_INCGC
bool gmUserList::Trace(gmMachine * a_machine, gmGarbageCollector* a_gc, const int a_workLeftToGo, int& a_workDone)
{
  int i;
  for(i = 0; i < m_length; ++i)
  {
    if(m_list[i].IsReference())
    {
      gmObject * object = GM_MOBJECT(a_machine, m_list[i].m_value.m_ref);
      a_gc->GetNextObject(object);
      ++a_workDone;
    }
  }
  
  ++a_workDone;
  return true;
}
#else //GM_USE_INCGC
void gmUserList::Mark(gmMachine * a_machine, gmuint32 a_mark)
{
  int i;
  for(i = 0; i < m_size; ++i)
  {
    if(m_list[i].IsReference())
    {
      gmObject * object = GM_MOBJECT(a_machine, m_list[i].m_value.m_ref);
      if(object->NeedsMark(a_mark)) object->Mark(a_machine, a_mark);
    }
  }
}
#endif //GM_USE_INCGC

gmUserList* gmUserList_Create(gmMachine* a_machine, int a_size)
{
  gmUserList * newList = (gmUserList *) a_machine->Sys_Alloc(sizeof(gmUserList));
  newList->Construct(a_machine, a_size);
  return newList;
}


// functions

static int GM_CDECL gmfList(gmThread * a_thread) // size
{
  gmUserList * list = (gmUserList *) a_thread->GetMachine()->Sys_Alloc(sizeof(gmUserList));
  list->Construct(a_thread->GetMachine(), GM_LIST_LIB_GROW_BY );
  
  // params are values.
  for( int i = 0; i < a_thread->GetNumParams(); ++i )
  {
	  list->Append( a_thread->GetMachine(), a_thread->Param( i ) );
  }

  a_thread->PushNewUser(list, GM_LIST);
  return GM_OK;
}

static int GM_CDECL gmfListLength(gmThread * a_thread) // return size
{
  gmUserObject * listObject = a_thread->ThisUserObject();
  GM_ASSERT(listObject->m_userType == GM_LIST);
  if(listObject->m_user)
  {
    gmUserList * list = (gmUserList *) listObject->m_user;
    a_thread->PushInt(list->Length());
  }
  return GM_OK;
}

static int GM_CDECL gmfListAppend(gmThread * a_thread) // return size
{
	gmUserObject * listObject = a_thread->ThisUserObject();
	GM_ASSERT(listObject->m_userType == GM_LIST);
	if(listObject->m_user)
	{
		gmUserList * list = (gmUserList *) listObject->m_user;

		for( int i = 0; i < a_thread->GetNumParams(); ++i )
		{
			list->Append( a_thread->GetMachine(), a_thread->Param( i ) );
		}
		
	}
	return GM_OK;
}

static int GM_CDECL gmfListRemoveAt(gmThread * a_thread) // return size
{
	GM_CHECK_NUM_PARAMS( 1 );
	GM_CHECK_INT_PARAM( idx, 0 );
	gmUserObject * listObject = a_thread->ThisUserObject();
	gmUserList * list = (gmUserList *) listObject->m_user;
	GM_ASSERT(listObject->m_userType == GM_LIST);
	if(idx >= list->Length() )
	{	
		return GM_EXCEPTION;
	}

	list->RemoveAt( idx );

	return GM_OK;
}

static int GM_CDECL gmfListRemove(gmThread * a_thread) // return size
{
	gmUserObject * listObject = a_thread->ThisUserObject();
	gmUserList * list = (gmUserList *) listObject->m_user;
	GM_ASSERT(listObject->m_userType == GM_LIST);

	for( int i = 0; i < a_thread->GetNumParams(); ++i ) 
	{
		list->Remove( a_thread->Param( i ) );
	}

	return GM_OK;
}

static void GM_CDECL gmListGetInd(gmThread * a_thread, gmVariable * a_operands)
{
  gmUserObject * listObject = (gmUserObject *) GM_OBJECT(a_operands->m_value.m_ref);
  GM_ASSERT(listObject->m_userType == GM_LIST);
  gmUserList * list = (gmUserList *) listObject->m_user;
  if(a_operands[1].m_type == GM_INT)
  {
    int index = a_operands[1].m_value.m_int;
    *a_operands = list->GetAt(index);
    return;
  }
  a_operands->Nullify();
}

static void GM_CDECL gmListSetInd(gmThread * a_thread, gmVariable * a_operands)
{
  gmUserObject * listObject = (gmUserObject *) GM_OBJECT(a_operands->m_value.m_ref);
  GM_ASSERT(listObject->m_userType == GM_LIST);
  gmUserList * list = (gmUserList *) listObject->m_user;
  if(a_operands[1].m_type == GM_INT)
  {
    int index = a_operands[1].m_value.m_int;

	// If index is outside of length bounds, fail.
	if( index >= list->Length() )
	{
		return;
	}

#if GM_USE_INCGC
    //Apply write barrier
    gmVariable oldVar = list->GetAt(index);
    if(oldVar.IsReference())
    {
      a_thread->GetMachine()->GetGC()->WriteBarrier((gmObject*)oldVar.m_value.m_ref);
    }
#endif //GM_USE_INCGC

    list->SetAt(index, a_operands[2]);
  }
}

#if GM_USE_INCGC
static void GM_CDECL gmGCDestructListUserType(gmMachine * a_machine, gmUserObject* a_object)
{
  if(a_object->m_user) 
  {
    gmUserList * list = (gmUserList *) a_object->m_user;
    list->Destruct(a_machine);
    a_machine->Sys_Free(list);
  }
  a_object->m_user = NULL;
}

static bool GM_CDECL gmGCTraceListUserType(gmMachine * a_machine, gmUserObject* a_object, gmGarbageCollector* a_gc, const int a_workLeftToGo, int& a_workDone)
{
  if(a_object->m_user) 
  {
    gmUserList * list = (gmUserList *) a_object->m_user;
    return list->Trace(a_machine, a_gc, a_workLeftToGo, a_workDone);
  }
  return true;
}

#else //GM_USE_INCGC
static void GM_CDECL gmMarkListUserType(gmMachine * a_machine, gmUserObject * a_object, gmuint32 a_mark)
{
  if(a_object->m_user) 
  {
    gmUserList * list = (gmUserList *) a_object->m_user;
    list->Mark(a_machine, a_mark);
  }
}

static void GM_CDECL gmGCListUserType(gmMachine * a_machine, gmUserObject * a_object, gmuint32 a_mark)
{
  if(a_object->m_user) 
  {
    gmUserList * list = (gmUserList *) a_object->m_user;
    list->Destruct(a_machine);
    a_machine->Sys_Free(list);
  }
  a_object->m_user = NULL;
}
#endif //GM_USE_INCGC

// libs

static gmFunctionEntry s_ListLib[] = 
{ 
  /*gm
    \lib gm
  */
  /*gm
    \function list
    \brief list will create a fixed size list object
    \param values to add to list.
    \return list
  */
  {"list", gmfList},
};

static gmFunctionEntry s_listTypeLib[] = 
{ 
  /*gm
    \lib list
  */
  /*gm
    \function Length
    \brief Length will return the current size of the fixed list
    \return int list size
  */
  {"Length", gmfListLength},
  /*gm
  \function Append
  \brief Append will add an element to the end of the list.
  \return null
  */
  {"Append", gmfListAppend},
  /*gm
  \function RemoveAt
  \brief Removes an item at an index.
  \param Index of item to remove.
  \return null
  */
  {"RemoveAt", gmfListRemoveAt},
  /*gm
  \function Remove
  \brief Removes all elements which match the specified value.
  \param (variable number), all elements of all parameters passed in are removed.
  \return null
  */
  {"Remove", gmfListRemove},
};

void gmBindListLib(gmMachine * a_machine)
{
  gmUserList::m_null.Nullify(); //Init static null

  a_machine->RegisterLibrary(s_ListLib, sizeof(s_ListLib) / sizeof(s_ListLib[0]));
  GM_LIST = a_machine->CreateUserType("list");
  a_machine->RegisterTypeLibrary(GM_LIST, s_listTypeLib, sizeof(s_listTypeLib) / sizeof(s_listTypeLib[0]));
#if GM_USE_INCGC
  a_machine->RegisterUserCallbacks(GM_LIST, gmGCTraceListUserType, gmGCDestructListUserType);
#else //GM_USE_INCGC
  a_machine->RegisterUserCallbacks(GM_LIST, gmMarkListUserType, gmGCListUserType);
#endif //GM_USE_INCGC
  a_machine->RegisterTypeOperator(GM_LIST, O_GETIND, NULL, gmListGetInd);
  a_machine->RegisterTypeOperator(GM_LIST, O_SETIND, NULL, gmListSetInd);
}

#endif // GM_LIST_LIB
