/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#ifndef _GMLISTLIB_H_
#define _GMLISTLIB_H_

#include "gmConfig.h"
#include "gmVariable.h"

// Fwd decls
class gmMachine;

#define GM_LIST_LIB 1
#define GM_LIST_LIB_GROW_BY    16

#if GM_LIST_LIB

extern gmType GM_LIST;

void gmBindListLib(gmMachine * a_machine);

/*!
  \class gmUserList
*/
class gmUserList
{
public:

  /// \brief Construct()
  bool Construct(gmMachine * a_machine, int a_initialsize = GM_LIST_LIB_GROW_BY);

  /// \brief Destruct()
  void Destruct(gmMachine * a_machine);
#if GM_USE_INCGC
  bool Trace(gmMachine * a_machine, gmGarbageCollector* a_gc, const int a_workLeftToGo, int& a_workDone);
#else //GM_USE_INCGC
  /// \brief Mark()
  void Mark(gmMachine * a_machine, gmuint32 a_mark);
#endif //GM_USE_INCGC

  /// \brief GetAt()
  GM_FORCEINLINE const gmVariable &GetAt(int a_index)
  {
    if(a_index >= 0 && a_index < m_size)
    {
      return m_list[a_index];
    }
    return m_null;
  }

  /// \brief SetAt()
  GM_FORCEINLINE bool SetAt(int a_index, const gmVariable &a_variable)
  {
    if(a_index >= 0 && a_index < m_size)
    {
      m_list[a_index] = a_variable;
      return true;
    }
    return false;
  }

  /// \brief Length()
  GM_FORCEINLINE int Length() const { return m_length; }

  /// \brief Resize()
  bool Resize(gmMachine * a_machine, int a_size);

  /// \brief Append()
  void Append( gmMachine * a_machine, const gmVariable &a_variable );

  /// \brief RemoveAt()
  void RemoveAt(int a_index );

  /// \brief Remove()
  bool Remove( const gmVariable &a_variable );


  // data
  gmVariable * m_list;
  int m_size;
  int m_length;

  static gmVariable m_null;
};


/// \brief Create a GM_ARRAY.  This much be put into a user object.
gmUserList* gmUserList_Create(gmMachine* a_machine, int a_size = 0);

#endif // GM_LIST_LIB

#endif // _GMARRAYLIB_H_
