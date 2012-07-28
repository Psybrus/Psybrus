/*
This is a version (aka dlmalloc) of malloc/free/realloc written by
Doug Lea and released to the public domain, as explained at
http://creativecommons.org/licenses/publicdomain.  Send questions,
comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.4 Wed May 27 09:56:23 2009  Doug Lea  (dl at gee)

Note: There may be an updated version of this malloc obtainable at
ftp://gee.cs.oswego.edu/pub/misc/malloc.c
Check before installing!
*
* Modified by Neil Richardson to allocate from a fixed size chunk
* of memory (via sbrk), and be capable of running multiple instances.
*
*/

#ifndef __DLMALLOC_H__
#define __DLMALLOC_H__

#include <cstdio>
#include <string.h>

namespace DLmalloc
{
	// DLmalloc structures and typedefs.
	struct malloc_chunk {
		size_t               prev_foot;  /* Size of previous chunk (if free).  */
		size_t               head;       /* Size and inuse bits. */
		struct malloc_chunk* fd;         /* double links -- used only if free. */
		struct malloc_chunk* bk;
	};

	typedef struct malloc_chunk  mchunk;
	typedef struct malloc_chunk* mchunkptr;
	typedef struct malloc_chunk* sbinptr;  /* The type of bins of chunks */
	typedef unsigned int bindex_t;         /* Described below */
	typedef unsigned int binmap_t;         /* Described below */
	typedef unsigned int flag_t;           /* The type of various bit flag sets */

	struct malloc_tree_chunk {
		/* The first four fields must be compatible with malloc_chunk */
		size_t                    prev_foot;
		size_t                    head;
		struct malloc_tree_chunk* fd;
		struct malloc_tree_chunk* bk;

		struct malloc_tree_chunk* child[2];
		struct malloc_tree_chunk* parent;
		bindex_t                  index;
	};

	typedef struct malloc_tree_chunk  tchunk;
	typedef struct malloc_tree_chunk* tchunkptr;
	typedef struct malloc_tree_chunk* tbinptr; /* The type of bins of trees */

	struct malloc_segment {
		char*        base;             /* base address */
		size_t       size;             /* allocated size */
		struct malloc_segment* next;   /* ptr to next segment */
		flag_t       sflags;           /* mmap and extern flag */
	};

	typedef struct malloc_segment  msegment;
	typedef struct malloc_segment* msegmentptr;

	/* ------------------- size_t and alignment properties -------------------- */

	static const size_t MAX_SIZE_T = (~(size_t)0);


	/* The byte and bit size of a size_t */
	static const size_t SIZE_T_SIZE = (sizeof(size_t));
	static const size_t SIZE_T_BITSIZE = (sizeof(size_t) << 3);

	/* Some constants coerced to size_t */
	/* Annoying but necessary to avoid errors on some platforms */
	static const size_t SIZE_T_ZERO = ((size_t)0);
	static const size_t SIZE_T_ONE = ((size_t)1);
	static const size_t SIZE_T_TWO = ((size_t)2);
	static const size_t SIZE_T_FOUR = ((size_t)4);
	static const size_t TWO_SIZE_T_SIZES = (SIZE_T_SIZE<<1);
	static const size_t FOUR_SIZE_T_SIZES = (SIZE_T_SIZE<<2);
	static const size_t SIX_SIZE_T_SIZES = (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES);
	static const size_t HALF_MAX_SIZE_T = (MAX_SIZE_T / 2U);

	static const size_t MALLOC_ALIGNMENT = ((size_t)8U);

	/* The bit mask value corresponding to MALLOC_ALIGNMENT */
	static const size_t  CHUNK_ALIGN_MASK = (MALLOC_ALIGNMENT - SIZE_T_ONE);

	/* footers stuff */
#define FOOTERS 0
#if FOOTERS
	static const size_t CHUNK_OVERHEAD = (TWO_SIZE_T_SIZES);
#else /* FOOTERS */
	static const size_t CHUNK_OVERHEAD = (SIZE_T_SIZE);
#endif /* FOOTERS */

	/* Bin types, widths and sizes */
	static const int NSMALLBINS = (32U);
	static const int NTREEBINS = (32U);
	static const int SMALLBIN_SHIFT =(3U);
	static const int SMALLBIN_WIDTH = (SIZE_T_ONE << SMALLBIN_SHIFT);
	static const int TREEBIN_SHIFT = (8U);
	static const int MIN_LARGE_SIZE = (SIZE_T_ONE << TREEBIN_SHIFT);
	static const int MAX_SMALL_SIZE = (MIN_LARGE_SIZE - SIZE_T_ONE);
	static const int MAX_SMALL_REQUEST = (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD);

	struct malloc_state {
		binmap_t   smallmap;
		binmap_t   treemap;
		size_t     dvsize;
		size_t     topsize;
		char*      least_addr;
		mchunkptr  dv;
		mchunkptr  top;
		size_t     trim_check;
		size_t     release_checks;
		size_t     magic;
		mchunkptr  smallbins[(NSMALLBINS+1)*2];
		tbinptr    treebins[NTREEBINS];
		size_t     footprint;
		size_t     max_footprint;
		flag_t     mflags;
#if USE_LOCKS
		MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#endif /* USE_LOCKS */
		msegment   seg;
		void*      extp;      /* Unused but available for extensions */
		size_t     exts;
	};

