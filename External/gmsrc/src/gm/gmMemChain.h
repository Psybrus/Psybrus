/*
    _____               __  ___          __            ____        _      __
   / ___/__ ___ _  ___ /  |/  /__  ___  / /_____ __ __/ __/_______(_)__  / /_
  / (_ / _ `/  ' \/ -_) /|_/ / _ \/ _ \/  '_/ -_) // /\ \/ __/ __/ / _ \/ __/
  \___/\_,_/_/_/_/\__/_/  /_/\___/_//_/_/\_\\__/\_, /___/\__/_/ /_/ .__/\__/
                                               /___/             /_/
                                             
  See Copyright Notice in gmMachine.h

*/

#ifndef _GMMEMCHAIN_H_
#define _GMMEMCHAIN_H_

#include "gmConfig.h"

/// \class gmMemChain
/// \brief gmMemChain is a simple memory allocator that allows many small allocations from one heap allocation.
class gmMemChain
{
public:

  gmMemChain(gmuptr a_elementSize, gmuptr a_numElementsInChunk);
  virtual ~gmMemChain();

  /// \brief Alloc()
  void* Alloc(gmuptr a_numElements);
  void* Alloc();

  /// \brief Alloc memory
  /// \param a_numBytes Number of bytes ot allocate.
  /// \param a_alignNumBytes Number of bytes to align to.
  void* AllocBytes(gmuptr a_numBytes, gmuptr a_alignNumBytes = 1);

  /// \brief Reset()
  void Reset();
  
  /// \brief ResetAndFreeMemory()
  void ResetAndFreeMemory();

  /// \brief Presize
  void Presize(int a_kbytes);
  
  /// \brief GetElementSize()
  inline gmuptr GetElementSize() { return m_elementSize; }

  /// \brief GetSystemMemUsed will return the number of bytes allocated by the system.
  gmuptr GetSystemMemUsed() const;

protected:

  struct MemChunk
  {
    MemChunk* m_nextChunk;
    MemChunk* m_lastChunk;
    void* m_curAddress;
    void* m_minAddress;
    void* m_lastAddress;
  };

  gmuptr m_chunkSize;     //!< Size of memory chunk
  gmuptr m_elementSize;   //!< Size of element
  MemChunk* m_rootChunk;        //!< First chunk in chain
  MemChunk* m_currentChunk;     //!< Current chunk alloc's use

  inline MemChunk* NewChunk();
  inline void FreeChunks();
};


#endif // _GMMEMCHAIN_H_