	typedef struct malloc_state*    mstate;

#ifdef HAVE_USR_INCLUDE_MALLOC_H
#include "/usr/include/malloc.h"
#else /* HAVE_USR_INCLUDE_MALLOC_H */
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED 1
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif  /* MALLINFO_FIELD_TYPE */
	struct mallinfo {
		MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
		MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
		MALLINFO_FIELD_TYPE smblks;   /* always 0 */
		MALLINFO_FIELD_TYPE hblks;    /* always 0 */
		MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
		MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
		MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
		MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
		MALLINFO_FIELD_TYPE fordblks; /* total free space */
		MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
	};
#endif /* STRUCT_MALLINFO_DECLARED */
#endif /* HAVE_USR_INCLUDE_MALLOC_H */

	/* ------------- Global malloc_state and malloc_params ------------------- */

	/*
	malloc_params holds global properties, including those that can be
	dynamically set using mallopt. There is a single instance, mparams,
	initialized in init_mparams. Note that the non-zeroness of "magic"
	also serves as an initialization flag.
	*/

	struct malloc_params {
		volatile size_t magic;
		size_t page_size;
		size_t granularity;
		size_t mmap_threshold;
		size_t trim_threshold;
		flag_t default_mflags;
	};

	//////////////////////////////////////////////////////////////////////////
	// allocator
	class allocator
	{
	public:
		allocator();
		~allocator();

		void init( void* pmembase, size_t memsize );

		/* dlmalloc routine */
		void* dlmalloc(size_t);
		void  dlfree(void*);
		void* dlcalloc(size_t, size_t);
		void* dlrealloc(void*, size_t);
		void* dlalignedrealloc(void*, size_t, size_t);
		void* dlmemalign(size_t alignment, size_t size);
		void* dlvalloc(size_t);
		int dlmallopt(int, int);
		size_t dlmalloc_footprint(void);
		size_t dlmalloc_max_footprint(void);
		void** dlindependent_calloc(size_t, size_t, void**);
		void** dlindependent_comalloc(size_t, size_t*, void**);
		void*  dlpvalloc(size_t);
		int  dlmalloc_trim(size_t);
		void  dlmalloc_stats(void);
		size_t dlmalloc_usable_size(void*);
		mallinfo dlmallinfo(void);

		size_t freebytes();
		size_t totalbytes();
		size_t breakbytes();
		size_t maxbreakbytes();
		bool inheap( void* );

	private:
		void init_top(mstate m, mchunkptr p, size_t psize);
		void add_segment(mstate m, char* tbase, size_t tsize, flag_t mmapped);
		int change_mparam(int param_number, int value);

		void* sys_alloc(mstate m, size_t nb);
		int sys_trim(mstate m, size_t pad);

		void* internal_realloc(mstate m, void* oldmem, size_t bytes);
		void* internal_alignedrealloc(mstate m, void* oldmem, size_t alignment, size_t bytes);
		void* internal_memalign(mstate m, size_t alignment, size_t bytes);

		void** ialloc(mstate m,
			size_t n_elements,
			size_t* sizes,
			int opts,
			void* chunks[]);

		mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb);

		void* internal_sbrk(int);

	private:
		malloc_params		mparams;
		malloc_state		_gm_;

		char*				m_pmembase;
		size_t				m_membrk;
		size_t				m_maxmembrk;
		size_t				m_memsize;
	};

	//////////////////////////////////////////////////////////////////////////
	// Inlines
	inline allocator::allocator()
	{
		
	}

	inline allocator::~allocator()
	{

	}

	inline void allocator::init( void* pmembase, size_t memsize )
	{
		m_pmembase = (char*)pmembase;
		m_membrk = 0;
		m_maxmembrk = 0;
		m_memsize = memsize;
	}

	inline void* allocator::internal_sbrk(int i)
	{
		if( ( i + m_membrk ) > m_memsize )
		{
			return ((void*)(MAX_SIZE_T));
		}
		
		void* parea = &m_pmembase[m_membrk];
		m_membrk += i;

		if( m_membrk > m_maxmembrk )
		{
			m_maxmembrk = m_membrk;
		}
		return parea;
	}

	inline size_t allocator::freebytes()
	{
		size_t unbrk = m_memsize - m_membrk;
		mallinfo info = dlmallinfo();
		return unbrk + info.fordblks;
	}

	inline size_t allocator::totalbytes()
	{
		return m_memsize;
	}

	inline size_t allocator::breakbytes()
	{
		return m_membrk;
	}

	inline size_t allocator::maxbreakbytes()
	{
		return m_maxmembrk;
	}

	inline bool allocator::inheap( void* p )
	{
		return p >= m_pmembase && p < ( m_pmembase + m_memsize );
	}
}

#endif
